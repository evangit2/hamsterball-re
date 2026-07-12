# E:WARP (Level Warp) Mod

When the ball touches a floor panel tagged `E:WARP(X)`, the race ends silently — no goal music, no popups, no results screen — and immediately loads and starts the specified level.

## Usage

1. Rename original `bass.dll` to `bass_real.dll`
2. Place this `bass.dll` next to `Hamsterball.exe`
3. In your custom MESHWORLD level, add a floor plane with the event name `E:WARP(X)` where X is a number or level name

## Tag Format

The tag accepts either a **number** (1-15) or a **level name**:

### By number (recommended)

| Tag | Level Loaded | Race Name |
|-----|-------------|-----------|
| `E:WARP(1)` | `levels\level1.MESHWORLD` | Warm-Up |
| `E:WARP(2)` | `levels\level2.MESHWORLD` | Beginner |
| `E:WARP(3)` | `levels\level3.MESHWORLD` | Intermediate |
| `E:WARP(4)` | `levels\level4.MESHWORLD` | Dizzy |
| `E:WARP(5)` | `levels\level5.MESHWORLD` | Tower |
| `E:WARP(6)` | `levels\level6.MESHWORLD` | Up |
| `E:WARP(7)` | `levels\level7.MESHWORLD` | Neon |
| `E:WARP(8)` | `levels\level8.MESHWORLD` | Expert |
| `E:WARP(9)` | `levels\level9.MESHWORLD` | Odd |
| `E:WARP(10)` | `levels\level10.MESHWORLD` | Toob |
| `E:WARP(11)` | `levels\level11.MESHWORLD` | Wobbly |
| `E:WARP(12)` | `levels\level12.MESHWORLD` | Glass |
| `E:WARP(13)` | `levels\level13.MESHWORLD` | Sky |
| `E:WARP(14)` | `levels\level14.MESHWORLD` | Master |
| `E:WARP(15)` | `levels\level15.MESHWORLD` | Impossible |

### By name (also works)

`E:WARP(neon)`, `E:WARP(dizzy)`, `E:WARP(cascade)`, `E:WARP(warmup)`, etc.

Also accepts `levelN` format: `E:WARP(level3)` = same as `E:WARP(3)`.

## How It Works

- Hooks `DispatchCollisionEvents` (0x40C5D0) via 8-byte trampoline to detect `E:WARP(...)` collisions
- Also hooks `App_FrameUpdate` epilogue (0x46C1F1) via 5-byte trampoline for main-thread deferred level loading
- When `E:WARP(...)` collision is detected:
  1. Sets the win state flags (same as N:GOAL but without music/popups)
  2. Sets a warp flag + 1-frame delay
  3. On the **next App_FrameUpdate epilogue** (main thread, after GameUpdate returns), loads the target level:
     - Calls `App_StartPracticeRace(app, levelIndex)` which handles everything internally:
       - `App_StartRace(app)` — cleans up previous race state
       - Manages BestTimeTracker recording/playback buffers
       - Creates `PlayerProfile` with race index set to `levelIndex`
       - Calls `Tournament_AdvanceRace(profile, 0)` which creates the correct `LevelBoard_*_ctor`

## Technical Details

- **Collision hook**: `DispatchCollisionEvents` (0x40C5D0) — 8-byte trampoline (PUSH -1 + MOV EAX,FS:[0])
- **Frame update hook**: `App_FrameUpdate` epilogue (0x46C1F1) — 5-byte trampoline (POP ESI + ADD ESP,8 + RET)
- **Win state**: Sets board+0xCD0 (goal reached), App+playerIdx*0xA0+0x5D6=1 (reached goal), +0x5FC=1 (scored), +0x5F0=1 (newly reached)
- **Level load**: Calls `App_StartPracticeRace(app, levelIndex)` — the same function the game calls when a player selects a practice race from the menu
- **No IAT hooks, no registry writes, no background threads**

## v3 Fix — Epilogue Hook (July 2026)

**Bug (v2)**: Hooked the *entry* of `App_FrameUpdate` (0x46C170). `loadTargetLevel()`
ran *before* `GameUpdate` was called in the same frame, modifying the scene graph
(destroying old board, creating new board) while the frame was still in progress.
`GameUpdate` then ran on the inconsistent scene state → crash at 0x41820B.

**Fix**: Hook the *epilogue* of `App_FrameUpdate` (0x46C1F1) — the 5-byte
`POP ESI / ADD ESP,8 / RET` sequence. `loadTargetLevel()` now runs *after*
`GameUpdate` returns, at the very end of the frame. The scene is in a stable
state and the new board will be ready for the *next* frame's `GameUpdate`.

## v2 Fix — Thread Safety (July 2026)

**Bug (v1)**: Used a background polling thread (Sleep(16) loop) to call
`loadTargetLevel()`. The background thread called `App_StartRace()` which
destroys the current board/scene while the main thread's Draw code was still
iterating over those same scene objects → use-after-free crash inside
`CreateMechanicalObjects` (crash address `0001:0001820B` = 0x41820B).

**Fix**: Replaced the background polling thread with a hook on `App_FrameUpdate`
epilogue which runs every frame on the main thread. Eliminated the race condition.

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll warp_mod.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc -Wl,--add-stdcall-alias
```

## Crash Test

Passed: 13.67s runtime, no crash (hbtestd Wine/Xvfb).
