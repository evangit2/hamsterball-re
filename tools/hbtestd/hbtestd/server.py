"""MCP server entry point."""
import asyncio
import sys
from typing import Any

from mcp.server.fastmcp import FastMCP

from .capture import Capture
from .config import Config
from .gamemgr import GameManager
from .input import InputDevice
from .telemetry import Telemetry


cfg = Config()
mgr = GameManager(cfg)
capture = Capture(cfg)
inputs = InputDevice(cfg)
telemetry = Telemetry(cfg)

mcp = FastMCP("hbtestd")
mcp.settings.host = cfg.server_host
mcp.settings.port = cfg.server_port


@mcp.tool()
async def start_game() -> dict[str, Any]:
    """Launch Hamsterball inside a virtual X display."""
    return await mgr.start_game()


@mcp.tool()
async def stop_game() -> dict[str, Any]:
    """Kill the Hamsterball process and clean up Wine/Xvfb."""
    return await mgr.stop_game()


@mcp.tool()
async def restart_game() -> dict[str, Any]:
    """Restart Hamsterball cleanly."""
    await mgr.stop_game()
    await asyncio.sleep(0.5)
    return await mgr.start_game()


@mcp.tool()
def get_status() -> dict[str, Any]:
    """Return current game process status."""
    return mgr.get_status()


@mcp.tool()
def screenshot() -> dict[str, Any]:
    """Capture a screenshot of the game and return its filesystem path."""
    return capture.capture()


@mcp.tool()
def screenshot_base64() -> dict[str, Any]:
    """Capture a screenshot and return it as base64 image data."""
    return capture.get_base64()


@mcp.tool()
def send_key(key: str, duration_ms: int = 50) -> dict[str, Any]:
    """Send a single keypress to the game window.

    Valid keys include: Left, Right, Up, Down, Return, Escape, space, w, a, s, d.
    """
    return inputs.send_key(key, duration_ms)


@mcp.tool()
def hold_key(key: str, duration_ms: int = 500) -> dict[str, Any]:
    """Hold a key down for duration_ms milliseconds."""
    return inputs.hold_key(key, duration_ms)


@mcp.tool()
def send_text(text: str) -> dict[str, Any]:
    """Type a string of text into the game window."""
    return inputs.send_text(text)


@mcp.tool()
def mouse_click(x: int, y: int, button: int = 1) -> dict[str, Any]:
    """Click at screen coordinates (x, y)."""
    return inputs.click(x, y, button)


@mcp.tool()
def get_telemetry() -> dict[str, Any]:
    """Read runtime info from the Hamsterball process (FPS targets, memory, CPU)."""
    return telemetry.get()


@mcp.tool()
def estimate_fps(samples: int = 5, interval: float = 0.2) -> dict[str, Any]:
    """Roughly estimate display FPS by timing screenshot captures."""
    return telemetry.estimate_fps_from_screenshots(samples, interval)


@mcp.tool()
def wait(seconds: float) -> dict[str, Any]:
    """Wait for the specified number of seconds."""
    import time
    time.sleep(seconds)
    return {"success": True, "waited_seconds": seconds}


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
        start_game, stop_game, restart_game, get_status, screenshot,
        screenshot_base64, send_key, hold_key, send_text, mouse_click,
        get_telemetry, estimate_fps, wait,
    ]])
    mcp.run(transport="sse")


if __name__ == "__main__":
    main()
