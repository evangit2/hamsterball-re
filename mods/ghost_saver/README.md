# Ghost Saver Mod (v24)

Saves Time Trial ghost data to per-race `.ghost` files so ghost recordings persist across game restarts. The vanilla game stores ghost data only in memory — it vanishes when you quit. This mod makes ghosts permanent.

## v24 Changes

- **Dynamic snapshot buffer**: Replaces the fixed 5000-frame static array with `malloc`/`realloc`. Long races (>83 seconds) are no longer truncated — the ghost ball would previously freeze mid-track when it ran out of data.
- **BTT ctor failure leak fix**: If the BTT constructor fails (vtable mismatch), the 528-byte struct is now freed via the game's CRT `_free` (0x4BA74D) instead of being leaked.
- **Thread synchronization**: A `CRITICAL_SECTION` protects all shared state between the detour hook (main thread) and the background monitor thread. Prevents torn reads on race names, stale recording state, and corrupted snapshot buffers during race transitions.

## How It Works

1. **On race start**: A detour hook on `App_StartPracticeRace` (0x428C50) fires. BEFORE calling the original function, the mod injects a saved ghost into `App+0x910` (playback buffer) and creates a dummy `App+0x90C` (recording buffer) with `NO_TIME`. This ensures `Board_ctor` — called inside `App_StartPracticeRace` — sees `App+0x910 != NULL` and creates the ghost ball at `scene+0x361C`. The dummy recording prevents the game's BTT management from destroying the injected playback.
2. **During race**: The game records on its own. The mod's background thread monitors the goal flag (`App+0x5D6`) but does NOT poll the ball position — recording is left entirely to the game's internal BTT system.
3. **On race finish**: When the ball crosses the goal, the mod reads the game's own recording from `App+0x90C`'s AthenaList (one frame per game tick). It saves the run as `PreviousRun.ghost` (always, regardless of time) and as `<RaceName>.ghost` (only if the time beats the existing best, or no ghost exists yet).

## Installation

1. Rename the original `bass.dll` to `bass_real.dll`
2. Copy `bass.dll` from this mod into the game folder
3. The mod auto-creates a `Ghosts/` directory and `ghost_saver_log.txt` on first run

## Ghost File Format (Binary)

```
Offset  Size  Field
0x00    4     Magic = 0x47485347 ("GHSG")
0x04    4     Version = 1
0x08    4     Time (game ticks, lower = better)
0x0C    4     Frame count
0x10    N*40  Snapshots (10 DWORDs each = 40 bytes, raw memory copy)
```

Each snapshot is 10 DWORDs (40 bytes):
- `pos_x pos_y pos_z` — ball position (floats at ball+0x164/0x168/0x16C)
- `facing_x facing_z` — direction vector (ball+0x190/0x194)
- `roll_angle` — accumulated roll (ball+0x150)
- `gravity_plane` — 0=flat, 1=tilted, 2=vertical (ball+0x748)
- `surface_blend_a surface_blend_b` — surface blending (ball+0x74C/0x750)
- `radius` — ball radius (ball+0x284)

A 5000-frame ghost file is ~200KB binary (vs ~400KB with the old hex text format).

## Files

| File | Purpose |
|------|---------|
| `Ghosts/Warm-Up.ghost` | Best ghost for Warm-Up |
| `Ghosts/Beginner.ghost` | Best ghost for Beginner |
| `Ghosts/PreviousRun.ghost` | Always the most recent run (any race) |
| `ghost_saver_log.txt` | Debug log (disabled by default, see `LOGGING_ENABLED`) |

## Technical Details

