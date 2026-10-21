"""Hamsterball game lifecycle manager."""
import asyncio
import io
import os
import subprocess
import time
import weakref
from pathlib import Path
from typing import Optional

import psutil

from .config import Config, match_slot_proc
from . import fpsmod


def any_game_live(exclude_pids: Optional[set] = None) -> list[int]:
    """PIDs of all live Hamsterball game processes, optionally excluding some.

    Zombie (defunct) entries are ignored — they are already dead.
    """
    found: list[int] = []
    for proc in psutil.process_iter(["pid", "name", "status"]):
        try:
            if (proc.info.get("name") or "").lower() != "hamsterball.exe":
                continue
            if proc.info.get("status") == psutil.STATUS_ZOMBIE:
                continue
            pid = proc.info["pid"]
            if not exclude_pids or pid not in exclude_pids:
                found.append(pid)
        except (psutil.NoSuchProcess, psutil.AccessDenied):
            continue
    return found


class GameManager:
    # All live managers, for cross-slot safety decisions (e.g. wineserver -k
    # must not nuke other slots' Wine when any game is still running).
    _registry: "weakref.WeakSet[GameManager]" = weakref.WeakSet()

    def __init__(self, cfg: Optional[Config] = None):
        self.cfg = cfg or Config()
        self.xvfb_proc: Optional[subprocess.Popen] = None
        self.game_proc: Optional[subprocess.Popen] = None
        self.wineserver_proc: Optional[subprocess.Popen] = None
        self.log_file: Optional[io.TextIOWrapper] = None
        self.start_time: Optional[float] = None
        self.pinned_pid: Optional[int] = None
        self._lock = asyncio.Lock()
        self._started_by_hbtestd = False
        GameManager._registry.add(self)

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

        # If our slot's game is already running (pinned or freshly scanned),
        # reuse it — never another slot's game.
        existing_pid = self._find_game_pid()
        if existing_pid:
            self.start_time = time.time()
            self.pinned_pid = existing_pid
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

        env = self.cfg.launch_env()
        env.setdefault("WINEDLLOVERRIDES", "d3d8=native")
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

        # Pin the PID we launched so later lookups never latch onto another
        # slot's (or another user's) game, even with an identical exe name.
        self.pinned_pid = pid

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

        own_pids: set[int] = set()
        if self.game_proc and self.game_proc.pid:
            own_pids.add(self.game_proc.pid)

        # Kill our slot's game only: prefer the pinned PID, else a scoped scan.
        # Never touch other slots' (or other users') games by bare name.
        pid = self._find_game_pid()
        if pid:
            own_pids.add(pid)
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
            self.pinned_pid = None

        if self.game_proc and self.game_proc.poll() is None:
            self.game_proc.terminate()
            try:
                self.game_proc.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.game_proc.kill()
            killed.append({"what": "game_proc", "pid": self.game_proc.pid})

        # wineserver -k kills EVERY Wine process on this host, including other
        # users' games. Only run it when no game processes remain at all.
        others = any_game_live(exclude_pids=own_pids)
        if not others:
            try:
                env = self.cfg.launch_env()
                wineserver_proc = subprocess.run(
                    [self.cfg.wine_binary, "server", "-k"],
                    capture_output=True,
                    text=True,
                    timeout=10,
                    env=env,
                )
                killed.append({"what": "wineserver", "status": wineserver_proc.returncode})
            except Exception as e:
                killed.append({"what": "wineserver", "error": str(e)})
        else:
            killed.append(
                {"what": "wineserver", "status": "skipped",
                 "reason": f"{len(others)} other game(s) still running"}
            )

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

    def _proc_matches_slot(self, proc: psutil.Process) -> bool:
        """Scoped ownership check: cmdline path or cwd ties it to our game dir."""
        try:
            with proc.oneshot():
                name = proc.name()
                try:
                    cmdline = proc.cmdline()
                except (psutil.NoSuchProcess, psutil.AccessDenied):
                    cmdline = []
                try:
                    cwd = proc.cwd()
                except (psutil.NoSuchProcess, psutil.AccessDenied):
                    cwd = ""
            return match_slot_proc(
                name,
                cmdline,
                cwd,
                self.cfg.game_executable_path,
                os.path.basename(self.cfg.game_executable_path),
                self.cfg.game_dir,
            )
        except (psutil.NoSuchProcess, psutil.AccessDenied):
            return False

    def _pinned_pid_live(self) -> Optional[int]:
        """Return the pinned PID if it is still our slot's live game."""
        if not self.pinned_pid:
            return None
        try:
            p = psutil.Process(self.pinned_pid)
            if p.status() == psutil.STATUS_ZOMBIE:
                self.pinned_pid = None
                return None
            if self._proc_matches_slot(p):
                return self.pinned_pid
        except (psutil.NoSuchProcess, psutil.AccessDenied):
            pass
        self.pinned_pid = None
        return None

    def _find_game_pid(self) -> Optional[int]:
        # Fast path: the PID we launched (or explicitly reused).
        pinned = self._pinned_pid_live()
        if pinned:
            return pinned

        # Scoped scan: only processes tied to this slot's game dir.
        # (Zombies ignored; bare name matches ignored — they latch onto
        # other users' games.)
        for proc in psutil.process_iter(["pid", "name", "status"]):
            try:
                if proc.info.get("status") == psutil.STATUS_ZOMBIE:
                    continue
                name = (proc.info.get("name") or "").lower()
                exe_name = os.path.basename(self.cfg.game_executable_path).lower()
                if name != exe_name and name != "wine" and name != "wine64":
                    continue
            except (psutil.NoSuchProcess, psutil.AccessDenied):
                continue
            try:
                p = psutil.Process(proc.info["pid"])
            except (psutil.NoSuchProcess, psutil.AccessDenied):
                continue
            if self._proc_matches_slot(p):
                return p.pid
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

