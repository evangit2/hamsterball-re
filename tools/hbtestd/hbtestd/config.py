"""hbtestd configuration helpers.

Multi-user model: each *slot* is an isolated {display, game_dir, ...} tuple so
several live games can coexist. The "default" slot preserves the legacy
single-user behavior exactly. Extra slots come from HBTESTD_SLOTS JSON:

  HBTESTD_SLOTS='{"bob": {"display": ":100", "game_dir": "/home/evan/hb_bob"}}'

Keys not given per slot inherit the corresponding HBTESTD_* default.
"""
import json
import os
from dataclasses import dataclass
from typing import Any, Optional


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
    allow_memory_write: bool = os.environ.get(
        "HBTESTD_ALLOW_MEMORY_WRITE", "0"
    ) == "1"
    default_module_name: str = os.environ.get(
        "HBTESTD_DEFAULT_MODULE", "Hamsterball.exe"
    )
    slot_name: str = "default"
    wineprefix: str = os.environ.get("HBTESTD_WINEPREFIX", "")

    @property
    def game_executable_path(self) -> str:
        return os.path.join(self.game_dir, self.game_exe)

    @property
    def sse_url(self) -> str:
        return f"http://{self.server_host}:{self.server_port}/sse"

    def can_write_memory(self, started_by_hbtestd: bool) -> bool:
        """Return True if memory writes are permitted for this process."""
        return self.allow_memory_write or started_by_hbtestd

    def launch_env(self, base: Optional[dict] = None) -> dict:
        """Environment for Wine/X11 child processes in this slot."""
        env = dict(base if base is not None else os.environ)
        env["DISPLAY"] = self.display
        if self.libgl_software:
            env["LIBGL_ALWAYS_SOFTWARE"] = "1"
        if self.wineprefix:
            env["WINEPREFIX"] = self.wineprefix
        return env


def _suffixed_path(path: str, slot: str) -> str:
    """Derive a per-slot file path; the default slot keeps the legacy path."""
    if slot == "default":
        return path
    root, dot, ext = path.rpartition(".")
    if not dot:
        return f"{path}_{slot}"
    return f"{root}_{slot}.{ext}"


def default_config() -> Config:
    """Config for the legacy default slot (pure HBTESTD_* env)."""
    return Config(slot_name="default")


def load_slot_configs() -> "dict[str, Config]":
    """Load default + HBTESTD_SLOTS JSON slot configs.

    Per-slot keys override the HBTESTD_* defaults; screenshot/log paths are
    auto-suffixed per slot so concurrent games never share output files.
    Unknown keys in a slot entry raise ValueError (fail fast on typos).
    """
    base = default_config()
    slots: dict[str, Config] = {"default": base}
    raw = os.environ.get("HBTESTD_SLOTS", "").strip()
    if not raw:
        return slots
    try:
        data = json.loads(raw)
    except json.JSONDecodeError as exc:
        raise ValueError(f"HBTESTD_SLOTS is not valid JSON: {exc}")
    if not isinstance(data, dict):
        raise ValueError("HBTESTD_SLOTS must be a JSON object of slot -> overrides")
    allowed = {f.name for f in base.__dataclass_fields__.values()} - {"slot_name"}
    for name, over in data.items():
        if not isinstance(name, str) or not name or name == "default":
            raise ValueError(f"invalid slot name: {name!r}")
        if not isinstance(over, dict):
            raise ValueError(f"slot {name!r} must map to an object of overrides")
        unknown = set(over) - allowed
        if unknown:
            raise ValueError(f"slot {name!r} has unknown keys: {sorted(unknown)}")
        kwargs: dict[str, Any] = dict(
            display=over.get("display", base.display),
            screen_resolution=over.get("screen_resolution", base.screen_resolution),
            game_dir=over.get("game_dir", base.game_dir),
            game_exe=over.get("game_exe", base.game_exe),
            server_host=base.server_host,
            server_port=base.server_port,
            screenshot_path=over.get(
                "screenshot_path",
                _suffixed_path(base.screenshot_path, name),
            ),
            log_path=over.get("log_path", _suffixed_path(base.log_path, name)),
            xvfb_binary=over.get("xvfb_binary", base.xvfb_binary),
            wine_binary=over.get("wine_binary", base.wine_binary),
            xdotool_binary=over.get("xdotool_binary", base.xdotool_binary),
            scrot_binary=over.get("scrot_binary", base.scrot_binary),
            libgl_software=over.get("libgl_software", base.libgl_software),
            fps_mod_dll=over.get("fps_mod_dll", base.fps_mod_dll),
            fps_mod_ini=over.get("fps_mod_ini", base.fps_mod_ini),
            fps_mod_enabled=over.get("fps_mod_enabled", base.fps_mod_enabled),
            allow_memory_write=over.get(
                "allow_memory_write", base.allow_memory_write
            ),
            default_module_name=over.get(
                "default_module_name", base.default_module_name
            ),
            slot_name=name,
            wineprefix=over.get("wineprefix", base.wineprefix),
        )
        slots[name] = Config(**kwargs)
    return slots


def match_slot_proc(
    proc_name: str,
    cmdline: "list[str]",
    proc_cwd: str,
    exe_path: str,
    exe_name: str,
    game_dir: str,
) -> bool:
    """True if a process belongs to the slot's game.

    A candidate matches only via the slot's own install: the exact exe path
    in its command line, or its working directory equal to the slot's game
    dir (Wine often exposes just the PE name with cwd set to the game dir).
    A bare process-name match is NOT enough — that is what used to latch
    onto other users' live games.
    """
    exe_path = exe_path.lower()
    exe_name = exe_name.lower()
    game_dir = os.path.normpath(game_dir.lower())
    name = (proc_name or "").lower()
    cmds = [(s or "").lower().rstrip('"') for s in (cmdline or [])]

    for s in cmds:
        # Exact absolute path match (Wine launch)
        if s == exe_path:
            return True
        # Real game directory in the command line (excludes Ghidra
        # '--program /Hamsterball.exe' style matches)
        if exe_name and s.endswith(exe_name) and game_dir in s:
            return True

    # Fallback: Wine exposes the PE name with cwd at the game dir
    if name == exe_name and proc_cwd:
        try:
            if os.path.normpath(proc_cwd.lower()) == game_dir:
                return True
        except Exception:
            pass
    return False
