# hbtestd — Automated Hamsterball Testing Daemon

An MCP server that runs Hamsterball under Wine/Xvfb and exposes tools for screenshots, input, and live runtime telemetry.

## What It Does

- Starts a virtual X display (`Xvfb :99`)
- Launches the original `Hamsterball.exe` under Wine with `LIBGL_ALWAYS_SOFTWARE=1`
- Exposes an MCP server with HTTP/SSE transport on `http://127.0.0.1:8777/sse`
- Provides tools to:
  - Start / stop / restart the game
  - Capture screenshots on demand
  - Send keyboard input (arrows, Enter, Escape, Space, WASD, etc.)
  - Click at screen coordinates
  - Read runtime telemetry (process stats + internal FPS values)
  - Estimate FPS from frame capture timing

## Install

```bash
cd /home/evan/hamsterball-re/tools/hbtestd
uv venv
uv pip install -e .
```

## Run

```bash
./run_server.sh
```

## Connect from Hermes

Add to `~/.hermes/config.yaml` under `mcp_servers`:

```yaml
mcp_servers:
  hbtestd:
    transport: sse
    url: http://127.0.0.1:8777/sse
```

Then restart Hermes or reload MCP servers.

## Tools

| Tool | Description |
|------|-------------|
| `start_game()` | Launch Hamsterball |
| `stop_game()` | Kill game + Wine + Xvfb |
| `restart_game()` | Clean restart |
| `get_status()` | PID, CPU, memory, runtime |
| `screenshot()` | Save PNG and return path |
| `screenshot_base64()` | Return PNG as base64 |
| `send_key(key)` | Single keypress |
| `hold_key(key, duration_ms)` | Hold key |
| `send_text(text)` | Type text |
| `mouse_click(x, y, button)` | Click at coordinates |
| `get_telemetry()` | Internal FPS targets & process stats |
| `estimate_fps(samples)` | Approximate FPS from captures |
| `wait(seconds)` | Sleep |

## Multi-User Slots

Several users can run live games at once. Each *slot* is an isolated
`{display, game_dir, screenshot/log paths}` instance; every tool takes an
optional `slot` parameter (default `"default"`, which preserves the old
single-user behavior exactly).

```python
start_game(slot="bob")          # launch bob's game
read_memory(address="0x4FDEEC", data_type="u32", slot="bob")
screenshot(slot="bob")          # -> /tmp/hbtestd_screenshot_bob.png
stop_game(slot="bob")           # kills only bob's game
list_slots()                    # configured slots + live status
```

Extra slots come from `HBTESTD_SLOTS` JSON in the server's environment
(unknown keys are rejected at startup):

```bash
HBTESTD_SLOTS='{"bob": {"display": ":100", "game_dir": "/home/evan/hb_bob"}}'
```

Slot safety rules (enforced in code, not convention):
- PID lookups are scoped to the slot's game dir (exact exe path in cmdline
  or process cwd); a bare `Hamsterball.exe` name match never counts, so one
  slot can never latch onto another user's game. Zombies are ignored.
- Launched PIDs are pinned; lookups prefer the pin while it is alive.
- `stop_game` / crash-test cleanup kill only their own process tree.
  `wineserver -k` runs only when zero game processes remain system-wide.
- Optional per-slot `wineprefix` isolates Wine prefixes between slots.

## Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `HBTESTD_GAME_DIR` | `/home/evan/hamsterball-wasm/boxedwine-package/hamsterball` | Game install directory |
| `HBTESTD_GAME_EXE` | `Hamsterball.exe` | Game executable name |
| `HBTESTD_DISPLAY` | `:99` | Virtual display number |
| `HBTESTD_RESOLUTION` | `800x600x24` | Xvfb screen spec |
| `HBTESTD_PORT` | `8777` | MCP SSE port |
| `HBTESTD_SCREENSHOT_PATH` | `/tmp/hbtestd_screenshot.png` | Default screenshot file |
| `HBTESTD_LOG_PATH` | `/tmp/hbtestd.log` | Game stdout/stderr log |
| `HBTESTD_LIBGL_SOFTWARE` | `1` | Force llvmpipe software rendering |
| `HBTESTD_SLOTS` | _(unset)_ | Extra slots JSON: `{"name": {"display", "game_dir", ...}}` |
| `HBTESTD_WINEPREFIX` | _(unset)_ | Isolated Wine prefix for the default slot |

## Runtime Telemetry

`get_telemetry()` reads from the Windows PE static addresses running inside the Wine process:

- `g_App` at `0x005341E0`
- `App+0x16C` → target update FPS (default 100)
- `App+0x170` → target render FPS (default 75)

This requires ptrace access to the Wine process. If `kernel.yama.ptrace_scope` blocks it, you will still get process statistics but not the internal FPS values.

## Requirements

- Wine
- Xvfb
- xdotool
- scrot
- Python ≥3.10
- MinGW cross compiler (if you want to build the FPS unlock DLL separately)
