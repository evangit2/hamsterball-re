# Global BUMPER Mod (global_objects_mkn)

Makes **BUMPER objects work on ANY race/level**, not just Beginner Race, Master Race, Warm-Up, and Arena-Toob.

## How It Works

BUMPER objects are mesh-level collision objects named `N:BUMPER%d` (1-8). They exist as visual meshes in many MESHWORLD files, but only have **physics effects** (velocity boost + sound + hit animation) on levels whose board vtable dispatches to a collision handler that recognizes `N:BUMPER`.

This mod hooks `DispatchCollisionEvents` (0x40C5D0) — the **base** collision dispatcher called by ALL board vtables. When an `N:BUMPER` collision is detected on a board that doesn't natively handle bumpers, the mod applies the same physics as the game's own bumper handlers:

1. **Sound**: Plays bumper SFX (`Sound_Play3D` at App+0x448, ball position)
2. **Velocity boost**: Scales XZ velocity by 4.0× (constant `_DAT_004cf41c`)
3. **Flat bounce**: Sets Y velocity to 0
4. **Speed clamp**: Min 5.0, Max 10.0 (race) / 12.0 (arena)
5. **Hit flag**: Sets bumper animation flag (`1.0f` at board-specific offset)

## Board Detection

The mod detects board type by checking `vtable[0x1D]` (offset 0x74):

| Board Type | Collision Handler | Hit Flag Offset |
|---|---|---|
| Race (Warm-Up, Beginner, Master, etc.) | 0x40C5D0 (base) | board + N×4 + 0x6448 |
| Arena (Beginner, Toob) | 0x412850 | board + (N-1)×4 + 0x53FC |

If the board already handles bumpers natively (Beginner, Master, Warm-Up, Arena), the mod skips processing to avoid double-effects.

## Installation

1. Rename original `bass.dll` to `bass_real.dll`
2. Place this `bass.dll` in the game folder

## Build

```
i686-w64-mingw32-gcc -shared -o bass.dll global_objects_mkn.c -lwinmm \
    -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
```

## Technical Details

### BUMPER Physics Constants (from .rdata)

| Address | Value | Purpose |
|---|---|---|
| 0x4CF41C | 4.0f | Velocity scale multiplier |
| 0x4CF55C | 5.0f | Minimum launch speed |
| 0x4CF9F8 | 10.0f | Max speed (race boards) |
| 0x4CF3DC | 12.0f | Max speed (arena boards) |

### Ball Physics Offsets

| Offset | Field |
|---|---|
| ball+0x164/168/16C | Position X/Y/Z (float) |
| ball+0x1A4 | Physics* pointer |
| phys+0xCA4/CA8/CAC | Velocity X/Y/Z (float) |

### Board Bumper Slots (per level setup function)

| Level | Setup Function | Mesh Path | Slot Offset | Stride | Max Bumpers |
|---|---|---|---|---|---|
| Warm-Up | 0x4110D0 | `levels\levelcascade` | +0x10DB | 0x106 | 8 |
| Beginner (L8) | 0x40FA20 | `levels\level8` | +0x10E3 | 0x106 | 8 |
| Master (L10) | 0x412010 | `levels\level10` | +0x10E7 | 0x106 | 4 |
| Arena-Beginner | 0x413D70 | `levels\arena-beginner` | +0x11F8 | 0x106 | 4 |
| Arena-Toob | 0x414F50 | `levels\arena-Toob` | +0x11F8 | 0x106 | 5 |
