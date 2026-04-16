# Hamsterball Screenshot Comparison - April 16, 2026

## Test Environment
- Wine 9.0 on Linux
- Xvfb :99 (1024x768x24) — no real GPU
- MinGW cross-compiled i686-w64-mingw32

## Original Game (Hamsterball.exe)
- **File**: `original_fullscreen.png`
- **Result**: D3DERR_NOTAVAILABLE error dialog
- Wine's D3D8 implementation requires a real GPU for the original game's
  fullscreen hardware vertex processing mode

## Reimplementation (hamsterball.exe)
- **File**: `reimpl.png`
- **Result**: D3D8 device created successfully, window shows clear color
- Our reimpl uses Windowed mode with software vertex processing fallback,
  so it initializes but shows blank (purple-gray) screen because the
  game loop rendering hasn't been wired up with actual geometry yet
- Console logs: "[D3D8] Device created, 1024x768", "[Load] Arena-WarmUp: 17 objects",
  "[Load] Ball at (184.9, 26.4, 183.4)"

## API Call Comparison (from Wine traces)

### Original Game D3D8 Calls
- Direct3DCreate8(D3D_SDK_VERSION)
- GetAdapterDisplayMode(D3DADAPTER_DEFAULT)
- CreateDevice(HAL, HARDWARE_VERTEXPROCESSING) → FAILS on Xvfb
- Also calls: BASS_Init, BASS_MusicLoad, DirectSoundCreate8, DirectInput8Create

### Our Reimplementation D3D8 Calls
- Direct3DCreate8(D3D_SDK_VERSION) ✓
- GetAdapterDisplayMode(D3DADAPTER_DEFAULT) ✓
- CreateDevice(HAL, HARDWARE_VERTEXPROCESSING) → fails
- CreateDevice(HAL, SOFTWARE_VERTEXPROCESSING) → succeeds ✓
- SetRenderState calls (ZENABLE, ZWRITEENABLE, LIGHTING, AMBIENT, SHADEMODE, etc.) ✓
- SetLight(0, directional) ✓
- DirectInput8Create → keyboard + mouse ✓
- DirectSoundCreate8 → fails gracefully (non-fatal) ✓

## Next Steps
1. Run on real GPU (not Xvfb) for full rendering comparison
2. Add Wine D3D8 call tracing wrapper (WINEDEBUG=+d3d8) to capture exact call sequence
3. Compare our CreateDevice params, SetRenderState sequence, and SetTransform calls
4. Implement MESHWORLD geometry rendering to fill the blank screen