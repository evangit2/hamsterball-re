# E:TELEPORT Mod

When the ball touches a floor panel tagged `E:TELEPORT(X)`, the race ends silently — no goal music, no popups, no results screen — and immediately loads and starts the specified level.

## Usage

1. Rename original `bass.dll` to `bass_real.dll`
2. Place this `bass.dll` next to `Hamsterball.exe`
3. In your custom MESHWORLD level, add a floor plane with the event name `E:TELEPORT(X)` where X is a number or level name

## Tag Format

The tag accepts either a **number** (1-15) or a **level name**:

### By number (recommended)

| Tag | Level Loaded | Race Name |
|-----|-------------|-----------|
| `E:TELEPORT(1)` | `levels\level1.MESHWORLD` | Warm-Up |
| `E:TELEPORT(2)` | `levels\level2.MESHWORLD` | Beginner |
| `E:TELEPORT(3)` | `levels\level3.MESHWORLD` | Intermediate |
| `E:TELEPORT(4)` | `levels\level4.MESHWORLD` | Dizzy |
| `E:TELEPORT(5)` | `levels\level5.MESHWORLD` | Tower |
| `E:TELEPORT(6)` | `levels\level6.MESHWORLD` | Up |
| `E:TELEPORT(7)` | `levels\level7.MESHWORLD` | Neon |
| `E:TELEPORT(8)` | `levels\level8.MESHWORLD` | Expert |
| `E:TELEPORT(9)` | `levels\level9.MESHWORLD` | Odd |
| `E:TELEPORT(10)` | `levels\level10.MESHWORLD` | Toob |
| `E:TELEPORT(11)` | `levels\level11.MESHWORLD` | Wobbly |
| `E:TELEPORT(12)` | `levels\level12.MESHWORLD` | Glass |
| `E:TELEPORT(13)` | `levels\level13.MESHWORLD` | Sky |
| `E:TELEPORT(14)` | `levels\level14.MESHWORLD` | Master |
| `E:TELEPORT(15)` | `levels\level15.MESHWORLD` | Impossible |

### By name (also works)

`E:TELEPORT(neon)`, `E:TELEPORT(dizzy)`, `E:TELEPORT(cascade)`, `E:TELEPORT(warmup)`, etc.

Also accepts `levelN` format: `E:TELEPORT(level3)` = same as `E:TELEPORT(3)`.

## How It Works

- Hooks `DispatchCollisionEvents` (0x40C5D0) via 8-byte trampoline
- When `E:TELEPORT(...)` collision is detected:
  1. Sets the win state flags (same as N:GOAL but without music/popups)
  2. Sets ball state to 5 (finished), player "reached goal" flags
  3. After 2 frames (deferred for safety), loads the target level:
     - Calls `App_StartRace(app)` to clean up current race
     - Creates a new `PlayerProfile` with race index set to `levelIndex - 1`
     - Calls `Tournament_AdvanceRace(profile, 0)` which creates the correct `LevelBoard_*_ctor`
       (AdvanceRace does `index + 1`, so we pre-decrement to land on the right case)

## Technical Details

- **Hook target**: `DispatchCollisionEvents` (0x40C5D0) — 8-byte trampoline (PUSH -1 + MOV EAX,FS:[0])
- **Win state**: Sets board+0xCD0 (goal reached), ball+0x30=5 (finished), App+playerIdx*0xA0+0x5D6=1 (reached goal), +0x5FC=1 (scored), +0x5F0=1 (newly reached)
- **Level load**: Creates PlayerProfile (0x98 bytes via HeapAlloc), sets profile+0x08 = levelIndex-1, calls Tournament_AdvanceRace(profile, 0)
- **No IAT hooks, no registry writes**

## Build

```bash
i686-w64-mingw32-g++ -shared -o bass.dll teleport_mod.cpp -static -lwinmm -Wl,--enable-stdcall-fixup
```

## Crash Test

Passed: 18.65s runtime, no crash (hbtestd Wine/Xvfb).
