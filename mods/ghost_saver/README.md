# Ghost Saver Mod

Saves Time Trial ghost data to `GHOST.txt` so ghost recordings persist across game restarts. The vanilla game stores ghost data only in memory — it vanishes when you quit. This mod makes ghosts permanent.

## How It Works

1. **On race start**: If `GHOST.txt` contains saved data for the current race name, the mod loads it into the game's playback buffer (`App+0x910`) so the ghost ball appears immediately.
2. **During race**: A polling thread (60Hz) records ball position/velocity/rotation into an internal buffer, mirroring the game's own recording.
3. **On race finish**: When the ball crosses the goal, the mod compares the finish time against the saved best time. If faster, it overwrites the saved data. If slower, it discards the new data.

## Installation

1. Rename the original `bass.dll` to `bass_real.dll`
2. Copy `bass.dll` from this mod into the game folder
3. The mod auto-creates `GHOST.txt` and `ghost_saver_log.txt` on first run

## GHOST.txt Format

```
[RACE:Warm-Up Race]
TIME=12345
FRAMES=1500
0.000 1.000 2.000 0.100 0.200 3.000 0 4.000 5.000 26.000
...
[END]
```

- `TIME` — race finish time in game ticks (lower = better)
- `FRAMES` — number of recorded snapshots
- Each data line is: `pos_x pos_y pos_z vel_x vel_y rotation state rot_x rot_y radius`

Multiple races are stored as separate `[RACE:name]...[END]` blocks.

## Technical Details

| Property | Value |
|----------|-------|
| Approach | Non-invasive polling thread (no hooks, no patches) |
| Thread rate | 60Hz (16ms sleep) |
| Max snapshots per race | 5000 (~83 seconds at 60fps) |
| File format | Plain text, one snapshot per line |
| Memory per race | ~200KB (5000 × 40 bytes) |

### Key Offsets Used

| Offset | Purpose |
|--------|---------|
| `App+0x90C` | BestTimeTracker* recording buffer (new race) |
| `App+0x910` | BestTimeTracker* playback buffer (ghost) |
| `App+0x5DC` | Current player ball pointer |
| `App+0x5D6` | Goal flag (BYTE, set when player crosses finish) |
| `App+0x5E8` | Race timer |
| `App+0x234` | Party mode flag (0 = Time Trial) |
| `BTT+0x424` | Race name string |
| `BTT+0x524` | Best time (dword, 9999999 = no time) |
| `Ball+0x164/168/16C` | Position X/Y/Z |
| `Ball+0x190/194` | Velocity X/Y |
| `Ball+0x150` | Rotation |
| `Ball+0x284` | Radius |

### Crash Test

- hbtestd: 39.01s, no crash ✓
- No IAT hooks, no code patches, no threads modifying game state
- All memory reads guarded with `IsBadReadPtr`

## Limitations

- Ghost data is loaded **after** the race starts (detected via polling). There may be a 1-frame delay before the ghost ball appears.
- The mod creates its own `BestTimeTracker` object via `operator_new` and `BestTimeTracker_ctor`. If the game already created one at `App+0x910`, the mod frees it first via its vtable destructor.
- `GHOST.txt` is plain text — a 60-second race at 60fps produces ~1500 lines (~72KB). The file can grow large if many races are saved.
- The mod only activates in Time Trial mode (`profile+0x11 != 0` and `App+0x234 == 0`).
