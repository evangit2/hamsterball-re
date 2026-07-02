# Global BG Texture (Clouds) Mod

Adds a clouds.png background sprite overlay to any level via a `Sprite_RenderQuad` hook inside `Graphics_RenderScene`.

## RE Analysis

Sky Race (level 12) ctor at `0x41F930` creates a `clouds.png` Sprite at `board+0x4374`, but this sprite is **never rendered** by any function — it's dead code. The actual clouds visual on Sky Race comes from:

1. **CLOUDSCAPE mesh** geometry in `Level9.MESHWORLD` (3D textured mesh)
2. **Fog/projection** setup in `Scene_SetupLevel9` (`0x410830`)

This mod provides a 2D clouds overlay for non-Sky levels by creating a sprite and calling `Sprite_RenderQuad` from a render hook at `0x454CEE` (inside `Graphics_RenderScene`).

For Sky Race, zeroing `board+0x4374` is cleanup-safe (dtor checks for NULL before freeing) but won't affect the 3D CLOUDSCAPE mesh.

## Config

Auto-generates `clouds_bg.txt` on first launch:

```ini
# Clouds Background Texture Mod
# Set to 1 to enable clouds, 0 to disable
# Default: clouds only on Sky Race

Warm-Up = 0
Beginner = 0
Intermediate = 0
Dizzy = 0
Tower = 0
Up = 0
Neon = 0
Expert = 0
Odd = 0
Toob = 0
Wobbly = 0
Glass = 0
Sky = 1
Master = 0
Impossible = 0
```

Edit values at runtime — the mod re-reads the config every 500ms.

## Installation

1. Rename original `bass.dll` to `bass_real.dll`
2. Copy this mod's `bass.dll` to the game folder

## Technical Details

- **Hook point**: `0x454CEE` — `MOV ESI, [EBP+0x738]` (6 bytes, inside `Graphics_RenderScene`)
- **Code cave**: VirtualAlloc'd, executes original instruction → calls C render function → jumps back
- **Sprite creation**: `Sprite_ctor` at `0x45D0C0` with `"textures\clouds.png"`
- **Sprite rendering**: `Sprite_RenderQuad` at `0x45D660` — renders a textured quad at ball position
- **Level detection**: `App+0x220 → +0x08` (race index 0-14)
- **Config is NOT overwritten** on subsequent launches (uses `CREATE_NEW`, not `CREATE_ALWAYS`)
- **Crash-tested**: 13.66s via hbtestd, no crash
