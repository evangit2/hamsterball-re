"""MCP server entry point."""
from __future__ import annotations

import asyncio
import struct
import sys
from typing import Any, Optional

from mcp.server.fastmcp import FastMCP

from . import fpsmod
from .addresses import list_known_symbols, get_struct_layout
from .capture import Capture
from .config import Config
from .errors import failure, success, tool_guard
from .gamemgr import GameManager
from .health import get_health as _get_health
from .input import InputDevice
from .logs import GameLog
from .memory import MemoryManager, _parse_addr
from .monitor import AddressMonitor, FreezeManager
from .telemetry import Telemetry


cfg = Config()
mgr = GameManager(cfg)
capture = Capture(cfg)
inputs = InputDevice(cfg)
telemetry = Telemetry(cfg)
logs = GameLog(cfg)

# Persistent monitor/freeze managers (created lazily with correct PID)
_monitor: Optional[AddressMonitor] = None
_freezer: Optional[FreezeManager] = None

mcp = FastMCP("hbtestd")
mcp.settings.host = cfg.server_host
mcp.settings.port = cfg.server_port


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _require_pid() -> int:
    pid = mgr._find_game_pid()
    if not pid:
        raise RuntimeError("game is not running")
    return pid


def _require_write_access() -> None:
    if not cfg.can_write_memory(mgr.started_by_hbtestd()):
        raise RuntimeError(
            "memory writes disabled; set HBTESTD_ALLOW_MEMORY_WRITE=1 or "
            "start the game via hbtestd"
        )


def _mm() -> MemoryManager:
    """Get a MemoryManager for the current game PID."""
    return MemoryManager(_require_pid())


def _get_monitor() -> AddressMonitor:
    global _monitor
    if _monitor is None or _monitor.mm.pid != _require_pid():
        _monitor = AddressMonitor(_mm())
    return _monitor


def _get_freezer() -> FreezeManager:
    global _freezer
    if _freezer is None or _freezer.mm.pid != _require_pid():
        _freezer = FreezeManager(_mm())
    return _freezer


# ---------------------------------------------------------------------------
# Game lifecycle tools
# ---------------------------------------------------------------------------

