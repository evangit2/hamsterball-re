"""Runtime telemetry from the Hamsterball process."""
import ctypes
import os
import struct
import time
from typing import Optional

import psutil

from .config import Config

# Known Hamsterball static addresses (image base 0x400000)
APP_GLOBAL_PTR = 0x005341E0  # DAT_005341e0 -> App*
APP_TARGET_FPS_OFFSET = 0x16C
APP_RENDER_FPS_OFFSET = 0x170
APP_LAST_FRAME_TIME = 0x164  # current tick / last frame time

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

    def _read_process_memory(self, pid: int, address: int, size: int) -> Optional[bytes]:
        """Read memory from a Linux process. Requires ptrace access."""
        buf = ctypes.create_string_buffer(size)
        local = Iovec(ctypes.cast(buf, ctypes.c_void_p), size)
        remote = Iovec(ctypes.c_void_p(address), size)
        n = libc.process_vm_readv(pid, ctypes.byref(local), 1, ctypes.byref(remote), 1, 0)
        if n != size:
            return None
        return buf.raw

    def get(self, pid: Optional[int] = None) -> dict:
        if pid is None:
            pid = self._find_game_pid()

        if not pid:
            return {"available": False, "error": "game process not found"}

        result: dict = {"available": True, "pid": pid, "read_at": time.time()}

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
                result["last_frame_time_raw"] = struct.unpack("<I", frame_time_bytes)[0]
        else:
            result["memory_read_error"] = "could not read App global (ptrace_scope may block)"

        return result

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

    def estimate_fps_from_screenshots(self, samples: int = 5, interval: float = 0.2) -> dict:
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
