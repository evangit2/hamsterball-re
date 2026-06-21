"""Hamsterball game lifecycle manager."""
import asyncio
import io
import os
import subprocess
import time
from pathlib import Path
from typing import Optional

import psutil

from .config import Config
from . import fpsmod


class GameManager:
    def __init__(self, cfg: Optional[Config] = None):
        self.cfg = cfg or Config()
        self.xvfb_proc: Optional[subprocess.Popen] = None
        self.game_proc: Optional[subprocess.Popen] = None
        self.wineserver_proc: Optional[subprocess.Popen] = None
        self.log_file: Optional[io.TextIOWrapper] = None
        self.start_time: Optional[float] = None
        self._lock = asyncio.Lock()
        self._started_by_hbtestd = False

    async def start_xvfb(self) -> bool:
        """Start a virtual X server if not already running."""
        if self._is_xvfb_running():
            return True

        display_num = self.cfg.display.lstrip(":")
        cmd = [
            self.cfg.xvfb_binary,
            self.cfg.display,
            "-screen",
            "0",
            self.cfg.screen_resolution,
            "+extension",
            "RANDR",
            "-noreset",
            "-nolisten",
            "tcp",
        ]
        try:
            self.xvfb_proc = subprocess.Popen(
                cmd,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
            # Wait briefly for Xvfb to come up
            await asyncio.sleep(0.5)
            return self._is_xvfb_running()
        except Exception as e:
            self._log(f"failed to start Xvfb: {e}")
            return False

    def _is_xvfb_running(self) -> bool:
        for proc in psutil.process_iter(["pid", "name", "cmdline"]):
            if proc.info["name"] == "Xvfb" and self.cfg.display in (
                proc.info.get("cmdline") or []
            ):
                return True
        return False

    async def start_game(self, timeout: float = 45.0) -> dict:
        async with self._lock:
            status = await self._do_start_game(timeout)
            return status

    async def _do_start_game(self, timeout: float) -> dict:
        if not os.path.exists(self.cfg.game_executable_path):
            return {
                "success": False,
                "error": f"game executable not found: {self.cfg.game_executable_path}",
            }

        if not await self.start_xvfb():
            return {"success": False, "error": "failed to start Xvfb"}

        fps_mod_files: list[str] = []
        if self.cfg.fps_mod_enabled:
            ok, msg = fpsmod.install_mod(self.cfg)
            if not ok:
                return {"success": False, "error": f"fps mod install failed: {msg}"}
            fps_mod_files = msg  # type: ignore[assignment]

        # If game is already running on the right display, reuse it.
        existing_pid = self._find_game_pid()
        if existing_pid:
            self.start_time = time.time()
            return {
                "success": True,
                "pid": existing_pid,
                "note": "game was already running; reused existing process",
            }

        # Rotate log file
        Path(self.cfg.log_path).parent.mkdir(parents=True, exist_ok=True)
        if self.log_file and not self.log_file.closed:
            self.log_file.close()
        self.log_file = open(self.cfg.log_path, "a")

        env = os.environ.copy()
        env["DISPLAY"] = self.cfg.display
        if self.cfg.libgl_software:
            env["LIBGL_ALWAYS_SOFTWARE"] = "1"
        # Use Wine's builtin d3d8.dll (translates D3D8→D3D9 internally).
        # Do NOT set WINEDLLOVERRIDES=d3d8=native — that would load a
        # MinGW-compiled proxy DLL that needs libgcc/libstdc++ runtime DLLs.
        env.setdefault("WINEDEBUG", "-all")

        cmd = [
            self.cfg.wine_binary,
            self.cfg.game_executable_path,
        ]

        try:
            self.game_proc = subprocess.Popen(
                cmd,
                cwd=self.cfg.game_dir,
                env=env,
                stdout=self.log_file,
                stderr=subprocess.STDOUT,
            )
            self.start_time = time.time()
            self._started_by_hbtestd = True
        except Exception as e:
            return {"success": False, "error": f"failed to launch game: {e}"}

        # Wait for window/executable to appear
        deadline = time.time() + timeout
        pid: Optional[int] = None
        while time.time() < deadline:
            pid = self._find_game_pid()
            if pid:
                break
            await asyncio.sleep(0.5)

        if not pid:
            return {
                "success": False,
                "error": f"game process did not appear within {timeout}s",
                "stdout_tail": self._tail_log(10),
            }

        result = {"success": True, "pid": pid, "display": self.cfg.display}
        if fps_mod_files:
            result["fps_mod_installed"] = fps_mod_files
        return result

    async def stop_game(self) -> dict:
        async with self._lock:
            return await self._do_stop_game()

    async def _do_stop_game(self) -> dict:
        killed = []

        # Remove fps mod files if they were installed
        if self.cfg.fps_mod_enabled:
            fpsmod.uninstall_mod(self.cfg)

        # Kill the game first
        pid = self._find_game_pid()
        if pid:
            try:
                p = psutil.Process(pid)
                p.terminate()
                try:
                    await asyncio.wait_for(asyncio.to_thread(p.wait, 5.0), timeout=6.0)
                except asyncio.TimeoutError:
                    p.kill()
                killed.append({"what": "game", "pid": pid})
            except Exception as e:
                killed.append({"what": "game", "pid": pid, "error": str(e)})

        if self.game_proc and self.game_proc.poll() is None:
            self.game_proc.terminate()
            try:
                self.game_proc.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.game_proc.kill()
            killed.append({"what": "game_proc", "pid": self.game_proc.pid})

        # Kill wineserver (cleans up lingering Wine processes)
        try:
            wineserver_proc = subprocess.run(
                [self.cfg.wine_binary, "server", "-k"],
                capture_output=True,
                text=True,
                timeout=10,
            )
            killed.append({"what": "wineserver", "status": wineserver_proc.returncode})
        except Exception as e:
            killed.append({"what": "wineserver", "error": str(e)})

        # Shutdown Xvfb if we started it
        if self.xvfb_proc and self.xvfb_proc.poll() is None:
            self.xvfb_proc.terminate()
            try:
                self.xvfb_proc.wait(timeout=3)
            except subprocess.TimeoutExpired:
                self.xvfb_proc.kill()
            killed.append({"what": "xvfb", "pid": self.xvfb_proc.pid})

        self.start_time = None
        self._started_by_hbtestd = False
        return {"success": True, "killed": killed}

    def _find_game_pid(self) -> Optional[int]:
        exe_path = self.cfg.game_executable_path.lower()
        exe_name = os.path.basename(exe_path).lower()
        game_dir = self.cfg.game_dir.lower()

        for proc in psutil.process_iter(["pid", "name", "cmdline"]):
            name = (proc.info.get("name") or "").lower()
            cmdline = proc.info.get("cmdline") or []

            # Wine may expose the Windows PE name as the Linux process name
            if name == exe_name:
                return proc.info["pid"]

            for s in cmdline:
                lower = (s or "").lower().rstrip('"')
                # Exact absolute path match (Wine launch)
                if lower == exe_path:
                    return proc.info["pid"]
                # Must contain the real game directory to avoid matching
                # strings like Ghidra's --program /Hamsterball.exe
                if game_dir in lower and lower.endswith(exe_name):
                    return proc.info["pid"]
        return None

    def is_running(self) -> bool:
        return self._find_game_pid() is not None

    def started_by_hbtestd(self) -> bool:
        return self._started_by_hbtestd

    def get_status(self) -> dict:
        pid = self._find_game_pid()
        if not pid:
            return {"running": False}
        try:
            p = psutil.Process(pid)
            with p.oneshot():
                runtime = time.time() - self.start_time if self.start_time else None
                return {
                    "running": True,
                    "pid": pid,
                    "ppid": p.ppid(),
                    "cmdline": p.cmdline(),
                    "cpu_percent": p.cpu_percent(interval=0.1),
                    "memory_mb": p.memory_info().rss / (1024 * 1024),
                    "runtime_seconds": runtime,
                    "display": self.cfg.display,
                }
        except Exception as e:
            return {"running": True, "pid": pid, "error": str(e)}

    def _tail_log(self, n: int) -> list[str]:
        try:
            with open(self.cfg.log_path, "r") as f:
                lines = f.readlines()
                return [line.rstrip() for line in lines[-n:]]
        except Exception:
            return []

    def _log(self, message: str) -> None:
        if self.log_file and not self.log_file.closed:
            self.log_file.write(f"{time.strftime('%Y-%m-%d %H:%M:%S')} {message}\n")
            self.log_file.flush()