| Property | Value |
|----------|-------|
| Approach | Detour hook on `App_StartPracticeRace` (0x428C50) + background monitor thread |
| Hook type | 7-byte code patch (5-byte JMP + 2 NOPs) with executable trampoline |
| Thread rate | 60Hz (16ms sleep) — monitors goal flag only, does NOT record ball data |
| Max snapshots per race | Unlimited (dynamic buffer, starts at 5000 and grows) |
| File format | Binary (magic + version + time + count + raw DWORDs) |
| Memory per race | ~200KB (5000 × 40 bytes) |
| BTT allocation | Game's own `operator_new` (0x4BA57B) — ensures heap consistency with game's `operator_delete` |
| Time Trial only | Hook checks `is_time_trial_active()` before injecting ghost |
| Time comparison | New ghost only saved if finish time < existing saved time (or no existing ghost) |

### Key Offsets Used

| Offset | Purpose |
|--------|---------|
| `App+0x90C` | BestTimeTracker* recording buffer (current race, game-managed) |
| `App+0x910` | BestTimeTracker* playback buffer (ghost, injected by mod) |
| `App+0x5D6` | Goal flag (BYTE, set when player crosses finish) |
| `App+0x234` | Party mode flag (0 = Time Trial) |
| `App+0x220→profile+0x0C→board+0x878→scene` | Board/scene pointer chain |
| `BTT+0x004` | Embedded AthenaList (vtable, count, data pointer) |
| `BTT+0x008` | AthenaList count (snapshot count) |
| `BTT+0x410` | AthenaList data array pointer |
| `BTT+0x424` | Race name string |
| `BTT+0x524` | Best time (DWORD, 9999999 = NO_TIME) |
| `0x4F7080` | Static race name table (DWORD pointers, indexed by race_index) |

### Architecture Notes

- **Pre-inject strategy**: The mod sets `App+0x910` BEFORE calling the original `App_StartPracticeRace`. This is critical — `Board_ctor` runs inside `App_StartPracticeRace` and only creates the ghost ball if `App+0x910` is non-NULL at ctor time.
- **Dummy recording**: A BTT with `NO_TIME` is placed at `App+0x90C` to prevent the game's BTT management from destroying the injected playback (if `App+0x90C` is NULL, the game enters a "one is NULL" branch and frees the other).
- **Wrong-race protection**: If `App+0x90C` already has a recording from a previous race, its time is neutralized to `NO_TIME` so it loses the comparison and doesn't replace the injected playback.
- **Memory leak fix (v23)**: The old `App+0x910` BTT is saved before being overwritten, then destroyed after the trampoline returns. The BTT deleting destructor (`vtable[0]`=0x4278C0, `__thiscall(this, flags=1)`, `RET 0x4`) is called with `flags=1` for full destroy — it calls `BestTimeTracker_dtor` (0x427760) to free all snapshots + list, then `operator delete` (0x4BA740) to free the BTT struct itself. This is safe because after the trampoline returns: the old scene is torn down, the old ghost ball is gone, and the game's BTT management has already run on the new BTT (not the old one).
- **Stale goal flag fix (v23)**: On race start, `g_prevGoalFlag` is set to the **current** `App+0x5D6` value instead of being forced to 0. This prevents a stale goal flag from the previous race being misinterpreted as a real goal crossing (which would set `g_raceFinished=1` with 0 snapshots, permanently blocking the real goal detection). Additionally, if 0 snapshots are read at goal time, `g_raceFinished` is reset to 0 so the real goal crossing is detected later.
- **Game's own recording**: The mod reads snapshots from the game's BTT at goal time, not via 60Hz polling. This avoids the 2x playback speed problem (polling at 60Hz when the game records at its internal rate produces double-speed playback).
- **Race name from static table**: The race name is looked up by index from the static table at `0x4F7080` (in the pre-inject hook), not from `BTT+0x424` which can be partially-written early in the race.

## Limitations

- Only activates in Time Trial mode (`profile+0x11 != 0` and `App+0x234 == 0`).
- Old hex-text `.ghost` files from v19 and earlier are NOT readable by v22+. Delete `GHOST.txt` and old `.ghost` files to start fresh.
