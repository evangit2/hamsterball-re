# Background Color Cycler

Cycles the ambient scene color through 6 colors every ~3 seconds.

## Files

- `BackgroundColorCycler.CEA` — Cheat Engine AutoAssembler script

## How It Works

Hooks `Graphics_RenderScene` at `0x454C97` where the game reads `gfx+0x730` (the D3DRS_AMBIENT color value) before passing it to `SetRenderState(D3DRS_AMBIENT, color)`.

Each frame, the script:
1. Increments a frame counter
2. When the counter reaches `bgcolor_timer` (default 180 = ~3 seconds at 60fps), advances to the next color in the palette
3. Overwrites `gfx+0x730` with the current cycling color
4. Executes the original `MOV ESI, [EBP+0x730]` instruction and returns

## Color Cycle

| Index | Color   | D3DCOLOR    |
|-------|---------|-------------|
| 0     | Blue    | 0xFF0000FF  |
| 1     | Green   | 0xFF00FF00  |
| 2     | Yellow  | 0xFFFFFF00  |
| 3     | White   | 0xFFFFFFFF  |
| 4     | Black   | 0xFF000000  |
| 5     | Red     | 0xFFFF0000  |

## Configuration

In Cheat Engine, after enabling the script, you can:
- **Change cycle speed**: Edit `bgcolor_timer` in the address list (default 180). Lower = faster, Higher = slower.
- **Start at a specific color**: Set `bgcolor_index` (0-5) and reset `bgcolor_frame` to 0.
- **Add more colors**: Add `dd XXXXXXXX` entries before the terminator `dd 00000000` in the `bgcolor_colors` block.

## Compatibility

- Works in **all races and all arenas** — hooks the shared `Graphics_RenderScene` function.
- **Mutually exclusive** with `GlobalNeonEffect.CEA` — both hook `0x454C97`. Enable one OR the other, never both.
- Register-safe: only uses EAX and EDX (both dead at the hook point). EBP preserved.

## Verification

- Hook address `0x454C97` verified via Ghidra disassembly of `Graphics_RenderScene` (0x454BC0).
- `gfx+0x730` confirmed as D3DRS_AMBIENT via decompilation: game calls `SetRenderState(0x8B, [gfx+0x730])` at 0x454CAC.
- D3DRS_AMBIENT = state 139 = 0x8B (verified D3D8 render state constant).
- No runtime test yet — requires CE on Windows.
