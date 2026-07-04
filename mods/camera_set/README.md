# Camera Set

Manually control camera position, rotation, zoom, pitch, and offset via the Options menu.

## Sliders

| Slider | Range | Default | Description |
|--------|-------|---------|-------------|
| Orbit Angle (rad) | -6.28 to 6.28 | 2.356 (135°) | Y-axis rotation around the ball |
| Zoom Distance | 100-3000 | 1100 | Distance from camera to ball |
| Pitch (elevation) | 0.1-3.0 | 0.9 | Camera height angle (higher = more top-down) |
| Offset X | -500 to 500 | 0 | Lateral camera offset |
| Offset Y | -500 to 500 | 0 | Vertical camera offset |
| Offset Z | -500 to 500 | 0 | Forward/back camera offset |

## Toggle

| Toggle | Default | Description |
|--------|---------|-------------|
| Custom Camera | NO | Enable/disable custom camera values |

## How It Works

The mod writes camera fields on a background thread (16ms interval) to the Scene struct:

- **Scene+0x29BC** — orbit angle (yaw rotation)
- **Scene+0x29C0** — orbit distance (zoom)
- **Scene+0x4384** — smooth distance target (prevents Camera_SmoothDistance from fighting us)
- **Scene+0x434C/4350/4354** — camera offset X/Y/Z

Pitch is patched via `PatchMemory` at address `0x41A2B5` — the hardcoded `0.9f` immediate in `Scene_SetCamera` (0x419FA0) that sets the orbit direction Y component.

When disabled, all values revert to game defaults.

## Technical Details

- **Pitch address**: `0x41A2B5` (imm32 within `MOV dword [ESP+0x2C], 0x3F666666` at `0x41A2B1`)
- **Camera_SmoothDistance** (0x4135F4) runs per-frame and writes both `Scene+0x29C0` and `Scene+0x4384`, so the background thread must write both to override it
- Scene pointer obtained via `App→+0x220→+0x0C→Board→+0x878→Scene`
- All scene writes guarded with `IsBadWritePtr`

## Build

```bash
i686-w64-mingw32-g++ -shared -o CameraSet.dll CameraSet.cpp \
  -Idocs/agent-knowledge -std=c++17 -Wall -static
```
