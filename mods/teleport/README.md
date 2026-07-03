# E:TELEPORT Mod

When the ball touches a floor panel tagged `E:TELEPORT(LevelName)`, the race ends silently — no goal music, no popups, no results screen — and immediately loads and starts the specified level.

## Usage

1. Rename original `bass.dll` to `bass_real.dll`
2. Place this `bass.dll` next to `Hamsterball.exe`
3. In your custom MESHWORLD level, add a floor plane with the event name `E:TELEPORT(LevelName)`

### Level Name Format

The level name in parentheses maps to the game's level files:

| Tag | Level Loaded | File |
|-----|-------------|------|
| `E:TELEPORT(Level1)` | Warm-Up Race | `levels\level1.MESHWORLD` |
| `E:TELEPORT(Level2)` | Beginner Race | `levels\level2.MESHWORLD` |
| `E:TELEPORT(Level3)` | Intermediate Race | `levels\level3.MESHWORLD` |
| `E:TELEPORT(Level4)` | Dizzy Race | `levels\level4.MESHWORLD` |
| `E:TELEPORT(Level5)` | Tower Race | `levels\level5.MESHWORLD` |
| `E:TELEPORT(Level6)` | Up Race | `levels\level6.MESHWORLD` |
| `E:TELEPORT(Level7)` | Neon Race | `levels\level7.MESHWORLD` |
| `E:TELEPORT(Level8)` | Expert Race | `levels\level8.MESHWORLD` |
| `E:TELEPORT(Level9)` | Odd Race | `levels\level9.MESHWORLD` |
| `E:TELEPORT(Level10)` | Toob Race | `levels\level10.MESHWORLD` |
| `E:TELEPORT(Level11)` | Wobbly Race | `levels\level11.MESHWORLD` |
| `E:TELEPORT(Level12)` | Glass Race | `levels\level12.MESHWORLD` |
| `E:TELEPORT(Level13)` | Sky Race | `levels\level13.MESHWORLD` |
| `E:TELEPORT(Level14)` | Master Race | `levels\level14.MESHWORLD` |
| `E:TELEPORT(Level15)` | Impossible Race | `levels\level15.MESHWORLD` |

Also accepts descriptive names: `E:TELEPORT(LevelNeon)`, `E:TELEPORT(LevelDizzy)`, `E:TELEPORT(LevelCascade)`, etc.

## How It Works

- Hooks `DispatchCollisionEvents` (0x40C5D0) via 8-byte trampoline
- When `E:TELEPORT(...)` collision is detected:
  1. Sets the win state flags (same as N:GOAL but without music/popups)
  2. Sets ball state to 5 (finished), player "reached goal" flags
  3. After 2 frames (deferred for safety), calls `App_StartRace` + `App_StartPracticeRace` to load the target level
- A polling thread (60Hz, 16ms sleep) handles the deferred level load

## Technical Details

- **Hook target**: `DispatchCollisionEvents` (0x40C5D0) — 8-byte trampoline (PUSH -1 + MOV EAX,FS:[0])
- **Win state replication**: Sets board+0xCD0 (goal reached), ball+0x30=5 (finished), App+playerIdx*0xA0+0x5D6=1 (reached goal), +0x5FC=1 (scored), +0x5F0=1 (newly reached)
- **Level load**: Calls `App_StartPracticeRace(app, raceIndex)` which creates PlayerProfile + `Tournament_AdvanceRace` → `LevelBoard_*_ctor`
- **No IAT hooks, no registry writes**

## Build

```bash
i686-w64-mingw32-g++ -shared -o bass.dll teleport_mod.cpp -static -lwinmm -Wl,--enable-stdcall-fixup
```

## Crash Test

Passed: 18.72s runtime, no crash (hbtestd Wine/Xvfb).
