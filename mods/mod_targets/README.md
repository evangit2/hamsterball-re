# Hamsterball Mod Manager (mod_targets)

## Overview

**mod_targets** is a mod manager DLL for Hamsterball that acts as a central controller for all other DLL mods in your game folder. When this DLL is present (installed as `bass.dll`), it **deactivates all mods by default** and only activates the ones you specify in a configuration file.

## How It Works

1. The game loads `bass.dll` at startup (this mod).
2. The mod reads `__mod_targets.txt` from the game directory.
3. It loads **GLOBAL** mods immediately.
4. A background thread polls the game's memory every 500ms to detect which level/arena is currently being played.
5. When the level/arena changes, it **loads** mods needed for the new context and **unloads** mods that are no longer needed.

## Installation

1. Rename your existing `bass.dll` to `bass_real.dll` (if not already done).
2. Copy this mod's `bass.dll` into the game folder.
3. Create a `mods/` subfolder inside the game folder (if it doesn't exist).
4. Place any mod DLLs you want managed inside the `mods/` folder.
5. Create `__mod_targets.txt` in the game folder (see below).

## Config File: `__mod_targets.txt`

Place this file in the **game root folder** (next to `Hamsterball.exe`). If it doesn't exist, the mod will auto-generate a blank template on first launch.

### Format

```
# Lines starting with # are comments.
# Mods not listed here are deactivated by default.
# Separate multiple mods with commas.

GLOBAL = mod_a.dll, mod_b.dll

LEVEL 1 = mod_c.dll
LEVEL 2 = mod_d.dll, mod_e.dll
LEVEL 3 =
...
LEVEL 15 =

ARENA 1 = mod_f.dll
ARENA 2 =
...
ARENA 15 =
```

### Sections

| Section | Description |
|---------|-------------|
| `GLOBAL` | Mods loaded globally for the entire game session. |
| `LEVEL 1-15` | Mods loaded only during the specified race. Unloaded when the race ends. |
| `ARENA 1-15` | Mods loaded only during the specified arena. Unloaded when the arena ends. |

### Level/Arena Order

| # | Race | Arena |
|---|------|-------|
| 1 | Warm-Up | Warm-Up |
| 2 | Beginner | Beginner |
| 3 | Intermediate | Intermediate |
| 4 | Dizzy | Dizzy |
| 5 | Tower | Tower |
| 6 | Up | Up |
| 7 | Neon | Neon |
| 8 | Expert | Expert |
| 9 | Odd | Odd |
| 10 | Toob | Toob |
| 11 | Wobbly | Wobbly |
| 12 | Glass | Glass |
| 13 | Sky | Sky |
| 14 | Master | Master |
| 15 | Impossible | Impossible |

## Detection Method

The mod detects the current level/arena by reading the race name string from game memory:
- `g_App` (0x005341E0) → `+0x220` (PlayerProfile) → `+0xC` (Board) → `+0x29B4` (race name)

The race name is set by each board constructor (e.g., `LevelBoard_WarmUp_ctor` sets "WARM-UP RACE"). Arena names contain "ARENA" instead of "RACE".

## Log File

The mod writes a log to `mod_targets_log.txt` in the game folder. This tracks:
- Startup info (game dir, mods dir)
- Config parsing results
- Context changes (menu → level → arena)
- Mod load/unload events

## Technical Notes

- **BASS proxy**: This mod replaces `bass.dll` and forwards all 10 BASS audio functions to `bass_real.dll`. Audio works normally if `bass_real.dll` is present.
- **Memory safety**: All game memory reads use `IsBadReadPtr` guards to prevent crashes during transitions.
- **Thread safety**: Mod load/unload operations are protected by a critical section.
- **Polling interval**: 500ms — fast enough to catch level transitions without noticeable performance impact.
- **Mod DLLs**: Each managed mod DLL should export a `DllMain` function. The mod manager simply calls `LoadLibrary`/`FreeLibrary` to activate/deactivate mods.
- **Mod naming**: You can specify mods with or without the `.dll` extension in the config file. The manager will append `.dll` automatically if missing.

## Limitations

- Only manages DLLs placed in the `mods/` subfolder. DLLs directly in the game root (other than `bass.dll` itself) are not managed.
- The mod unloads DLLs with `FreeLibrary`, which may not fully clean up resources if a mod has background threads or hooks still active. This is a limitation of the DLL loading mechanism.
- If a managed mod also acts as a `bass.dll` proxy, it should be the only one loaded at a time (audio conflicts).

## Build

### Cross-compile (MinGW on Linux)
```bash
i686-w64-mingw32-gcc -shared -o bass.dll mod_targets.c \
  -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```

### Native (MSVC on Windows)
```cmd
cl /LD mod_targets.c /Fe:bass.dll /link winmm.lib
```
