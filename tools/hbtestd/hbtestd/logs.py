"""Read the Hamsterball/Wine game log."""
from __future__ import annotations

import os
from pathlib import Path
from typing import Optional

from .config import Config


class GameLog:
    def __init__(self, cfg: Optional[Config] = None):
        self.cfg = cfg or Config()

    def tail(self, lines: int = 50) -> dict:
        """Return the last N lines of the game log."""
        path = Path(self.cfg.log_path)
        if not path.exists():
            return {"success": True, "lines": [], "path": str(path), "note": "log file does not exist"}
        try:
            with open(path, "r", errors="replace") as f:
                all_lines = f.readlines()
            tail = [line.rstrip("\n") for line in all_lines[-lines:]]
            return {"success": True, "path": str(path), "lines": tail, "total_lines": len(all_lines)}
        except Exception as e:
            return {"success": False, "error": str(e)}

    def read(self, offset: int = 0, limit: int = 200) -> dict:
        """Read a slice of the game log."""
        path = Path(self.cfg.log_path)
        if not path.exists():
            return {"success": True, "lines": [], "path": str(path), "note": "log file does not exist"}
        try:
            with open(path, "r", errors="replace") as f:
                all_lines = f.readlines()
            chunk = [line.rstrip("\n") for line in all_lines[offset : offset + limit]]
            return {
                "success": True,
                "path": str(path),
                "lines": chunk,
                "offset": offset,
                "limit": limit,
                "total_lines": len(all_lines),
            }
        except Exception as e:
            return {"success": False, "error": str(e)}
