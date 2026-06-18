"""hbtestd health/status helpers.

Provides a single structured status snapshot covering the server, game process,
virtual display, required binaries, and current configuration.
"""
from __future__ import annotations

import os
import shutil
import time
from typing import Any, Optional

import psutil

from .config import Config
from .gamemgr import GameManager


def _ptrace_scope() -> int | None:
    try:
        with open("/proc/sys/kernel/yama/ptrace_scope", "r") as f:
            return int(f.read().strip())
    except Exception:
        return None


def _binary_status(name: str) -> dict[str, Any]:
    path = shutil.which(name)
    return {"available": path is not None, "path": path}


def get_health(cfg: Optional[Config] = None, mgr: Optional[GameManager] = None) -> dict[str, Any]:
    cfg = cfg or Config()
    mgr = mgr or GameManager(cfg)
    pid = mgr._find_game_pid()
    game_status: dict[str, Any] = {"running": pid is not None}
    if pid:
        try:
            p = psutil.Process(pid)
            with p.oneshot():
                game_status.update(
                    {
                        "pid": pid,
                        "ppid": p.ppid(),
                        "cmdline": p.cmdline(),
                        "cpu_percent": p.cpu_percent(interval=0.05),
                        "memory_mb": round(p.memory_info().rss / (1024 * 1024), 2),
                    }
                )
        except Exception as e:
            game_status["error"] = str(e)

    xvfb_running = False
    try:
        for proc in psutil.process_iter(["pid", "name", "cmdline"]):
            if proc.info["name"] == "Xvfb" and cfg.display in (proc.info.get("cmdline") or []):
                xvfb_running = True
                break
    except Exception:
        pass

    return {
        "success": True,
        "server": {
            "version": "0.3.0",
            "timestamp": time.time(),
        },
        "game": game_status,
        "display": {
            "display": cfg.display,
            "xvfb_running": xvfb_running,
        },
        "dependencies": {
            "wine": _binary_status(cfg.wine_binary),
            "xvfb": _binary_status(cfg.xvfb_binary),
            "xdotool": _binary_status(cfg.xdotool_binary),
            "scrot": _binary_status(cfg.scrot_binary),
        },
        "ptrace_scope": _ptrace_scope(),
        "config": {
            "game_dir": cfg.game_dir,
            "game_exe": cfg.game_exe,
            "screenshot_path": cfg.screenshot_path,
            "log_path": cfg.log_path,
            "fps_mod_enabled": cfg.fps_mod_enabled,
        },
    }
