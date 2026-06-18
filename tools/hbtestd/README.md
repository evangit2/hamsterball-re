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
