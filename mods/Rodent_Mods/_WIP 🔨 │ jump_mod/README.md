# jump_mod v22

Press SPACE to jump (Player 1 only, raycast ground detection).

## What changed from v20

**v22 adds countdown and race-end gating:**

1. **Countdown gate:** Before allowing a jump, checks `Scene+0x3A4C`
   (countdown_done flag). This flag is set to 1 by `Scene_HandleRaceEnd`
   (0x41B130) when all 3 Ready/Set/Go phases complete. If 0, the game
   itself blocks all input in `Scene_vmethod31` (0x41AC70) — the jump
   mod now mirrors this behavior.

2. **Race-end gate:** Checks `ball+0x14C` (freeze flag). Set to 1 by
   `Scene_HandleRaceEnd` at 0x41B40D when the race timer expires
   (player touches the goal). Also checked by the game's `Ball_Update`
   at 0x4060A1. When set, the ball is frozen and jumping is blocked.

Both gates are checked in the input thread BEFORE running the raycast,
so denied jumps don't waste a raycast call.

## What changed from v17

**v20 changed ground detection from a fixed epsilon to a slope-aware
threshold (radius × 1.45):**

On a slope of angle θ, a straight-down raycast hits at distance r/cos(θ)
from the ball center, not r. Using `radius * 1.45` covers slopes up to
45° (cos(45°) ≈ 0.707, r/0.707 ≈ 1.414r). The 1.45 factor gives a small
safety margin beyond the theoretical minimum of √2 ≈ 1.414.

## Features

- **Raycast ground detection:** Casts a ray straight down from the ball
  position using the game's own `Mesh_FindClosestCollision` (0x465D90).
  If the hit point is within `radius × 1.45` of the ball Y, the ball is
  grounded and can jump.
- **Countdown gating:** No jumping during Ready/Set/Go countdown
  (`Scene+0x3A4C == 0`).
- **Race-end gating:** No jumping after touching the goal
  (`ball+0x14C == 1`).
- **Airborne denial:** Can't jump while in the air (raycast misses).
- **Edge detection:** SPACE uses rising-edge detection (one jump per keypress).
- **Safety checks:** Won't jump if ball pointer not yet captured or
  during fall/respawn.

## Hook points

| Hook | Address | Original bytes | Purpose |
|------|---------|---------------|---------|
| Phase 15 cave | `0x407BB4` | `8B 4C 24 1C 8B 11` (6 bytes) | Jump impulse application |

The input thread polls the keyboard (DIK_SPACE at KeyboardDevice+0x45)
every 16ms. On rising-edge keypress, it checks countdown/race-end gates,
then runs the raycast. If grounded, sets `g_want_jump=1`. The Phase 15
cave checks this flag and adds an upward impulse to `ball+0x174` (Y force
accumulator) if set.

## Files

- `jump_mod_raycast.c` — C source code (BASS proxy + raycast + gates)
- `bass.dll` — Compiled DLL (MinGW cross-compiled, PE32 i386)
- `jump_mod_v22.zip` — Distribution archive

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll jump_mod_raycast.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```
