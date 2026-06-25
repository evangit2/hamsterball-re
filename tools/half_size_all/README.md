# half_size_all

Shrinks the player's ball to half size by inlining Ball_Shrink's physics fields

## How It Works

Hooks `Scene_SpawnBallsAndObjects` at the point where the player ball has just been created and registered. A code cave checks if the ball's player index (`ball+0x18`) is 0, and if so, writes the same three fields that `Ball_Shrink` (0x00402200) sets — but **without calling the function**, so no sound effect plays.

**Fields written (identical to Ball_Shrink):**

| Ball Offset | Field | Value | Effect |
|-------------|-------|-------|--------|
| `+0x284` | radius | 13.0 (0x41500000) | Half visual + collision size |
| `+0x188` | physics_scale | 2.5 (0x40200000) | Half max speed |
| `+0xC4C` | in_shrunk | 1 | Shrunk physics state |

**Only player index 0 is affected.** AI balls, split balls, follow balls, and board-init balls remain normal size.

## Hook Details

| Hook Point | Address | Original Instruction | Catches |
|------------|---------|---------------------|---------|
| Scene_SpawnBallsAndObjects | 0x0041C8D7 | `MOV byte [ESI+0x281], 0` (7 bytes) | Player balls (loop, gated on index 0) |

The code cave:
1. `CMP dword [ESI+0x18], 0` — is this player 0?
2. If no → skip to step 4
3. If yes → write radius=13.0, physics_scale=2.5, is_falling=1
4. Execute original `MOV byte [ESI+0x281], 0`
5. `JMP` back to 0x0041C8DE

## Files
- `half_size_balls.c` — C source code
- `bass.dll` — Compiled DLL (PE32 i386)
- `half_size_balls.zip` — Packaged zip

## Proxy Type
BASS.dll proxy. Installation:
1. Rename original `bass.dll` → `bass_real.dll` in the Hamsterball game folder
2. Copy the mod's `bass.dll` into the game folder
3. Launch Hamsterball

A `Hamsterball_half_size.log` file is written next to the EXE showing whether the hook applied.
