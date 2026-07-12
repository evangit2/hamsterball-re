# 8ball_goal_fix

Prevents crash when 8-ball (BadBall) touches an N:GOAL trigger.

## Root Cause

BadBalls (8-balls) have `player_index = -1` (set in `Ball_ctor2` at 0x4039E0, never overwritten by `CreateBadBall`). When a BadBall hits an N:GOAL trigger, `DispatchCollisionEvents` (0x40C5D0) uses `player_index` as an array index:

```
addr = player_index * 0xA0 + App
```

With `player_index = -1`, this computes `addr = App - 0xA0`, then writes to three offsets: `App+0x536`, `App+0x550`, `App+0x55C` — corrupting App state and crashing the game.

Raptisoft already knew about this problem: the `E:LIMIT` handler in the same function (at 0x0040C785) has a `CMP ECX,-1 / JZ skip` guard. They just forgot to add the same guard to the N:GOAL and E:ACTION(SCORE) handlers.

## Fix

Three code caves that replicate the E:LIMIT pattern: check `player_index < 0` before per-player writes, and if so, skip to the safe code (camera repositioning, status strings — which don't use player_index).

| Patch | Address | What it guards | Skip target |
|-------|---------|----------------|-------------|
| 1 | 0x0040CF64 | N:GOAL "finished" flag (App+0x5D6, App+0x5FC) | 0x0040CFA0 (camera) |
| 2 | 0x0040D03A | N:GOAL "show results" flag (App+0x5F0) | 0x0040D05E (epilogue) |
| 3 | 0x0040CA33 | E:ACTION(SCORE) score write (App+0x5E4) | 0x0040CA74 (loop continue) |

## Result

- 8-ball crossing goal: goal music plays, camera repositions, board flag set ✅
- Player crossing goal after: race finishes normally with results screen ✅
- No crash ✅
- Identical visible behavior to original game minus the crash ✅

## Installation

1. Rename original `bass.dll` to `bass_real.dll`
2. Copy this mod's `bass.dll` into the game folder
3. Launch Hamsterball.exe

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll 8ball_goal_fix.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc -Wl,--add-stdcall-alias
```

## Crash Test

Passed via hbtestd (35s survival, process alive). Tested on Wine/Xvfb/llvmpipe.
