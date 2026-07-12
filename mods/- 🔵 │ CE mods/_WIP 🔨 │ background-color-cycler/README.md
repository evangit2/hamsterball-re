# Background Color Cycler v3

Cycles the **actual background (Clear) color** of all races and arenas through 15 colors on a ~3-second timer.

## v2→v3 Root Cause Fix

**v2 problem:** Hooked at `0x454CEE` inside `Graphics_RenderScene` — AFTER `Clear()` already painted the background. It only updated `D3DRS_FOGCOLOR` (fog blending on distant geometry), NOT the actual background fill.

**v3 fix:** Hooks at `0x46C214` inside `App_ResetFrame` — BEFORE `Clear()` runs. The cycled color is written to `gfx+0x738`, then the game pushes `EAX` (our color) and calls `Graphics_ClearViewport` → `Clear()` paints the background with our cycled color.

## Frame Pipeline (traced from disassembly)

1. **`App_ResetFrame` (0x46C200):**
   - `0x46C214`: `MOV EAX, [ECX+0x738]` ← **v3 HOOK** (inject cycled color)
   - `0x46C21A`: `PUSH EAX` ← pushes our color
   - `0x46C21B`: `CALL Graphics_ClearViewport` ← **Clear() paints background with our color**

2. **`Graphics_RenderScene` (0x454BC0):**
   - `0x454CEE`: `MOV ESI, [EBP+0x738]` ← reads our already-updated color
   - `0x454D00`: `SetRenderState(0x22, ESI)` ← fog color matches background

## Hook Point

| Address | Original Bytes | Instruction | Hook |
|---------|---------------|-------------|------|
| `0x46C214` | `8B 81 38 07 00 00` | `MOV EAX, [ECX+0x738]` | JMP to code cave + NOP |

- **ECX** = gfx pointer (MUST preserve — `ClearViewport` reads it as `this`)
- **EAX** = output (game pushes `EAX` as Clear color at `0x46C21A`)
- **EDX** = clobbered (safe — reloaded inside `ClearViewport`)

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

## Version History

| | v1 | v2 | v3 |
|---|---|---|---|
| Hook point | `0x405E22` (Ball_Update) | `0x454CEE` (Graphics_RenderScene) | `0x46C214` (App_ResetFrame) |
| What it changed | `gfx+0x730` (ambient) | `gfx+0x738` (fog state only) | `gfx+0x738` (Clear color) |
| Effect | Object brightness | Fog blending only | **Actual background** |
| Colors | 15 | 15 | 15 |

## Compatibility

- Works on ALL races and arenas (hook is in `App_ResetFrame`, shared across all scenes)
- Compatible with GlobalNeonEffect (different hook points: v3 at `0x46C214`, neon at `0x405E22`/`0x403DC0`/`0x4050D9`)
