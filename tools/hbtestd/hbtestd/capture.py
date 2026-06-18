"""Screenshot capture from virtual X display."""
from __future__ import annotations

import os
import shutil
import struct
import subprocess
from pathlib import Path
from typing import Optional

from .config import Config


class Capture:
    def __init__(self, cfg: Optional[Config] = None):
        self.cfg = cfg or Config()

    def capture(self, output_path: Optional[str] = None) -> dict:
        out = output_path or self.cfg.screenshot_path
        Path(out).parent.mkdir(parents=True, exist_ok=True)

        if not shutil.which(self.cfg.scrot_binary):
            return {
                "success": False,
                "error": f"{self.cfg.scrot_binary} not found; install scrot",
            }

        env = os.environ.copy()
        env["DISPLAY"] = self.cfg.display

        cmd = [
            self.cfg.scrot_binary,
            "--silent",
            "--overwrite",
            out,
        ]

        try:
            result = subprocess.run(
                cmd,
                env=env,
                capture_output=True,
                text=True,
                timeout=10,
            )
            if result.returncode != 0:
                return {
                    "success": False,
                    "error": result.stderr or "scrot failed",
                }
            if not os.path.exists(out):
                return {"success": False, "error": "screenshot file was not created"}
            size = os.path.getsize(out)
            return {
                "success": True,
                "path": out,
                "size_bytes": size,
                "display": self.cfg.display,
            }
        except subprocess.TimeoutExpired:
            return {"success": False, "error": "screenshot capture timed out"}
        except Exception as e:
            return {"success": False, "error": str(e)}

    def get_base64(self, output_path: Optional[str] = None) -> dict:
        from base64 import b64encode

        cap = self.capture(output_path)
        if not cap["success"]:
            return cap

        try:
            with open(cap["path"], "rb") as f:
                data = f.read()
            ext = Path(cap["path"]).suffix.lstrip(".") or "png"
            return {
                "success": True,
                "path": cap["path"],
                "mime_type": f"image/{ext}",
                "base64": b64encode(data).decode("ascii"),
                "size_bytes": cap["size_bytes"],
            }
        except Exception as e:
            return {"success": False, "error": str(e)}

    def validate(self, output_path: Optional[str] = None) -> dict:
        """Check that a screenshot exists, is a valid PNG, and isn't blank."""
        path = output_path or self.cfg.screenshot_path
        if not os.path.exists(path):
            return {"success": False, "error": "screenshot file not found"}

        try:
            with open(path, "rb") as f:
                header = f.read(24)
        except Exception as e:
            return {"success": False, "error": f"read failed: {e}"}

        is_png = header[:8] == b"\x89PNG\r\n\x1a\n"
        if not is_png:
            return {"success": False, "error": "file is not a valid PNG"}

        width, height = struct.unpack(">II", header[16:24])
        size = os.path.getsize(path)

        # Very small files or tiny dimensions suggest a blank/minimal capture
        blank = size < 1024 or width < 10 or height < 10

        return {
            "success": True,
            "path": path,
            "is_png": is_png,
            "width": width,
            "height": height,
            "size_bytes": size,
            "possibly_blank": blank,
        }
