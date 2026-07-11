# Local Mods — Per-Level/Arena DLL Mod Loader

**Type:** HB+ API v2.0 mod (DLL proxy, loaded by bass.dll)  
**Author:** Hamsterbot  
**Files:** `local_mods.dll` (mod), `local_mods.txt` (config)

## What It Does

This mod lets you load specific DLL mods on a per-level or per-arena basis. Mods placed in the `\Localmods\` folder are NOT auto-loaded by the game — they're only loaded when `local_mods.txt` specifies them for the current level/arena.

## Installation

1. Place `local_mods.dll` in the game's `Mods\` folder (same as other HB+ mods).
2. Place `local_mods.txt` in the game root folder (next to `Hamsterball.exe`).
3. Create a folder called `Localmods` in the game root folder.
4. Put any mod DLLs you want conditionally loaded into `\Localmods\`.

## Config File Format (`local_mods.txt`)

```
==================== HB+  |  LOCAL MODS ====================

LEVELS
1 = (modnameA.dll, modnameB.dll)
2 = (modnameC.dll)
3 = 
...
15 = 

ARENAS
1 = (modnameA.dll)
2 = 
...
15 = 

============================================================
```

- **LEVELS** section: Numbers 1-15 correspond to race levels.
- **ARENAS** section: Numbers 1-15 correspond to arena levels.
- After the `=`, list mod DLL filenames in parentheses, separated by commas.
- Leave empty after `=` to load no mods for that slot.
- The mod DLLs must be in `\Localmods\` folder.

### Level Index Mapping

**Races (LEVELS):**
| # | Level |
|---|-------|
| 1 | Warm-Up |
| 2 | Beginner (Cascade) |
| 3 | Intermediate |
| 4 | Dizzy |
| 5 | Tower |
| 6 | Up |
| 7 | Neon (Dark) |
| 8 | Expert |
| 9 | Odd |
| 10 | Toob |
| 11 | Wobbly |
| 12 | Glass |
| 13 | Sky |
| 14 | Master |
| 15 | Impossible |

**Arenas (ARENAS):**
| # | Arena |
|---|-------|
| 1 | Warm-Up |
| 2 | Beginner |
| 3 | Intermediate |
| 4 | Dizzy |
| 5 | Tower |
| 6 | Up |
| 7 | Neon |
| 8 | Expert |
| 9 | Odd |
| 10 | Toob |
| 11 | Wobbly |
| 12 | Sky |
| 13 | Master |
| 14 | Glass |
| 15 | Impossible |

## How It Works

1. On startup, the mod reads `local_mods.txt` from the game root.
2. On `onLevelStart()`, it identifies the current level/arena via the scene name.
3. It loads (via `LoadLibraryA`) the DLLs listed for that slot from `\Localmods\`.
4. Each loaded DLL must export `CreateModInstance()` returning a `HamsterballAPI*`.
5. The loader calls `Initialize()` then `onLevelStart()` on each loaded mod.
6. All callbacks (`onBallUpdate`, `onGameUpdate`, etc.) are forwarded to loaded mods.
7. On `onSceneEnd()`, loaded mods are notified and then unloaded (`FreeLibrary`).

## Mod DLL Requirements

Mods placed in `\Localmods\` must:
- Be compiled against HB+ API v2.0 (`HAMSTERBALL_API_VERSION 2`)
- Export `CreateModInstance()` returning `HamsterballAPI*`
- Follow the standard HB+ mod structure (derive from `HamsterballAPI`)

## Notes

- The config file is re-read on every level start, so changes take effect immediately.
- Mods are unloaded when the level ends (scene end), then new mods are loaded for the next level.
- If a mod fails to load (missing file, missing export), it's silently skipped.
- Maximum 16 mods per level/arena slot, 64 total loaded at once.
- `local_mods.txt` is auto-generated with empty defaults if not found.

## Build

```bash
# MinGW cross-compile (Linux → Windows 32-bit)
bash build.sh
```

Requires `i686-w64-mingw32-g++` and the `HamsterballAPI.h` header.
