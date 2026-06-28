# Sharp Textures Mod

Force custom texture filtering in Hamsterball.

## What It Does

D3D8's default texture filtering is LINEAR (smooth/blurry). This mod hooks
`Graphics_BeginFrame` (called every frame) and overrides the filtering on all
8 texture stages using `SetTextureStageState`.

## Config File

`sharp_textures.txt` (next to `bass.dll`), auto-generated on first run:

```ini
# Sharp Textures config
# Values: 0=none, 1=point(sharp), 2=linear(smooth), 3=anisotropic

MAGFILTER = 1
MINFILTER = 1
MIPFILTER = 1
```

### Filter Values

| Value | Constant | Effect |
|-------|----------|--------|
| 0 | NONE | No filtering |
| 1 | POINT | Sharp / nearest-neighbor (pixelated) |
| 2 | LINEAR | Smooth / bilinear (blurry) |
| 3 | ANISOTROPIC | High quality (GPU-dependent) |

## Installation

1. Rename the original `bass.dll` to `bass_real.dll`
2. Copy this mod's `bass.dll` into the game folder
3. On Android/Wine: set DLL override to `native` for `bass.dll`

## Technical Details

- **Hook target:** `Graphics_BeginFrame` (0x00453B50), 7-byte JMP detour
- **D3D8 device:** Graphics+0x154
- **SetTextureStageState:** vtable[63] (offset 0xFC), `__stdcall`
- **Android/Wine safe:** No IAT hooks, no threads, no GetTexture calls
- **Crash-tested:** 38.6s via hbtestd, no crash

### v2 → v3 Rollback Note

v2 attempted per-stage texture identification using `GetTexture` (vtable[61])
to apply different filters for checker/brick textures. However, the vtable
index was wrong — 61 is `SetTexture`, not `GetTexture` (which is index 60).
This caused `SetTexture` to be called with a stack pointer as the texture
parameter, corrupting D3D state and crashing the game at 1 second during
LoadingScreen Gadget's Draw. v3 reverts to the proven v1 approach: unconditional
`SetTextureStageState` on all 8 stages.
