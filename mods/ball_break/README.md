# Ball Break Mod

Press **X** to shatter your ball and respawn at the nearest checkpoint.

## How It Works

1. A background thread polls DIK_X (0x2D) every 16ms
2. On rising-edge keypress, checks gates:
   - Countdown finished (scene+0x3A4C == 1)
   - Race not ended (App+0x5D6 == 0)
   - Player flag2 clear (App+0x5D5 == 0)
   - Ball not already shattered (ball+0x2E8 == 0)
3. Calls `Ball_Shatter` (0x408D70) — the game's own "ball breaks into 3 pieces" function
4. Sets `ball+0x2E8=1` (shattered flag)
5. Next frame, `Scene_UpdateBallsAndState` (0x41B540) detects the shattered flag and calls `Ball_FindClosestRespawnPoint` (0x405190) → teleports ball to nearest respawn point, clears velocity, starts fall animation

## Reverse Engineering Details

| Function | Address | Convention | Purpose |
|---|---|---|---|
| `Ball_Shatter` | 0x408D70 | `__thiscall(ball, Vec3List*)` | Breaks ball into 3 debris pieces, plays sound |
| `Ball_StartFall` | 0x402200 | `__fastcall(ball)` | Sets ball+0xC4C=1, radius to 13.0 (falling) |
| `Ball_EndFall` | 0x402270 | `__fastcall(ball)` | Sets ball+0xC4C=0, radius to 26.0 (normal) |
| `Ball_FindClosestRespawnPoint` | 0x405190 | `__fastcall(ball)` | Finds nearest respawn, teleports, clears velocity |
| `Scene_UpdateBallsAndState` | 0x41B540 | `__fastcall(scene)` | Per-frame: checks ball+0x2E8, triggers respawn |

### Ball Struct Offsets
| Offset | Type | Field |
|---|---|---|
| 0x014 | DWORD | Board pointer |
| 0x018 | int | Player index (0=P1, -1=AI) |
| 0x164 | float | Position X |
| 0x168 | float | Position Y |
| 0x16C | float | Position Z |
| 0x2E8 | byte | Is shattered (set by Shatter) |
| 0x2F9 | byte | Is falling (set by FindClosestRespawnPoint) |
| 0x300 | DWORD | Respawn timer (set to 150 = 0x96) |
| 0x324 | byte | Is invincible (if 1, destroyed instead of respawned) |

## Installation

1. Back up your original `bass.dll` (rename to `bass_real.dll`)
2. Copy this mod's `bass.dll` into the Hamsterball game folder
3. Launch the game
4. Press **X** during a race to break and respawn

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll ball_break.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```

Crash-tested: process survives 40s on Wine/Xvfb (DLL loads cleanly, no stack corruption).
