"""MCP server entry point."""
from __future__ import annotations

import asyncio
import sys
from typing import Any

from mcp.server.fastmcp import FastMCP

from . import fpsmod
from .capture import Capture
from .config import Config
from .errors import failure, success, tool_guard
from .gamemgr import GameManager
from .health import get_health as _get_health
from .input import InputDevice
from .logs import GameLog
from .telemetry import Telemetry


cfg = Config()
mgr = GameManager(cfg)
capture = Capture(cfg)
inputs = InputDevice(cfg)
telemetry = Telemetry(cfg)
logs = GameLog(cfg)

mcp = FastMCP("hbtestd")
mcp.settings.host = cfg.server_host
mcp.settings.port = cfg.server_port


@mcp.tool()
@tool_guard
async def start_game(fps_mod: bool = False, target_fps: int = 144, render_fps: int = 144) -> dict[str, Any]:
    """Launch Hamsterball inside a virtual X display.

    Set fps_mod=True to install the bass.dll FPS mod before launching.
    target_fps and render_fps override the mod defaults when fps_mod is enabled.
    """
    if fps_mod:
        cfg.fps_mod_enabled = True
        if target_fps or render_fps:
            fpsmod.write_mod_ini(cfg, target_fps=target_fps, render_fps=render_fps)
    return success(**await mgr.start_game())


@mcp.tool()
@tool_guard
async def stop_game() -> dict[str, Any]:
    """Kill the Hamsterball process and clean up Wine/Xvfb."""
    return success(**await mgr.stop_game())


@mcp.tool()
@tool_guard
async def restart_game(fps_mod: bool = False, target_fps: int = 144, render_fps: int = 144) -> dict[str, Any]:
    """Restart Hamsterball cleanly."""
    await mgr.stop_game()
    await asyncio.sleep(0.5)
    if fps_mod:
        cfg.fps_mod_enabled = True
        if target_fps or render_fps:
            fpsmod.write_mod_ini(cfg, target_fps=target_fps, render_fps=render_fps)
    return success(**await mgr.start_game())


@mcp.tool()
@tool_guard
def get_status() -> dict[str, Any]:
    """Return current game process status."""
    return success(**mgr.get_status())


@mcp.tool()
@tool_guard
def get_health() -> dict[str, Any]:
    """Return full server/game/display health status."""
    return _get_health(cfg, mgr)


@mcp.tool()
@tool_guard
def screenshot() -> dict[str, Any]:
    """Capture a screenshot of the game and return its filesystem path."""
    return capture.capture()


@mcp.tool()
@tool_guard
def screenshot_base64() -> dict[str, Any]:
    """Capture a screenshot and return it as base64 image data."""
    return capture.get_base64()


@mcp.tool()
@tool_guard
def validate_screenshot() -> dict[str, Any]:
    """Validate the latest screenshot (PNG header, dimensions, blank detection)."""
    return capture.validate()


@mcp.tool()
@tool_guard
def send_key(key: str, duration_ms: int = 50) -> dict[str, Any]:
    """Send a single keypress to the game window.

    Valid keys include: Left, Right, Up, Down, Return, Escape, space, w, a, s, d.
    """
    return inputs.send_key(key, duration_ms)


@mcp.tool()
@tool_guard
def hold_key(key: str, duration_ms: int = 500) -> dict[str, Any]:
    """Hold a key down for duration_ms milliseconds."""
    return inputs.hold_key(key, duration_ms)


@mcp.tool()
@tool_guard
def send_combo(keys: list[str], duration_ms: int = 50) -> dict[str, Any]:
    """Hold multiple keys together, e.g. ['ctrl', 'a'] or ['shift', 'Left']."""
    return inputs.send_combo(keys, duration_ms)


@mcp.tool()
@tool_guard
def tap_pattern(key: str, count: int = 3, interval_ms: int = 100) -> dict[str, Any]:
    """Tap a key repeatedly."""
    return inputs.tap_pattern(key, count, interval_ms)


@mcp.tool()
@tool_guard
def list_keys() -> dict[str, Any]:
    """Return the list of supported key names."""
    return inputs.list_keys()


@mcp.tool()
@tool_guard
def send_text(text: str) -> dict[str, Any]:
    """Type a string of text into the game window."""
    return inputs.send_text(text)


@mcp.tool()
@tool_guard
def mouse_click(x: int, y: int, button: int = 1) -> dict[str, Any]:
    """Click at screen coordinates (x, y)."""
    return inputs.click(x, y, button)


@mcp.tool()
@tool_guard
def mouse_move(x: int, y: int) -> dict[str, Any]:
    """Move the mouse cursor to (x, y)."""
    return inputs.mouse_move(x, y)


@mcp.tool()
@tool_guard
def get_telemetry() -> dict[str, Any]:
    """Read runtime info from the Hamsterball process (FPS targets, memory, CPU)."""
    return telemetry.get()


@mcp.tool()
@tool_guard
def get_telemetry_history(count: int = 10) -> dict[str, Any]:
    """Return recent telemetry samples."""
    return telemetry.history(count)


@mcp.tool()
@tool_guard
def estimate_runtime_fps(samples: int = 10, interval: float = 0.1) -> dict[str, Any]:
    """Estimate actual game FPS by polling the in-game tick counter."""
    return telemetry.estimate_runtime_fps(samples, interval)


@mcp.tool()
@tool_guard
def estimate_display_fps(samples: int = 5, interval: float = 0.2) -> dict[str, Any]:
    """Roughly estimate display FPS by timing screenshot captures."""
    return telemetry.estimate_fps_from_screenshots(samples, interval)


@mcp.tool()
@tool_guard
def get_game_log(lines: int = 50) -> dict[str, Any]:
    """Return the last N lines of the game log."""
    return logs.tail(lines)


@mcp.tool()
@tool_guard
def read_game_log(offset: int = 0, limit: int = 200) -> dict[str, Any]:
    """Read a slice of the game log."""
    return logs.read(offset, limit)


@mcp.tool()
@tool_guard
def wait(seconds: float) -> dict[str, Any]:
    """Wait for the specified number of seconds."""
    import time
    time.sleep(seconds)
    return success(waited_seconds=seconds)


def main() -> None:
    # Make sure the virtual X display is available for tools that need it.
    loop = asyncio.new_event_loop()
    try:
        if not loop.run_until_complete(mgr.start_xvfb()):
            print("WARNING: Xvfb could not be started; some tools may fail", file=sys.stderr)
    except Exception as exc:
        print(f"WARNING: Failed to ensure Xvfb is running: {exc}", file=sys.stderr)

    print(f"hbtestd starting on {cfg.server_host}:{cfg.server_port}")
    print(f"SSE endpoint: {cfg.sse_url}")
    print("Available tools:", [t.__name__ for t in [
        start_game, stop_game, restart_game, get_status, get_health,
        screenshot, screenshot_base64, validate_screenshot,
        send_key, hold_key, send_combo, tap_pattern, list_keys,
        send_text, mouse_click, mouse_move,
        get_telemetry, get_telemetry_history, estimate_runtime_fps, estimate_display_fps,
        get_game_log, read_game_log, wait,
    ]])
    mcp.run(transport="sse")


if __name__ == "__main__":
    main()
