"""hbtestd configuration helpers."""
import os
from dataclasses import dataclass


@dataclass
class Config:
    display: str = os.environ.get("HBTESTD_DISPLAY", ":99")
    screen_resolution: str = os.environ.get("HBTESTD_RESOLUTION", "800x600x24")
    game_dir: str = os.environ.get(
        "HBTESTD_GAME_DIR",
        "/home/evan/hamsterball-wasm/boxedwine-package/hamsterball",
    )
    game_exe: str = os.environ.get("HBTESTD_GAME_EXE", "Hamsterball.exe")
    server_host: str = os.environ.get("HBTESTD_HOST", "127.0.0.1")
    server_port: int = int(os.environ.get("HBTESTD_PORT", "8777"))
    screenshot_path: str = os.environ.get(
        "HBTESTD_SCREENSHOT_PATH", "/tmp/hbtestd_screenshot.png"
    )
    log_path: str = os.environ.get("HBTESTD_LOG_PATH", "/tmp/hbtestd.log")
    xvfb_binary: str = os.environ.get("HBTESTD_XVFB", "Xvfb")
    wine_binary: str = os.environ.get("HBTESTD_WINE", "wine")
    xdotool_binary: str = os.environ.get("HBTESTD_XDOTOOL", "xdotool")
    scrot_binary: str = os.environ.get("HBTESTD_SCROT", "scrot")
    libgl_software: bool = os.environ.get("HBTESTD_LIBGL_SOFTWARE", "1") == "1"
    fps_mod_dll: str = os.environ.get(
        "HBTESTD_FPS_MOD_DLL",
        "/home/evan/hamsterball-re/tools/bass_fps_proxy/bass.dll",
    )
    fps_mod_ini: str = os.environ.get(
        "HBTESTD_FPS_MOD_INI",
        "/home/evan/hamsterball-re/tools/bass_fps_proxy/hamsterball_fps.ini",
    )
    fps_mod_enabled: bool = os.environ.get("HBTESTD_FPS_MOD_ENABLED", "0") == "1"

    @property
    def game_executable_path(self) -> str:
        return os.path.join(self.game_dir, self.game_exe)

    @property
    def sse_url(self) -> str:
        return f"http://{self.server_host}:{self.server_port}/sse"
