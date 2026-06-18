# Hamsterball FPS Unlock

Removes the game's built-in frame rate caps (100 Hz update / 75 Hz render).

## What It Does

Hamsterball has two FPS caps:

1. **Update cap**: 100 Hz (`App+0x16C` = 100, frame_time = 1000/100 = 10ms)
2. **Render cap**: 75 Hz (`App+0x170` = 75, render_time = 1000/75 = 13ms)

The game loop in `App_Run` (0x46BD80) checks if enough time has passed since the last update/render. If not, it skips the frame and calls `Sleep(0)`. There's also a conditional jump (`JBE` at 0x46BF55) that skips the `Present` call if the render rate isn't met.

This DLL:
- Sets `App+0x16C` = 1000 (update cap → 1000 Hz)
- Sets `App+0x170` = 1000 (render cap → 1000 Hz)  
- NOPs out the render-skip `JBE` at 0x46BF55 (always render)

## Files

| File | Description |
|------|-------------|
| `fps_unlock.dll` | The unlock DLL — inject into Hamsterball.exe |
| `fps_unlock.log` | Created on first run — confirms patches applied |

## Usage

### Method 1: DLL Injection (recommended)

Use any DLL injector (including the one in `tools/collision_hook/`):

1. Copy `fps_unlock.dll` to your Hamsterball game folder
2. Launch `Hamsterball.exe`
3. Inject `fps_unlock.dll` using your preferred injector
4. Check `fps_unlock.log` to confirm patches applied

### Method 2: D3D8 Proxy (auto-load)

Rename `fps_unlock.dll` to `d3d8.dll` and place it next to `Hamsterball.exe`. The game will auto-load it on startup. The DLL forwards all D3D8 calls to the real d3d8.dll while applying the FPS patch.

*(Note: proxy mode requires additional forwarding code — see the d3d8_proxy_logger tool for an example of how to build a full proxy DLL.)*

## What Changed (Technical Details)

### App_Ctor (0x46DC40) — Default Values

```c
param_1[0x5b] = 100;   // App+0x16C = target FPS (100 Hz)
param_1[0x5c] = 0x4b;  // App+0x170 = render FPS (75 Hz)
```

### App_Run (0x46BD80) — Frame Timing

```c
// Update cap: 1000 / App+0x16C = 10ms per update tick
frame_time = 1000 / app->target_fps;   // 0x46BDF5: IDIV [ESI+0x16C]

// Render cap: 1000 / App+0x170 = 13ms per render
render_time = 1000 / app->render_fps;  // 0x46BDAC: IDIV [ESI+0x170]

// Render-skip check at 0x46BF55:
// JBE 0x46BFB4  → skip Present if not enough time elapsed
// Patched to: NOP NOP (always render)
```

### Patches Applied

| Address | Original | Patched | Description |
|---------|----------|---------|-------------|
| `App+0x16C` | `100` (0x64) | `1000` (0x3E8) | Update FPS cap |
| `App+0x170` | `75` (0x4B) | `1000` (0x3E8) | Render FPS cap |
| `0x46BF55` | `76 5D` (JBE) | `90 90` (NOP) | Render-skip conditional |

## Limitations

- **Vsync**: The game uses D3D8 Present with default presentation interval. If your GPU driver forces vsync, the actual FPS will be limited to your monitor refresh rate. Disable vsync in your GPU control panel for true uncapped FPS.
- **Physics**: The game's physics simulation runs at the update rate. At very high FPS, physics may behave differently (faster/smoother). The game was designed for 100 Hz max.
- **Menu**: The FPS unlock affects both gameplay and menus.

## Building

```bash
make
```

Cross-compiled with MinGW i686 (32-bit Windows target).
