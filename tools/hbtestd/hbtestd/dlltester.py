"""DLL mod crash tester for Hamsterball.

Quickly tests whether a DLL mod causes the game to crash on startup.
Workflow:
  1. Back up the original DLL (if not already backed up)
  2. Copy in the modded DLL
  3. Launch the game under Wine/Xvfb with a short timeout
  4. If the process dies early → crash detected, capture exit code
  5. If the process survives past the timeout → OK
  6. Restore the original DLL

Design goals:
  - Total test time < 10 seconds
  - Original files always restored, even on crash
  - Structured result returned to the calling agent
"""
from __future__ import annotations

import os
import shutil
import subprocess
import time
from dataclasses import dataclass
from typing import Optional

import psutil

from .config import Config


@dataclass
class TestResult:
    ok: bool
    crash: bool
    exit_code: Optional[int]
    runtime_seconds: float
    error: Optional[str] = None
    dll_path: Optional[str] = None
    backup_path: Optional[str] = None
    restored: bool = False


class DllTester:
    """Test a DLL mod by swapping it into the game directory and launching."""

    # Default DLLs that can be proxied
    PROXYABLE_DLLS = ["bass.dll", "d3d8.dll", "dinput8.dll", "dsound.dll"]

    def __init__(self, cfg: Config):
        self.cfg = cfg

    def _dll_path(self, dll_name: str) -> str:
        return os.path.join(self.cfg.game_dir, dll_name)

    def _backup_path(self, dll_name: str) -> str:
        return os.path.join(self.cfg.game_dir, dll_name + ".hbtestd_orig")

    def _backup_exists(self, dll_name: str) -> bool:
        return os.path.exists(self._backup_path(dll_name))

    def _backup_original(self, dll_name: str) -> str:
        """Back up the original DLL if not already backed up."""
        original = self._dll_path(dll_name)
        backup = self._backup_path(dll_name)
        if not os.path.exists(backup) and os.path.exists(original):
            shutil.copy2(original, backup)
        return backup

    def _restore_original(self, dll_name: str) -> bool:
        """Restore the original DLL from backup."""
        original = self._dll_path(dll_name)
        backup = self._backup_path(dll_name)
        if os.path.exists(backup):
            shutil.copy2(backup, original)
            return True
        return False

    def _is_xvfb_running(self) -> bool:
        for proc in psutil.process_iter(["pid", "name", "cmdline"]):
            name = (proc.info.get("name") or "").lower()
            if name == "xvfb":
                cmdline = proc.info.get("cmdline") or []
                if self.cfg.display in cmdline:
                    return True
        return False

    def _ensure_xvfb(self) -> bool:
        """Ensure Xvfb is running on the configured display."""
        if self._is_xvfb_running():
            return True
        try:
            subprocess.Popen(
                [self.cfg.xvfb_binary, self.cfg.display,
                 "-screen", "0", self.cfg.screen_resolution,
                 "+extension", "RANDR", "-noreset", "-nolisten", "tcp"],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
            time.sleep(1.0)
            return self._is_xvfb_running()
        except Exception:
            return False

    def _kill_game(self) -> None:
        """Kill any running game/wine processes."""
        for proc in psutil.process_iter(["pid", "name", "cmdline"]):
            name = (proc.info.get("name") or "").lower()
            if name in ("hamsterball.exe", "wine", "wine64", "wineserver"):
                try:
                    proc.kill()
                except (psutil.NoSuchProcess, psutil.AccessDenied):
                    pass
        # Also try wine server cleanup
        try:
            subprocess.run(
                [self.cfg.wine_binary, "server", "-k"],
                capture_output=True,
                timeout=5,
            )
        except Exception:
            pass

    def _is_game_running(self) -> Optional[int]:
        """Check if the game process is running, return PID or None."""
        for proc in psutil.process_iter(["pid", "name"]):
            name = (proc.info.get("name") or "").lower()
            if name == "hamsterball.exe":
                return proc.info["pid"]
        return None

    def test_dll(
        self,
        mod_dll_path: str,
        target_dll: str = "bass.dll",
        timeout: float = 8.0,
        cleanup_delay: float = 1.0,
    ) -> TestResult:
        """Test a DLL mod by launching the game with it.

        Args:
            mod_dll_path: Path to the modded DLL file to test.
            target_dll: Name of the DLL to replace (e.g. "bass.dll").
            timeout: Seconds to wait before declaring the DLL safe.
                     Game crash usually happens within 1-2s of launch.
            cleanup_delay: Seconds to wait after killing game before restoring.

        Returns:
            TestResult with crash status, exit code, and runtime.
        """
        start_time = time.time()

        # Validate inputs
        if not os.path.exists(mod_dll_path):
            return TestResult(
                ok=False, crash=False, exit_code=None,
                runtime_seconds=0.0,
                error=f"mod DLL not found: {mod_dll_path}",
            )

        if target_dll not in self.PROXYABLE_DLLS:
            return TestResult(
                ok=False, crash=False, exit_code=None,
                runtime_seconds=0.0,
                error=f"unsupported target DLL: {target_dll}. "
                      f"Must be one of {self.PROXYABLE_DLLS}",
            )

        original_path = self._dll_path(target_dll)
        backup_path = self._backup_path(target_dll)

        # Kill any existing game
        self._kill_game()
        time.sleep(0.5)

        # Ensure Xvfb is running (game exits immediately without a display)
        if not self._ensure_xvfb():
            return TestResult(
                ok=False, crash=False, exit_code=None,
                runtime_seconds=0.0,
                error="failed to start Xvfb display server",
            )

        # Back up the original DLL
        self._backup_original(target_dll)

        # Also back up bass_real.dll if this is a bass proxy
        # (the mod might depend on it being present)
        if target_dll == "bass.dll":
            real_dll = os.path.join(self.cfg.game_dir, "bass_real.dll")
            real_backup = os.path.join(self.cfg.game_dir, "bass_real.dll.hbtestd_orig")
            if not os.path.exists(real_backup) and os.path.exists(real_dll):
                shutil.copy2(real_dll, real_backup)

        # Swap in the modded DLL
        shutil.copy2(mod_dll_path, original_path)

        # Remove save files that might interfere
        for f in ["RSoftInfo.dat", "HS.CFG", "unins000.dat"]:
            p = os.path.join(self.cfg.game_dir, f)
            if os.path.exists(p):
                os.remove(p)

        # Prepare environment
        env = os.environ.copy()
        env["DISPLAY"] = self.cfg.display
        if self.cfg.libgl_software:
            env["LIBGL_ALWAYS_SOFTWARE"] = "1"
        env.setdefault("WINEDEBUG", "-all")

        # Launch the game
        proc = subprocess.Popen(
            [self.cfg.wine_binary, self.cfg.game_executable_path],
            cwd=self.cfg.game_dir,
            env=env,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
        )

        # Poll for crash: check if the process exits within timeout
        crash = False
        exit_code = None
        deadline = time.time() + timeout

        while time.time() < deadline:
            time.sleep(0.2)
            poll = proc.poll()
            if poll is not None:
                # Process exited before timeout → likely crash
                exit_code = poll
                crash = True
                break

        # If we reach here without crash, the game is still running → OK
        if not crash:
            # Game survived! Kill it now.
            proc.terminate()
            try:
                proc.wait(timeout=3)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait(timeout=2)

        # Kill wine processes
        self._kill_game()
        time.sleep(cleanup_delay)

        # Restore original DLL
        restored = self._restore_original(target_dll)

        # Also restore bass_real.dll if we backed it up
        if target_dll == "bass.dll":
            real_dll = os.path.join(self.cfg.game_dir, "bass_real.dll")
            real_backup = os.path.join(self.cfg.game_dir, "bass_real.dll.hbtestd_orig")
            if os.path.exists(real_backup):
                if os.path.exists(real_dll):
                    os.remove(real_dll)
                shutil.copy2(real_backup, real_dll)

        runtime = time.time() - start_time
        elapsed_alive = time.time() - start_time - (cleanup_delay + 0.5)

        # Capture game output for crash diagnostics
        output = ""
        if proc.stdout:
            try:
                output = proc.stdout.read(4096).decode(errors="replace")
            except Exception:
                pass

        return TestResult(
            ok=not crash,
            crash=crash,
            exit_code=exit_code,
            runtime_seconds=round(runtime, 2),
            error=None if not crash else f"game exited with code {exit_code} after {elapsed_alive:.1f}s",
            dll_path=mod_dll_path,
            backup_path=backup_path,
            restored=restored,
        )

    def restore_all(self) -> dict:
        """Restore all backed-up DLLs."""
        restored = []
        for dll_name in self.PROXYABLE_DLLS:
            backup = self._backup_path(dll_name)
            if os.path.exists(backup):
                original = self._dll_path(dll_name)
                shutil.copy2(backup, original)
                restored.append(dll_name)
        # Also restore bass_real.dll
        real_backup = os.path.join(self.cfg.game_dir, "bass_real.dll.hbtestd_orig")
        if os.path.exists(real_backup):
            real_dll = os.path.join(self.cfg.game_dir, "bass_real.dll")
            if os.path.exists(real_dll):
                os.remove(real_dll)
            shutil.copy2(real_backup, real_dll)
            restored.append("bass_real.dll")
        return {"restored": restored}

    def list_backups(self) -> dict:
        """List all backed-up DLLs."""
        backups = {}
        for dll_name in self.PROXYABLE_DLLS:
            backup = self._backup_path(dll_name)
            if os.path.exists(backup):
                backups[dll_name] = {
                    "backup_path": backup,
                    "size": os.path.getsize(backup),
                }
        return backups
