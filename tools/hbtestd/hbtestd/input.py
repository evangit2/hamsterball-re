"""Send keyboard and mouse input to the game."""
import os
import subprocess
from typing import Optional

from .config import Config

# Map friendly names to xdotool key names
KEY_MAP = {
    "left": "Left",
    "right": "Right",
    "up": "Up",
    "down": "Down",
    "enter": "Return",
    "return": "Return",
    "esc": "Escape",
    "escape": "Escape",
    "space": "space",
    "spacebar": "space",
    "tab": "Tab",
    "shift": "Shift",
    "ctrl": "Control",
    "alt": "Alt",
    "w": "w",
    "a": "a",
    "s": "s",
    "d": "d",
}


class InputDevice:
    def __init__(self, cfg: Optional[Config] = None):
        self.cfg = cfg or Config()

    def _run(self, cmd: list[str]) -> dict:
        env = os.environ.copy()
        env["DISPLAY"] = self.cfg.display
        try:
            result = subprocess.run(
                cmd,
                env=env,
                capture_output=True,
                text=True,
                timeout=10,
            )
            return {
                "success": result.returncode == 0,
                "returncode": result.returncode,
                "stdout": result.stdout.strip(),
                "stderr": result.stderr.strip(),
            }
        except subprocess.TimeoutExpired:
            return {"success": False, "error": "input command timed out"}
        except Exception as e:
            return {"success": False, "error": str(e)}

    def _resolve_window_id(self) -> Optional[str]:
        """Find the Hamsterball window ID on the virtual display."""
        env = os.environ.copy()
        env["DISPLAY"] = self.cfg.display
        try:
            result = subprocess.run(
                [
                    self.cfg.xdotool_binary,
                    "search",
                    "--name",
                    "Hamsterball",
                ],
                env=env,
                capture_output=True,
                text=True,
                timeout=5,
            )
            if result.returncode == 0 and result.stdout.strip():
                # Return first matching ID
                return result.stdout.strip().splitlines()[0]
        except Exception:
            pass
        return None

    def send_key(self, key: str, duration_ms: int = 50) -> dict:
        key = KEY_MAP.get(key.lower(), key)
        wid = self._resolve_window_id()
        cmd = [self.cfg.xdotool_binary]
        if wid:
            cmd += ["key", "--window", wid, key]
        else:
            cmd += ["key", key]
        result = self._run(cmd)
        return {
            "success": result.get("success", False),
            "key": key,
            "duration_ms": duration_ms,
            "window_id": wid,
            **result,
        }

    def hold_key(self, key: str, duration_ms: int = 500) -> dict:
        key = KEY_MAP.get(key.lower(), key)
        wid = self._resolve_window_id()
        base = [self.cfg.xdotool_binary]

        def key_cmd(action: str) -> list[str]:
            cmd = list(base)
            if wid:
                cmd += [action, "--window", wid, key]
            else:
                cmd += [action, key]
            return cmd

        res_down = self._run(key_cmd("keydown"))
        import time
        time.sleep(duration_ms / 1000.0)
        res_up = self._run(key_cmd("keyup"))

        return {
            "success": res_down.get("success", False) and res_up.get("success", False),
            "key": key,
            "duration_ms": duration_ms,
            "window_id": wid,
        }

    def send_text(self, text: str) -> dict:
        wid = self._resolve_window_id()
        cmd = [self.cfg.xdotool_binary]
        if wid:
            cmd += ["type", "--window", wid, text]
        else:
            cmd += ["type", text]
        result = self._run(cmd)
        return {"success": result.get("success", False), "text": text, **result}

    def click(self, x: int, y: int, button: int = 1) -> dict:
        wid = self._resolve_window_id()
        cmd = [self.cfg.xdotool_binary]
        # Move mouse on the target display, then click.
        # --window for mouse is not directly supported; rely on global pointer.
        cmd += ["mousemove", "--sync", str(x), str(y), "click", str(button)]
        result = self._run(cmd)
        return {
            "success": result.get("success", False),
            "x": x,
            "y": y,
            "button": button,
            "window_id": wid,
            **result,
        }

    def mouse_move(self, x: int, y: int) -> dict:
        wid = self._resolve_window_id()
        cmd = [self.cfg.xdotool_binary]
        cmd += ["mousemove", "--sync", str(x), str(y)]
        result = self._run(cmd)
        return {"success": result.get("success", False), "x": x, "y": y, **result}
