"""FPS mod helpers for hbtestd.

Lets the MCP server optionally install the bass.dll proxy FPS mod into the
game directory before launching, and restore the original bass.dll on stop.
"""
import os
from pathlib import Path
from typing import Optional

from .config import Config


def write_mod_ini(
    cfg: Config,
    target_fps: int = 144,
    render_fps: int = 144,
    uncap: int = 0,
) -> str:
    """Write an override hamsterball_fps.ini next to the source mod files.

    Returns the path written.
    """
    # Write into the same directory as the configured mod DLL so gamemgr copies it.
    dll_dir = os.path.dirname(cfg.fps_mod_dll) or "."
    ini_path = os.path.join(dll_dir, "hamsterball_fps.ini")

    with open(ini_path, "w") as f:
        f.write("[FPS]\n")
        f.write(f"TargetFPS={target_fps}\n")
        f.write(f"RenderFPS={render_fps}\n\n")
        f.write("[Uncap]\n")
        f.write(f"Uncap={uncap}\n")
    return ini_path


def install_mod(cfg: Config) -> tuple[bool, list[str] | str]:
    """Install the bass.dll FPS mod into the game directory."""
    game_dir = cfg.game_dir
    bass_path = os.path.join(game_dir, "bass.dll")
    bass_real_path = os.path.join(game_dir, "bass_real.dll")
    ini_path = os.path.join(game_dir, "hamsterball_fps.ini")

    if not os.path.exists(cfg.fps_mod_dll):
        return False, f"fps mod dll not found: {cfg.fps_mod_dll}"

    try:
        if os.path.exists(bass_real_path):
            return False, "bass_real.dll already exists; mod may already be installed"

        if os.path.exists(bass_path):
            os.replace(bass_path, bass_real_path)

        import shutil

        shutil.copy(cfg.fps_mod_dll, bass_path)
        installed: list[str] = [bass_path]

        if os.path.exists(cfg.fps_mod_ini):
            shutil.copy(cfg.fps_mod_ini, ini_path)
            installed.append(ini_path)

        return True, installed
    except Exception as e:
        return False, str(e)


def uninstall_mod(cfg: Config) -> None:
    """Restore original bass.dll and remove the mod files."""
    game_dir = cfg.game_dir
    bass_path = os.path.join(game_dir, "bass.dll")
    bass_real_path = os.path.join(game_dir, "bass_real.dll")
    ini_path = os.path.join(game_dir, "hamsterball_fps.ini")

    try:
        if os.path.exists(bass_real_path):
            if os.path.exists(bass_path):
                os.remove(bass_path)
            os.replace(bass_real_path, bass_path)
        if os.path.exists(ini_path):
            os.remove(ini_path)
    except Exception:
        pass
