# half_size_all

Halves ALL balls' size using the game's own `Ball_Shrink` function

## How It Works

Instead of patching float immediates, this mod hooks two ball creation points and calls the game's own `Ball_Shrink` function (0x00402200) — the same function Odd Race uses to shrink the ball in its pipe maze.

**`Ball_Shrink`** sets:
- `ball+0x284` = 13.0 (radius, down from 26.0)
- `ball+0x188` = 2.5 (physics_scale, down from 5.0)
- `ball+0xC4C` = 1 (is_falling flag)

### Hooks

| Hook | Address | Target | Catches |
|------|---------|--------|---------|
| Ball_ctor2 exit | 0x00403DB1 | `Ball_Shrink` via code cave | Player, split, follow, board-init balls |
| CreateBadBall FSTP | 0x0040BE74 | `Ball_Shrink` via code cave | AI/bad balls (after SIZE computed) |

Each hook saves registers, sets ECX = ball pointer (`__fastcall`), calls `Ball_Shrink`, restores registers, then executes the original instruction.

## Files
- `half_size_balls.c` — C source code
- `bass.dll` — Compiled DLL (PE32 i386)
- `half_size_balls.zip` — Packaged zip

## Proxy Type
BASS.dll proxy. Installation:
1. Rename original `bass.dll` → `bass_real.dll` in the Hamsterball game folder
2. Copy the mod's `bass.dll` into the game folder
3. Launch Hamsterball

A `Hamsterball_half_size.log` file is written next to the EXE showing which hooks applied.