@mcp.tool()
@tool_guard
async def start_game(fps_mod: bool = False, target_fps: int = 144, render_fps: int = 144) -> dict[str, Any]:
    """Launch Hamsterball inside a virtual X display.

    Set fps_mod=True to install the bass.dll FPS mod before launching.
    target_fps and render_fps override the mod defaults when fps_mod is enabled.
    Note: FPS can also be patched at runtime via patch_game_fps without the mod.
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
    # Stop any active monitors/freezers
    global _monitor, _freezer
    if _monitor:
        _monitor.stop()
    if _freezer:
        _freezer.stop()
    return success(**await mgr.stop_game())


@mcp.tool()
@tool_guard
async def restart_game(fps_mod: bool = False, target_fps: int = 144, render_fps: int = 144) -> dict[str, Any]:
    """Restart Hamsterball cleanly."""
    global _monitor, _freezer
    if _monitor:
        _monitor.stop()
        _monitor = None
    if _freezer:
        _freezer.stop()
        _freezer = None
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


# ---------------------------------------------------------------------------
# Screenshot tools
# ---------------------------------------------------------------------------

@mcp.tool()
@tool_guard
def screenshot() -> dict[str, Any]:
    """Capture a screenshot of the game and return its filesystem path."""
    return capture.capture()


# --------------------------------------------------------------------------- #
# Level management tools
# --------------------------------------------------------------------------- #

@mcp.tool()
@tool_guard
def install_level(custom_path: str, level_slot: str = "Level1") -> dict[str, Any]:
    """Swap a custom MESHWORLD file into a level slot, backing up the original.

    Args:
        custom_path: Path to the .MESHWORLD file to install.
        level_slot: Level name to replace (e.g. Level1, Arena-WarmUp).
    """
    import os
    import shutil

    levels_dir = os.path.join(cfg.game_dir, "Levels")
    target = os.path.join(levels_dir, f"{level_slot}.MESHWORLD")
    backup = target + ".bak"

    if not os.path.exists(custom_path):
        return failure(f"custom level not found: {custom_path}")

    # Backup original if not already backed up
    if os.path.exists(target) and not os.path.exists(backup):
        shutil.copy2(target, backup)

    # Copy custom level into slot
    shutil.copy2(custom_path, target)
    return success(
        installed=custom_path,
        slot=level_slot,
        target=target,
        backup=backup if os.path.exists(backup) else None,
    )


@mcp.tool()
@tool_guard
def restore_level(level_slot: str = "Level1") -> dict[str, Any]:
    """Restore the original level file from backup.

    Args:
        level_slot: Level name to restore (e.g. Level1, Arena-WarmUp).
    """
    import os
    import shutil

    levels_dir = os.path.join(cfg.game_dir, "Levels")
    target = os.path.join(levels_dir, f"{level_slot}.MESHWORLD")
    backup = target + ".bak"

    if not os.path.exists(backup):
        return failure(f"no backup found for {level_slot}")

    shutil.copy2(backup, target)
    os.remove(backup)
    return success(restored=target, slot=level_slot)


# --------------------------------------------------------------------------- #
# Navigation tools
# --------------------------------------------------------------------------- #

@mcp.tool()
@tool_guard
async def navigate_to_race(
    wait_title: float = 35.0,
    key_delay: float = 5.0,
) -> dict[str, Any]:
    """Navigate the original Hamsterball menu to start a race.

    Assumes the game is already running. Sends the following key sequence:
      Space (title → main menu)
      → Return (LET'S PLAY → CHOOSE A GAME)
      → Return (select first game mode)
      → Down + Return (select NORMAL difficulty, if shown)
      → Mouse click at (600, 480) to hit PLAY!

    Args:
        wait_title: Seconds to wait for the title screen before navigating.
        key_delay: Seconds between key presses.
    """
    import asyncio

    steps = [
        # Wait for title screen
        ("wait", wait_title, "Waiting for title screen"),
        # Space → main menu
        ("key", "space", "Title → Main menu"),
        ("wait", key_delay, ""),
        # Return → CHOOSE A GAME
        ("key", "Return", "LET'S PLAY → CHOOSE A GAME"),
        ("wait", key_delay, ""),
        # Return → select game mode (Time Trials / Tournament)
        ("key", "Return", "Select game mode"),
        ("wait", key_delay, ""),
        # Down + Return → NORMAL difficulty (if shown)
        ("key", "Down", "Navigate to NORMAL"),
        ("wait", 2.0, ""),
        ("key", "Return", "Select NORMAL → Level intro"),
        ("wait", key_delay, ""),
        # Click PLAY! to start the race
        ("click", (600, 480), "Click PLAY! → Start race"),
    ]

    for step in steps:
        kind, value, desc = step
        if desc:
            print(f"  {desc}")
        if kind == "wait":
            await asyncio.sleep(value)
        elif kind == "key":
            inputs.send_key(value)
            await asyncio.sleep(0.5)
        elif kind == "click":
            inputs.click(value[0], value[1])
            await asyncio.sleep(0.5)

    # Wait for race to load
    await asyncio.sleep(15)
    result = capture.capture()
    return success(navigation="complete", screenshot=result)


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


# ---------------------------------------------------------------------------
# Input tools
# ---------------------------------------------------------------------------

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


# ---------------------------------------------------------------------------
# Telemetry tools
# ---------------------------------------------------------------------------

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


# ---------------------------------------------------------------------------
# Log tools
# ---------------------------------------------------------------------------

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


# ===========================================================================
# MEMORY TOOLS
# ===========================================================================
# The following tools provide full read/write/search/analyze capabilities
# for the live Hamsterball process memory.
#
# Supported types: u8, i8, u16, i16, u32, i32, u64, i64, float, double,
#                  bool, string, vec3, bytes
#
# Address specifications (used by resolve_address):
#   "0x005341E0"            -> absolute address
#   "RVA:0x1341E0"          -> module base + RVA
#   "module:Ntdll.dll:0x5000" -> specific module base + offset
#   "g_App"                 -> known global pointer address
#   "g_App+0x16C"           -> known address + offset
#   "app.target_fps"        -> dereference g_App, add offset
#   "scene.camera_angle"    -> dereference g_Scene, add offset
#   "ptr:0x005341E0"        -> dereference pointer at address
#   "chain:0x005341E0,0x10,0x20" -> multi-level pointer chain
#   "func.Ball_Ctor"        -> known function address
#   "vtable.Ball"           -> known vtable address
# ===========================================================================


@mcp.tool()
@tool_guard
def get_module_base(module_name: Optional[str] = None) -> dict[str, Any]:
    """Return the base address of a loaded module (default: Hamsterball.exe)."""
    mm = _mm()
    name = module_name or cfg.default_module_name
    base = mm.get_module_base(name)
    if base is None:
        return failure(f"module {name!r} not found")
    return success(module=name, base_address=f"0x{base:08X}", base=base)


@mcp.tool()
@tool_guard
def get_module_info(module_name: Optional[str] = None) -> dict[str, Any]:
    """Return full module info (base, end, size, regions)."""
    mm = _mm()
    name = module_name or cfg.default_module_name
    info = mm.get_module_info(name)
    if info is None:
        return failure(f"module {name!r} not found")
    return success(
        module=name,
        base=f"0x{info['base']:08X}",
        end=f"0x{info['end']:08X}",
        size=info["size"],
        region_count=len(info["regions"]),
    )


@mcp.tool()
@tool_guard
def list_memory_regions(module_name: Optional[str] = None) -> dict[str, Any]:
    """List readable memory regions of the game process."""
    mm = _mm()
    regions = mm.list_regions()
    if module_name:
        regions = [r for r in regions if module_name in r["path"]]
    return success(
        pid=mm.pid,
        count=len(regions),
        regions=[
            {
                "start": f"0x{r['start']:08X}",
                "end": f"0x{r['end']:08X}",
                "size": r["size"],
                "perms": r["perms"],
                "path": r["path"],
            }
            for r in regions[:200]
        ],
    )


@mcp.tool()
@tool_guard
def resolve_address(spec: str) -> dict[str, Any]:
    """Resolve an address specification to an absolute address.

    Supports:
      "0x005341E0"            -> absolute
      "RVA:0x1341E0"          -> base + RVA
      "module:Ntdll.dll:0x5000" -> module base + offset
      "g_App"                 -> known global pointer address (0x005341E0)
      "g_App+0x16C"           -> known address + offset
      "app.target_fps"        -> dereference g_App, add offset
      "scene.camera_angle"    -> dereference g_Scene, add offset
      "ptr:0x005341E0"        -> dereference pointer at address
      "chain:0x005341E0,0x10,0x20" -> multi-level pointer chain
      "func.Ball_Ctor"        -> known function address
      "vtable.Ball"           -> known vtable address
    """
    mm = _mm()
    addr = mm.resolve_address(spec)
    if addr is None:
        return failure(f"could not resolve address spec: {spec}")
    return success(spec=spec, address=f"0x{addr:08X}", address_int=addr)


@mcp.tool()
@tool_guard
def read_memory(
    address: str,
    size: int = 4,
    data_type: str = "bytes",
) -> dict[str, Any]:
    """Read memory from the live game process.

    address: hex string like '0x005341E0', decimal, or symbol spec
    data_type: bytes | u8 | i8 | u16 | i16 | u32 | i32 | u64 | i64 | float | double | bool | string | vec3
    size: only used for data_type='bytes' (default 4) or 'string' (max length)
    """
    mm = _mm()
    addr = mm.resolve_address(address) or _parse_addr(address)

    result: dict[str, Any] = {
        "success": True,
        "pid": mm.pid,
        "address": f"0x{addr:08X}",
        "data_type": data_type,
    }

    if data_type == "bytes":
        raw = mm.read_bytes(addr, size)
        result["size"] = len(raw)
        result["hex"] = raw.hex()
        result["value"] = raw.hex()
    elif data_type == "string":
        s = mm.read_string(addr, max_length=size if size > 0 else 256)
        result["value"] = s
        result["size"] = len(s)
    elif data_type == "vec3":
        x, y, z = mm.read_vec3(addr)
        result["value"] = {"x": x, "y": y, "z": z}
        result["size"] = 12
    else:
        value = mm.read_typed(addr, data_type)
        result["value"] = value
        fmt_size = {"u8": 1, "i8": 1, "u16": 2, "i16": 2, "u32": 4, "i32": 4,
                    "u64": 8, "i64": 8, "float": 4, "double": 8, "bool": 1}
        result["size"] = fmt_size.get(data_type, 4)

    return result


@mcp.tool()
@tool_guard
def write_memory(
    address: str,
    value: Any,
    data_type: str = "u32",
) -> dict[str, Any]:
    """Write a value into the live game process memory.

    address: hex string like '0x004FD680', decimal, or symbol spec
    data_type: u8 | i8 | u16 | i16 | u32 | i32 | u64 | i64 | float | double | bool | string | vec3 | bytes
    value: the value to write (vec3 accepts [x, y, z] or {"x":..,"y":..,"z":..})
    """
    _require_write_access()
    mm = _mm()
    addr = mm.resolve_address(address) or _parse_addr(address)

    n = mm.write_typed(addr, value, data_type)

    # Read back to verify
    try:
        verify = mm.read_typed(addr, data_type) if data_type != "bytes" else None
    except Exception:
        verify = None

    return success(
        pid=mm.pid,
        address=f"0x{addr:08X}",
        bytes_written=n,
        data_type=data_type,
        written_value=value,
        readback=verify,
    )


@mcp.tool()
@tool_guard
def read_batch(reads: list[dict]) -> dict[str, Any]:
    """Read multiple addresses in one call.

    Each item: {"label": "ball_x", "address": "0x...", "type": "float"}
    Returns: {"results": [{"label": ..., "address": ..., "value": ..., "type": ...}, ...]}
    """
    mm = _mm()
    # Resolve symbol specs to absolute addresses
    for item in reads:
        if isinstance(item.get("address"), str):
            resolved = mm.resolve_address(item["address"])
            if resolved is not None:
                item["address"] = resolved
    results = mm.read_batch(reads)
    return success(pid=mm.pid, count=len(results), results=results)


@mcp.tool()
@tool_guard
def write_batch(writes: list[dict]) -> dict[str, Any]:
    """Write multiple addresses in one call.

    Each item: {"address": "0x...", "value": 42, "type": "u32"}
    Returns: status for each write
    """
    _require_write_access()
    mm = _mm()
    for item in writes:
        if isinstance(item.get("address"), str):
            resolved = mm.resolve_address(item["address"])
            if resolved is not None:
                item["address"] = resolved
    results = mm.write_batch(writes)
    return success(pid=mm.pid, count=len(results), results=results)


@mcp.tool()
@tool_guard
def dump_hex(address: str, size: int = 64) -> dict[str, Any]:
    """Hex dump of memory at the given address.

    Returns hex and ASCII representation, 16 bytes per line.
    """
    mm = _mm()
    addr = mm.resolve_address(address) or _parse_addr(address)
    return success(**mm.dump_hex(addr, size))


@mcp.tool()
@tool_guard
def find_memory_pattern(
    pattern_hex: str,
    module_name: Optional[str] = None,
    max_results: int = 100,
) -> dict[str, Any]:
    """Search for a byte pattern in readable game memory.

    Use '??' for wildcards, e.g. '89 ?? 05'.
    """
    mm = _mm()
    # Normalize ?? to ? for our matcher
    pattern_hex = pattern_hex.replace("??", "?")
    try:
        pattern = bytes.fromhex(pattern_hex.replace("?", "0"))
    except ValueError as e:
        return failure(f"invalid hex pattern: {e}")
    results = mm.find_pattern(pattern, module_name=module_name, max_results=max_results)
    return success(
        pid=mm.pid,
        pattern=pattern_hex,
        matches=[f"0x{addr:08X}" for addr in results],
        count=len(results),
    )


@mcp.tool()
@tool_guard
def scan_value(
    value: Any,
    data_type: str = "u32",
    module_name: Optional[str] = None,
    max_results: int = 200,
) -> dict[str, Any]:
    """Scan memory for all addresses containing a specific value.

    This is the first pass of a Cheat Engine style value search.
    Use scan_next to narrow results.

    Args:
        value: the value to search for
        data_type: u32 | i32 | u16 | i16 | u8 | i8 | u64 | i64 | float | double
        module_name: restrict to a specific module (e.g. 'Hamsterball.exe')
        max_results: stop after this many hits
    """
    mm = _mm()
    results = mm.scan_value(value, data_type, module_name=module_name, max_results=max_results)
    return success(
        pid=mm.pid,
        searched_value=value,
        data_type=data_type,
        matches=[{"address": f"0x{r['address']:08X}", "value": r["value"]} for r in results],
        count=len(results),
    )


@mcp.tool()
@tool_guard
def scan_next(
    value: Any,
    prev_addresses: list[str],
    data_type: str = "u32",
    max_results: int = 200,
) -> dict[str, Any]:
    """Narrow down previous scan results by checking which still match a new value.

    Pass the addresses from a previous scan_value or scan_next call.
    """
    mm = _mm()
    # Parse address strings to ints
    addrs = []
    for a in prev_addresses:
        a_int = mm.resolve_address(a)
        if a_int is None:
            a_int = _parse_addr(a)
        addrs.append(a_int)

    results = mm.scan_next(value, data_type, prev_results=addrs, max_results=max_results)
    return success(
        pid=mm.pid,
        searched_value=value,
        data_type=data_type,
        prev_count=len(prev_addresses),
        matches=[{"address": f"0x{r['address']:08X}", "value": r["value"]} for r in results],
        count=len(results),
    )


@mcp.tool()
@tool_guard
def scan_float_range(
    low: float,
    high: float,
    module_name: Optional[str] = None,
    max_results: int = 200,
) -> dict[str, Any]:
    """Scan memory for floats within a range [low, high].

    Useful for finding health/speed/timer values when you know the approximate range.
    """
    mm = _mm()
    results = mm.scan_float_range(low, high, module_name=module_name, max_results=max_results)
    return success(
        pid=mm.pid,
        range=[low, high],
        matches=[{"address": f"0x{r['address']:08X}", "value": r["value"]} for r in results],
        count=len(results),
    )


@mcp.tool()
@tool_guard
def resolve_pointer_chain(base: str, offsets: list[int]) -> dict[str, Any]:
    """Follow a multi-level pointer chain to a final address.

    base: starting address (hex string or symbol)
    offsets: list of offsets applied at each level

    Example: base="0x005341E0", offsets=[0x10, 0x20]
    Reads [0x005341E0] -> ptr, ptr+0x10 -> ptr2, ptr2+0x20 -> final address
    """
    mm = _mm()
    base_addr = mm.resolve_address(base) or _parse_addr(base)
    try:
        final = mm.resolve_pointer_chain(base_addr, offsets)
        return success(
            base=f"0x{base_addr:08X}",
            offsets=[f"0x{o:X}" for o in offsets],
            final_address=f"0x{final:08X}",
            final_address_int=final,
        )
    except Exception as e:
        return failure(str(e))


@mcp.tool()
@tool_guard
def read_pointer(address: str) -> dict[str, Any]:
    """Read a 32-bit pointer value from memory."""
    mm = _mm()
    addr = mm.resolve_address(address) or _parse_addr(address)
    ptr = mm.read_pointer(addr)
    return success(
        address=f"0x{addr:08X}",
        pointer_value=f"0x{ptr:08X}",
        pointer_int=ptr,
    )


@mcp.tool()
@tool_guard
def list_known_addresses() -> dict[str, Any]:
    """Return all known Hamsterball addresses, offsets, and symbols.

    Includes: globals (g_App, g_Scene), struct layouts (App, Ball, Scene,
    RumbleBoard, SceneObject, Gadget), function addresses, and vtable addresses.
    """
    return success(**list_known_symbols())


@mcp.tool()
@tool_guard
def get_struct_info(struct_name: str) -> dict[str, Any]:
    """Return the field layout for a known struct.

    Valid struct_name: App | Ball | Scene | RumbleBoard | SceneObject | Gadget
    """
    layout = get_struct_layout(struct_name)
    if layout is None:
        return failure(f"unknown struct: {struct_name}")
    return success(
        struct=struct_name,
        fields={k: f"0x{v:X}" for k, v in layout.items()},
        field_count=len(layout),
    )


# ---------------------------------------------------------------------------
# Address monitoring tools (time-series sampling)
# ---------------------------------------------------------------------------

@mcp.tool()
@tool_guard
def monitor_address(name: str, address: str, data_type: str = "u32") -> dict[str, Any]:
    """Add an address to monitor. Samples are taken periodically.

    name: a label for this watch (e.g. 'ball_x')
    address: hex string or symbol spec
    data_type: type to sample (u32, float, vec3, etc.)
    """
    mon = _get_monitor()
    mm = mon.mm
    addr = mm.resolve_address(address) or _parse_addr(address)
    mon.add_watch(name, addr, data_type)
    return success(
        name=name,
        address=f"0x{addr:08X}",
        data_type=data_type,
        monitor_running=mon.is_running(),
    )


@mcp.tool()
@tool_guard
def unmonitor_address(name: str) -> dict[str, Any]:
    """Remove an address from monitoring."""
    mon = _get_monitor()
    removed = mon.remove_watch(name)
    if not removed:
        return failure(f"watch {name!r} not found")
    return success(removed=name)


@mcp.tool()
@tool_guard
def list_monitors() -> dict[str, Any]:
    """List all active address monitors."""
    mon = _get_monitor()
    return success(watches=mon.list_watches(), running=mon.is_running())


@mcp.tool()
@tool_guard
def get_monitor_history(name: str, count: int = 50) -> dict[str, Any]:
    """Return time-series history for a monitored address."""
    mon = _get_monitor()
    history = mon.get_history(name, count)
    return success(name=name, samples=history, count=len(history))


@mcp.tool()
@tool_guard
def monitor_start(interval: float = 0.25) -> dict[str, Any]:
    """Start background monitoring of all watched addresses."""
    mon = _get_monitor()
    mon.interval = interval
    mon.start()
    return success(interval=interval, watches=mon.list_watches())


@mcp.tool()
@tool_guard
def monitor_stop() -> dict[str, Any]:
    """Stop background monitoring."""
    mon = _get_monitor()
    mon.stop()
    return success(stopped=True)


@mcp.tool()
@tool_guard
def monitor_sample() -> dict[str, Any]:
    """Take a single sample of all monitored addresses (manual, no background thread)."""
    mon = _get_monitor()
    snapshot = mon.sample_once()
    return success(
        sampled={
            name: {"value": e.get("value"), "ok": e.get("ok", False), "error": e.get("error")}
            for name, e in snapshot.items()
        }
    )


# ---------------------------------------------------------------------------
# Value freeze tools (continuously pin a value)
# ---------------------------------------------------------------------------

@mcp.tool()
@tool_guard
def freeze_address(
    name: str,
    address: str,
    value: Any,
    data_type: str = "u32",
    interval_ms: int = 20,
) -> dict[str, Any]:
    """Freeze an address to a specific value.

    A background thread continuously writes the value every interval_ms,
    preventing the game from changing it.

    name: label for this freeze (e.g. 'inf_lives')
    address: hex string or symbol spec
    value: the value to pin
    data_type: type of the value
    interval_ms: how often to re-write (default 20ms)
    """
    _require_write_access()
    fz = _get_freezer()
    fz.interval = interval_ms / 1000.0
    mm = fz.mm
    addr = mm.resolve_address(address) or _parse_addr(address)
    fz.freeze(name, addr, value, data_type)
    return success(
        name=name,
        address=f"0x{addr:08X}",
        value=value,
        data_type=data_type,
        interval_ms=interval_ms,
        frozen=True,
    )


@mcp.tool()
@tool_guard
def unfreeze_address(name: str) -> dict[str, Any]:
    """Stop freezing an address."""
    _require_write_access()
    fz = _get_freezer()
    removed = fz.unfreeze(name)
    if not removed:
        return failure(f"freeze {name!r} not found")
    return success(unfrozen=name)


@mcp.tool()
@tool_guard
def unfreeze_all() -> dict[str, Any]:
    """Stop freezing all addresses."""
    _require_write_access()
    fz = _get_freezer()
    count = len(fz._frozen)
    fz.unfreeze_all()
    return success(unfrozen_count=count)


@mcp.tool()
@tool_guard
def list_frozen() -> dict[str, Any]:
    """List all frozen addresses."""
    fz = _get_freezer()
    return success(frozen=fz.list_frozen(), running=fz.is_running())


# ---------------------------------------------------------------------------
# Hamsterball-specific convenience tools
# ---------------------------------------------------------------------------

@mcp.tool()
@tool_guard
def patch_game_fps(target_fps: int, render_fps: int) -> dict[str, Any]:
    """Patch FPS targets in the running App singleton.

    Writes to App+0x16C (target_fps) and App+0x170 (render_fps) directly.
    No DLL mod required — patches live memory.
    """
    _require_write_access()
    mm = _mm()

    APP_GLOBAL_PTR = 0x005341E0
    APP_TARGET_FPS_OFFSET = 0x16C
    APP_RENDER_FPS_OFFSET = 0x170

    app_ptr_bytes = mm.read_bytes(APP_GLOBAL_PTR, 4)
    app_addr = struct.unpack("<I", app_ptr_bytes)[0]
    if not app_addr:
        return failure("App pointer is null")

    mm.write_u32(app_addr + APP_TARGET_FPS_OFFSET, int(target_fps))
    mm.write_u32(app_addr + APP_RENDER_FPS_OFFSET, int(render_fps))

    return success(
        pid=mm.pid,
        app_address=f"0x{app_addr:08X}",
        target_fps=mm.read_u32(app_addr + APP_TARGET_FPS_OFFSET),
        render_fps=mm.read_u32(app_addr + APP_RENDER_FPS_OFFSET),
    )


@mcp.tool()
@tool_guard
def read_app_state() -> dict[str, Any]:
    """Read key fields from the live App struct.

    Returns target_fps, render_fps, last_frame_tick, difficulty,
    and player scores in one call.
    """
    mm = _mm()
    from .addresses import APP, G_APP_PTR

    app_ptr = mm.read_u32(G_APP_PTR)
    if not app_ptr:
        return failure("App pointer is null (game may not be fully initialized)")

    result: dict[str, Any] = {"app_ptr": f"0x{app_ptr:08X}"}

    # Read all known App fields
    for field, offset in APP.items():
        try:
            if field in ("target_fps", "render_fps", "last_frame_tick",
                         "difficulty", "control1", "control2", "control3", "control4"):
                result[field] = mm.read_u32(app_ptr + offset)
            elif field in ("player_score", "max_score"):
                result[field] = mm.read_float(app_ptr + offset)
            elif field == "is_paused":
                result[field] = mm.read_bool(app_ptr + offset)
            else:
                result[field] = mm.read_u32(app_ptr + offset)
        except Exception as e:
            result[field] = f"error: {e}"

    return success(**result)


@mcp.tool()
@tool_guard
def read_ball_state(ball_index: int = 0) -> dict[str, Any]:
    """Read key fields from a Ball struct.

    Resolves the ball list from Scene+0x29D4 (AthenaList) and reads
    position, velocity, radius, max_speed, is_8ball, etc.

    Args:
        ball_index: which ball to read (0=player 1, 1=player 2, etc.)
    """
    mm = _mm()
    from .addresses import BALL, G_APP_PTR, SCENE

    # Try to get Scene pointer from g_Scene
    scene_ptr = mm.read_u32(0x005341E4)
    if not scene_ptr:
        # Try from App (App often holds a Scene reference)
        app_ptr = mm.read_u32(G_APP_PTR)
        if not app_ptr:
            return failure("App pointer is null")
        # Scene is typically at a high App offset - try scanning
        # For now, try the global
        return failure("Scene pointer is null (g_Scene not set)")

    # Read AthenaList at Scene+0x29D4
    # AthenaList layout: ptr to array, count, capacity (3 u32s)
    list_addr = scene_ptr + SCENE["ball_list"]
    list_data = mm.read_bytes(list_addr, 12)
    arr_ptr, count, capacity = struct.unpack("<III", list_data)

    if not arr_ptr or count == 0:
        return failure("ball list is empty or null")

    if ball_index >= count:
        return failure(f"ball_index {ball_index} out of range (count={count})")

    # Read the ball pointer at arr_ptr + ball_index * 4
    ball_ptr_addr = arr_ptr + ball_index * 4
    ball_ptr = mm.read_u32(ball_ptr_addr)
    if not ball_ptr:
        return failure(f"ball {ball_index} pointer is null")

    result: dict[str, Any] = {
        "ball_index": ball_index,
        "ball_ptr": f"0x{ball_ptr:08X}",
        "ball_count": count,
    }

    # Read Ball fields
    for field, offset in BALL.items():
        try:
            if field == "vtable":
                result[field] = f"0x{mm.read_u32(ball_ptr + offset):08X}"
            elif field in ("position", "velocity", "gravity"):
                x, y, z = mm.read_vec3(ball_ptr + offset)
                result[field] = {"x": round(x, 2), "y": round(y, 2), "z": round(z, 2)}
            elif field in ("radius", "max_speed", "friction", "speed_scale"):
                result[field] = round(mm.read_float(ball_ptr + offset), 4)
            elif field in ("is_8ball", "is_active"):
                result[field] = mm.read_bool(ball_ptr + offset)
            elif field == "best_streak":
                result[field] = mm.read_i32(ball_ptr + offset)
            else:
                result[field] = mm.read_u32(ball_ptr + offset)
        except Exception as e:
            result[field] = f"error: {e}"

    return success(**result)


# ---------------------------------------------------------------------------
# Utility
# ---------------------------------------------------------------------------

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
        # Game lifecycle
        start_game, stop_game, restart_game, get_status, get_health,
        # Screenshots
        screenshot, screenshot_base64, validate_screenshot,
        # Input
        send_key, hold_key, send_combo, tap_pattern, list_keys,
        send_text, mouse_click, mouse_move,
        # Telemetry
        get_telemetry, get_telemetry_history, estimate_runtime_fps, estimate_display_fps,
        # Logs
        get_game_log, read_game_log,
        # Memory - basic
        get_module_base, get_module_info, list_memory_regions,
        resolve_address, read_memory, write_memory, read_batch, write_batch,
        dump_hex, find_memory_pattern,
        # Memory - scanning
        scan_value, scan_next, scan_float_range,
        # Memory - pointers
        resolve_pointer_chain, read_pointer,
        # Memory - symbols
        list_known_addresses, get_struct_info,
        # Memory - monitoring
        monitor_address, unmonitor_address, list_monitors,
        get_monitor_history, monitor_start, monitor_stop, monitor_sample,
        # Memory - freezing
        freeze_address, unfreeze_address, unfreeze_all, list_frozen,
        # Hamsterball-specific
        patch_game_fps, read_app_state, read_ball_state,
        # Utility
        wait,
    ]])
    mcp.run(transport="sse")


if __name__ == "__main__":
    main()
