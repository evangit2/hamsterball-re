# Sharp Textures Mod

Force custom texture filtering in Hamsterball, with a separate override for
Checker and Brick textures.

## What It Does

D3D8's default texture filtering is LINEAR (smooth/blurry). This mod hooks the
D3D8 device's vtable to intercept **every** `SetTextureStageState` call the
game makes, overriding the filter values in real-time.

**Two filter profiles:**

1. **Default** — Applied to all textures.
2. **Checker/Brick override** — Applied only to textures whose filename
   contains "checker" or "brick" (e.g. `BlueChecker.bmp`, `redbrick.png`).
   This override takes priority over the default.

The mod also hooks `SetTexture` (vtable[61]) to track which texture is bound
to each stage. This allows per-stage filter selection: if the bound texture is
a tracked checker/brick texture, the checker filter profile is used; otherwise
the default profile applies.

## Why v5 (vtable hook)?

v4 set filters once per frame at `Graphics_BeginFrame`. However, the game's own
render code calls `SetTextureStageState` **after** `BeginFrame`, overriding the
mod's settings. This caused some textures (flags, goal pads, small objects)
to remain blurry even though they used the same checker texture file.

v5 patches the D3D8 device vtable directly, intercepting **every** call to
`SetTextureStageState`. This ensures the mod's filter values persist regardless
of what the game sets during rendering.

## Config File

`sharp_textures.txt` (next to `bass.dll`), auto-generated on first run:

```ini
# Sharp Textures config
# Values: 0=none, 1=point(sharp), 2=linear(smooth), 3=anisotropic

# Default filter for ALL textures:
MAGFILTER = 2
MINFILTER = 2
MIPFILTER = 2

# Override for textures containing 'checker' or 'brick' in filename:
CHECKER_MAGFILTER = 1
CHECKER_MINFILTER = 1
CHECKER_MIPFILTER = 1
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

- **Hook target:** D3D8 device vtable — `SetTextureStageState` (vtable[63]) and
  `SetTexture` (vtable[61])
- **Vtable hook installed on:** First `Graphics_BeginFrame` call (when D3D8
  device is available)
- **Texture cache scan:** Graphics+0x2E8 (count), Graphics+0x6F0 (array)
- **Texture object:** D3D texture at +0x04, filename at +0x08
- **Calling convention:** D3D8 COM methods use `__stdcall`
- **Android/Wine safe:** No IAT hooks, no threads
- **Crash-tested:** 38.6s hbtestd
