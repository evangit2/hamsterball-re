# Background Color Cycler v2

Cycles the **actual background (fog) color** of all races and arenas through 15 colors on a ~3-second timer.

## What It Does

Intercepts the per-frame background color read in `Graphics_RenderScene` and replaces the fog color with a cycling palette of 15 colors.

## How Background Colors Work in Hamsterball

Deep trace through the binary:

1. **`Level_InitScene` (0x40B090)** — Called on level/arena load. Calls `Color_RandomRGBA()` to pick a **random** fog/background color, then calls `Gfx_SetVTable22` (0x453B20).
2. **`Gfx_SetVTable22` (0x453B20)** — Sets `D3DRS_FOGCOLOR` (render state 0x22) via `SetRenderState(0x22, color)` and caches it at `gfx+0x738`.
3. **`Graphics_RenderScene` (0x454BC0)** — Every frame, reads `gfx+0x738` at address `0x454CEE` (`MOV ESI, [EBP+0x738]`), then calls `SetRenderState(0x22, ESI)` at `0x454D00`.

The **fog color fills the entire background** (sky, distant areas, everything beyond the 3D geometry). This is NOT ambient lighting — it's the actual background color you see behind all level geometry.

## Hook Point

| Address | Original Bytes | Instruction | Hook |
|---------|---------------|-------------|------|
| `0x454CEE` | `8B B5 38 07 00 00` | `MOV ESI, [EBP+0x738]` | JMP to code cave + NOP |

- **EBP** = gfx pointer (preserved)
- **gfx+0x738** = cached D3DRS_FOGCOLOR
- After the hook, the game's own `SetRenderState(0x22, ESI)` uses our cycled color

## Color Palette (15 colors)

| Index | Color | Hex |
|-------|-------|-----|
| 0 | Black | `FF000000` |
| 1 | Blue | `FF0000FF` |
| 2 | Green | `FF00FF00` |
| 3 | Yellow | `FFFFFF00` |
| 4 | White | `FFFFFFFF` |
| 5 | Red | `FFFF0000` |
| 6 | Cyan | `FF00FFFF` |
| 7 | Magenta | `FFFF00FF` |
| 8 | Gray | `FF808080` |
| 9 | Orange | `FFFF8000` |
| 10 | Purple | `FF8000FF` |
| 11 | Light Blue | `FF0080FF` |
| 12 | Lime | `FF80FF00` |
| 13 | Pink | `FFFF0080` |
| 14 | Teal | `FF008080` |

## Timer

- Default: 180 frames (~3 seconds at 60 FPS)
- Edit `bgcolor_timer` in the CE address list to change cycle speed

## Register Safety

- **EBP** (gfx pointer): preserved, not touched
- **ESI**: set to cycled color (game expects this for `SetRenderState`)
- **EAX, ECX, EDX**: clobbered (all reloaded by game after hook point)
- **EBX, EDI**: not touched

## v1 vs v2

| | v1 (old) | v2 (this) |
|---|---------|-----------|
| Hook point | `0x405E22` (Ball_Update) | `0x454CEE` (Graphics_RenderScene) |
| What it changed | `gfx+0x730` (ambient lighting) | `gfx+0x738` (fog/background color) |
| Effect | Changed object brightness | Changes actual background color |
| Colors | 15 | 15 |

## Compatibility

- Works on ALL races and arenas (hook is in the shared render pipeline)
- Mutually exclusive with GlobalNeonEffect (both hook the same render function, though at different offsets)
