"""Runtime telemetry from the Hamsterball process."""
from __future__ import annotations

import ctypes
import os
import struct
import time
from collections import deque
from typing import Any, Optional

import psutil

from .config import Config

# Known Hamsterball static addresses (image base 0x400000)
# g_App is a singleton stored as a pointer at DAT_005341E0 (set by App_Ctor).
# Offsets are relative to the App* structure and verified against App_Ctor
# (0x46DC40) and App_Run (0x46BD80) decompilations:
#   param_1[0x5b] = 100  -> App+0x16C = target update FPS
#   param_1[0x5c] = 0x4b -> App+0x170 = target render FPS
#   param_1[0x59] = GetTickCount() -> App+0x164 = last update tick
APP_GLOBAL_PTR = 0x005341E0  # DAT_005341e0 -> App*
APP_TARGET_FPS_OFFSET = 0x16C
APP_RENDER_FPS_OFFSET = 0x170
APP_LAST_FRAME_TIME = 0x164  # last update tick (GetTickCount value)

libc = ctypes.CDLL("libc.so.6")


class Iovec(ctypes.Structure):
    _fields_ = [("iov_base", ctypes.c_void_p), ("iov_len", ctypes.c_size_t)]


libc.process_vm_readv.argtypes = [
    ctypes.c_int,
    ctypes.POINTER(Iovec),
    ctypes.c_ulong,
    ctypes.POINTER(Iovec),
    ctypes.c_ulong,
    ctypes.c_ulong,
]
libc.process_vm_readv.restype = ctypes.c_ssize_t


