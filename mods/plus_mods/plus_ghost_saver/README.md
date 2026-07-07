# Ghost Saver (HB+ API Mod)

Saves and loads persistent ghost data for Hamsterball Time Trial mode. Ghost recordings survive game restarts, stored as binary `.ghost` files in the `Ghosts/` directory.

## Features

- **Per-race ghost files**: Each race gets its own `.ghost` file (e.g. `Warm-Up.ghost`)
- **Previous run**: Always saves the most recent run as `Previous_Run.ghost`
- **Best time comparison**: Only overwrites a race's ghost if the new time is faster
- **Toggle button**: "Save Ghosts" toggle in the options menu (default ON)
- **Binary format**: Compact binary format (40 bytes per frame vs ~80 bytes hex text)
- **Atomic saves**: Writes to `.tmp` file then renames — no corruption on crash

## How It Works

1. **On race start**: Hooks `App_StartPracticeRace` via `RegisterCustomHook`. Before the original function runs, it loads the saved ghost from disk and injects it into `App+0x910` (the playback BTT). This way `Board_ctor` sees the ghost BTT and creates a ghost ball at `scene+0x361C`.

2. **During the race**: Monitors the goal flag (`App+0x5D6`) for a 0→1 transition via `onGameUpdate()`.

3. **On goal**: Reads the game's own recording from the BTT's AthenaList at `App+0x90C`, saves it to a `.ghost` file if the time is better than the existing ghost.

4. **On scene end**: Resets recording state.

## Files

- `GhostSaver.h` — Constants, globals, function declarations
- `GhostSaverUtils.cpp` — Race name lookup, filename conversion, file I/O, state checks
- `GhostSaverHook.cpp` — `App_StartPracticeRace` hook and ghost injection logic
- `GhostSaver.cpp` — Mod class (`Initialize`, `onGameUpdate`, `onSceneEnd`, `onLevelStart`)

## Game Memory Layout

| Offset | Field | Description |
|--------|-------|-------------|
| `App+0x90C` | BestTimeTracker* | Recording (current run) |
| `App+0x910` | BestTimeTracker* | Playback (ghost ball) |
| `App+0x220` | PlayerProfile* | Profile pointer |
| `App+0x234` | BYTE | Party mode (0 = Time Trial) |
| `App+0x5D6` | BYTE | Goal flag (0→1 = finished) |
| `BTT+0x004` | AthenaList | Embedded snapshot list |
| `BTT+0x008` | DWORD | AthenaList count |
| `BTT+0x410` | DWORD* | AthenaList data array |
| `BTT+0x41C` | DWORD | Playback index |
| `BTT+0x424` | char[128] | Race name |
| `BTT+0x524` | DWORD | Best time (ticks) |
| `BTT+0x528` | — | Total BTT size |

## Ghost File Format

```
[4 bytes] magic    = 0x47485347 ("GHSG")
[4 bytes] version  = 1
[4 bytes] time     (game ticks, lower = better)
[4 bytes] frame_count
[frame_count * 40 bytes] snapshots (10 DWORDs each)
```

## Conversion Notes

Converted from `ghost_saver.c` v25.4 (bass.dll proxy, 1136 lines C with inline asm) to HB+ API (4 files, C++ with `Call<>`/`CallMethod<>` templates).

Key simplifications:
- **No background thread** — uses `onGameUpdate()` callback (fires every tick on main thread)
- **No CRITICAL_SECTION** — everything runs on main thread, no race conditions
- **No inline asm** — uses `Call<>`/`CallMethod<>` templates from `HamsterballAPI.h`
- **No manual detour** — uses `api->RegisterCustomHook()` instead
- **No BASS proxy** — HB+ framework handles DLL loading

## Author

BookwormKevin (original logic by Hamsterbot, converted to HB+ API)
