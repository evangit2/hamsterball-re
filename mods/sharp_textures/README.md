# Sharp Textures Mod

Force custom texture filtering in Hamsterball, with a separate override for
Checker and Brick textures.

## What It Does

D3D8's default texture filtering is LINEAR (smooth/blurry). This mod hooks
`Graphics_BeginFrame` (called every frame) and overrides the filtering on all
8 texture stages using `SetTextureStageState`.

**Two filter profiles:**

1. **Default** — Applied to all textures.
2. **Checker/Brick override** — Applied only to textures whose filename
   contains "checker" or "brick" (e.g. `purplechecker.bmp`, `bluebrick.png`).
   This override takes priority over the default.

The mod scans the game's texture cache each frame (rescanning when the texture
count changes, e.g. on level load) to identify checker/brick textures by
filename. It then uses `GetTexture` (vtable[60]) per stage to check which
texture is currently bound and applies the appropriate filter.

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

### Default Config

The default config makes everything smooth (LINEAR=2) except checker/brick
textures, which are sharp (POINT=1). Adjust to taste.

## Installation

1. Rename the original `bass.dll` to `bass_real.dll`
2. Copy this mod's `bass.dll` into the game folder
3. On Android/Wine: set DLL override to `native` for `bass.dll`

## Technical Details

- **Hook target:** `Graphics_BeginFrame` (0x00453B50), 7-byte JMP detour
- **D3D8 device:** Graphics+0x154
- **Texture cache:** Graphics+0x2E8 (count), Graphics+0x6F0 (array)
- **Texture object:** 0x74 bytes — D3D texture at +0x04, filename at +0x08
- **GetTexture:** vtable[60] (0xF0), **SetTextureStageState:** vtable[63] (0xFC)
- **Calling convention:** D3D8 COM methods use `__stdcall` (this on stack)
- **Android/Wine safe:** No IAT hooks, no threads
- **Crash-tested:** 38.6s hbtestd + 45s manual game run with Warm-Up navigation