class Telemetry:
    def __init__(self, cfg: Optional[Config] = None):
        self.cfg = cfg or Config()
        self._history: deque[dict[str, Any]] = deque(maxlen=60)

    def _read_process_memory(self, pid: int, address: int, size: int) -> Optional[bytes]:
        """Read memory from a Linux process. Requires ptrace access."""
        buf = ctypes.create_string_buffer(size)
        local = Iovec(ctypes.cast(buf, ctypes.c_void_p), size)
        remote = Iovec(ctypes.c_void_p(address), size)
        n = libc.process_vm_readv(pid, ctypes.byref(local), 1, ctypes.byref(remote), 1, 0)
        if n != size:
            return None
        return buf.raw

    def get(self, pid: Optional[int] = None) -> dict[str, Any]:
        if pid is None:
            pid = self._find_game_pid()

        if not pid:
            return {"available": False, "error": "game process not found"}

        result: dict[str, Any] = {"available": True, "pid": pid, "read_at": time.time()}

        # Basic process metrics
        try:
            p = psutil.Process(pid)
            with p.oneshot():
                result["cpu_percent"] = p.cpu_percent(interval=0.05)
                result["memory_mb"] = p.memory_info().rss / (1024 * 1024)
        except Exception as e:
            result["process_error"] = str(e)

        # Try to read App global and FPS values from the Windows binary
        app_ptr_bytes = self._read_process_memory(pid, APP_GLOBAL_PTR, 4)
        if app_ptr_bytes:
            app_ptr = struct.unpack("<I", app_ptr_bytes)[0]
            result["app_ptr"] = f"0x{app_ptr:08X}"

            target_fps_bytes = self._read_process_memory(pid, app_ptr + APP_TARGET_FPS_OFFSET, 4)
            if target_fps_bytes:
                result["target_fps"] = struct.unpack("<i", target_fps_bytes)[0]

            render_fps_bytes = self._read_process_memory(pid, app_ptr + APP_RENDER_FPS_OFFSET, 4)
            if render_fps_bytes:
                result["render_fps"] = struct.unpack("<i", render_fps_bytes)[0]

            frame_time_bytes = self._read_process_memory(pid, app_ptr + APP_LAST_FRAME_TIME, 4)
            if frame_time_bytes:
                result["last_frame_tick"] = struct.unpack("<I", frame_time_bytes)[0]
        else:
            result["memory_read_error"] = "could not read App global (ptrace_scope may block)"

        self._history.append(result)
        return result

    def history(self, count: Optional[int] = None) -> dict[str, Any]:
        """Return recent telemetry samples."""
        items = list(self._history)
        if count is not None:
            items = items[-count:]
        return {"success": True, "count": len(items), "samples": items}

    def estimate_runtime_fps(self, samples: int = 10, interval: float = 0.1) -> dict[str, Any]:
        """Estimate actual rendered FPS by polling the in-game tick counter.

        This measures how fast the game's internal last_frame_tick advances,
        which correlates with the real update/render loop rate.
        """
        pid = self._find_game_pid()
        if not pid:
            return {"success": False, "error": "game process not found"}

        app_ptr_bytes = self._read_process_memory(pid, APP_GLOBAL_PTR, 4)
        if not app_ptr_bytes:
            return {"success": False, "error": "could not read App pointer"}
        app_ptr = struct.unpack("<I", app_ptr_bytes)[0]

        ticks = []
        for _ in range(samples + 1):
            t0 = time.time()
            frame_bytes = self._read_process_memory(pid, app_ptr + APP_LAST_FRAME_TIME, 4)
            if not frame_bytes:
                return {"success": False, "error": "could not read last_frame_tick"}
            tick = struct.unpack("<I", frame_bytes)[0]
            ticks.append((t0, tick))
            time.sleep(interval)

        # Compute deltas (ms per game tick)
        deltas = []
        for (t1, tick1), (t2, tick2) in zip(ticks, ticks[1:]):
            dt = t2 - t1
            d_tick = (tick2 - tick1) & 0xFFFFFFFF
            if d_tick > 0 and dt > 0:
                # GetTickCount wraps every ~49 days; d_tick is unsigned
                if d_tick > 0x7FFFFFFF:
                    continue
                deltas.append((dt, d_tick))

        if not deltas:
            return {"success": True, "estimated_fps": 0.0, "samples": samples}

        total_real_s = sum(d[0] for d in deltas)
        total_ticks = sum(d[1] for d in deltas)
        ms_per_tick = total_real_s / total_ticks * 1000.0 if total_ticks else 0
        fps = 1000.0 / ms_per_tick if ms_per_tick > 0 else 0.0
        return {
            "success": True,
            "estimated_fps": round(fps, 1),
            "ms_per_tick": round(ms_per_tick, 3),
            "samples": len(deltas),
        }

    def _find_game_pid(self) -> Optional[int]:
        exe_path = self.cfg.game_executable_path.lower()
        exe_name = os.path.basename(exe_path).lower()
        game_dir = self.cfg.game_dir.lower()

        for proc in psutil.process_iter(["pid", "name", "cmdline"]):
            name = (proc.info.get("name") or "").lower()
            cmdline = proc.info.get("cmdline") or []

            if name == exe_name:
                return proc.info["pid"]

            for s in cmdline:
                lower = (s or "").lower().rstrip('"')
                if lower == exe_path:
                    return proc.info["pid"]
                if game_dir in lower and lower.endswith(exe_name):
                    return proc.info["pid"]
        return None

    def estimate_fps_from_screenshots(self, samples: int = 5, interval: float = 0.2) -> dict[str, Any]:
        """Rough FPS estimate by timing how fast we can capture frames.

        This measures the display's update rate, not the game's internal FPS.
        """
        from .capture import Capture

        cap = Capture(self.cfg)
        deltas = []
        for i in range(samples):
            t0 = time.time()
            res = cap.capture()
            if not res["success"]:
                return {"success": False, "error": res.get("error", "capture failed")}
            t1 = time.time()
            if i > 0:
                deltas.append(t1 - t0)
            time.sleep(interval)

        if not deltas:
            return {"success": True, "estimated_fps": 0.0, "samples": samples}

        avg = sum(deltas) / len(deltas)
        fps = 1.0 / avg if avg > 0 else 0.0
        return {
            "success": True,
            "estimated_fps": round(fps, 1),
            "avg_frame_time_s": round(avg, 4),
            "samples": samples,
        }
