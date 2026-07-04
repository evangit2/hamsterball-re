# Ghost Saver Mod (v19)

Saves Time Trial ghost data to `GHOST.txt` so ghost recordings persist across game restarts. The vanilla game stores ghost data only in memory — it vanishes when you quit. This mod makes ghosts permanent.

## How It Works

1. **On race start**: An `App_StartPracticeRace` detour hook fires. BEFORE calling the original function, the mod pre-populates `App+0x910` (playback buffer) from `GHOST.txt` and creates a dummy `App+0x90C` (recording buffer) with NO_TIME. This ensures `Board_ctor` (called inside `App_StartPracticeRace`) sees `App+0x910 != NULL` and creates the ghost ball at `scene+0x361C`. The dummy recording protects the playback from being destroyed by the game's BTT management code.
2. **During race**: A polling thread (60Hz) records ball position/velocity/rotation into an internal buffer, mirroring the game's own recording.
3. **On race finish**: When the ball crosses the goal, the mod compares the finish time against the saved best time. If faster (or no saved ghost exists), it overwrites the saved data. If slower, it discards the new data.

## Installation

1. Rename the original `bass.dll` to `bass_real.dll`
2. Copy `bass.dll` from this mod into the game folder
3. The mod auto-creates `GHOST.txt` and `ghost_saver_log.txt` on first run

## GHOST.txt Format

```
[RACE:Warm-Up Race]
TIME=12345
FRAMES=1500
0x00000000 0x3F800000 0x40000000 0x3F800000 0x3F800000 0x00000000 0x00000000 0x00000000 0x00000000 0x41D00000
...
[END]
```

- `TIME` — race finish time in game ticks (lower = better)
- `FRAMES` — number of recorded snapshots
- Each data line is 10 raw DWORDs in hex (exact bit patterns, not floats)
- Snapshot fields: `pos_x pos_y pos_z facing_x facing_z roll_angle gravity_plane surface_blend_a surface_blend_b radius`

Multiple races are stored as separate `[RACE:name]...[END]` blocks.

## Technical Details

| Property | Value |
|----------|-------|
| Approach | Detour hook on `App_StartPracticeRace` (0x428C50) + 60Hz polling thread |
| Hook type | 7-byte code patch (5-byte JMP + 2 NOPs) with executable trampoline |
| Thread rate | 60Hz (16ms sleep) |
| Max snapshots per race | 5000 (~83 seconds at 60fps) |
| File format | Plain text, raw DWORD hex per line |
| Memory per race | ~200KB (5000 × 40 bytes) |
| BTT allocation | Game's own `operator_new` (0x4BA57B) — ensures heap consistency with game's `operator_delete` |
| Time Trial only | Hook checks `is_time_trial_active()` before injecting ghost |
| Time comparison | New ghost only saved if finish time < existing saved time (or no existing ghost) |

### Key Offsets Used

| Offset | Purpose |
|--------|---------|
| `App+0x90C` | BestTimeTracker* recording buffer (new race) |
| `App+0x910` | BestTimeTracker* playback buffer (ghost) |
| `App+0x5DC` | Current player ball pointer (primary) |
| `App+0x220→profile+0x0C→board+0x878→scene+0x29D0` | Ball pointer fallback chain |
| `App+0x5D6` | Goal flag (BYTE, set when player crosses finish) |
| `App+0x234` | Party mode flag (0 = Time Trial) |
| `BTT+0x424` | Race name string |
| `BTT+0x524` | Best time (dword, 9999999 = no time) |
| `Ball+0x164/168/16C` | Position X/Y/Z |
| `Ball+0x190/194` | Facing direction X/Z |
| `Ball+0x150` | Roll angle |
| `Ball+0x748` | Gravity plane (byte, read as DWORD) |
| `Ball+0x74C/750` | Surface blend A/B |
| `Ball+0x284` | Radius |

## Limitations

- The mod only activates in Time Trial mode (`profile+0x11 != 0` and `App+0x234 == 0`).
- The `App_StartPracticeRace` hook is a code patch — it modifies 7 bytes of executable code at `0x428C50` with a JMP to the mod's hook function, backed by an executable trampoline that preserves the original instructions.
- `GHOST.txt` is plain text — a 60-second race at 60fps produces ~1500 lines (~72KB). The file can grow large if many races are saved.
- The mod allocates the `BestTimeTracker` and snapshots via the game's own `operator_new` (0x4BA57B) so the game can safely free them via `operator_delete` when the level ends. Using `malloc` here would cause heap corruption.
- Ball pointer is fetched via `App+0x5DC` (primary) with a fallback to `App→Profile→Board→Scene→Scene+0x29D0` if the primary pointer is stale.
