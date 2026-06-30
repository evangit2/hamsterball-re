# Wall Bumpers Mod

All walls act as pinball bumpers — the ball bounces off walls with amplified force.

## Installation

1. Rename original `bass.dll` → `bass_real.dll` in your Hamsterball folder
2. Copy the modded `bass.dll` into the game folder
3. Launch the game

## Controls

| Key | Action |
|-----|--------|
| F8  | Toggle mod on/off (default: ON) |
| F9  | Cycle bumper force (40 → 60 → 80 → 120 → 200) |

## Log File

The mod writes `bumper_mod.log` in the game directory. Check this file to confirm:
- Hook installed successfully
- Per-frame ball tracking data
- Bumper hit events with velocity details

## How It Works (v3)

The mod hooks `Ball_AI_ChaseNearest` (0x408390) — ball vtable[4], called every frame for all balls including the player in race mode.

### Detection: Two Methods

1. **Velocity Reversal Detection** (primary, works in all modes):
   Tracks ball position each frame and computes velocity from position delta. If the dot product of consecutive velocity vectors is negative (velocity reversed), the ball hit something — amplify the bounce.

2. **Collision Entry Scanning** (secondary, arena mode only):
   Scans the PhysicsObject's collision entry list for type==2 (wall) entries and reads surface normals.

### History

- **v1**: Hooked Ball_FallUpdate (0x408830) — dead code, never called for player
- **v2**: Hooked Ball_AI_ChaseNearest (0x408390) but scanned collision entries which are EMPTY for player in race mode (Ball_Update never called). Also had BASS proxy stack corruption (void WINAPI name() with zero params).
- **v3**: Fixed BASS proxy signatures + velocity-reversal detection that works in all modes.

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll wall_bumper_mod.c \
    -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
    -Wl,--add-stdcall-alias
```
