# Difficulty Settings Mod

Difficulty-based entity replacement for Hamsterball. Reads a `difficulty_settings.txt` config file next to `bass.dll` that maps entity names to replacements depending on tournament difficulty (Pipsqueak / Normal / Frenzied!).

## How It Works

The mod hooks `Scene_CreateEntities` (0x0041C5B0) — the dispatch function that runs **before** all entity factories (CreateBadBall, CreateMouseTrap, CreateLevelObjects, CreateExpertLevelObjects). Before the original function runs, the mod:

1. **Reads the original difficulty** (`App+0x23C`, 0=Pipsqueak, 1=Normal, 2=Frenzied!) to select the correct replacement table.

2. **Iterates the MeshWorld entity list** and replaces entity name pointers based on the config:
   - **NOTHING**: Entity name is replaced with `"REF:NOTHING"` — no factory matches it, so the entity is silently skipped.
   - **Replacement name** (e.g. `BONK`): Entity name is replaced — the original factory won't match, but the replacement's factory will create an object at the same position.

3. **Temporarily overrides `App+0x23C` to 1 (Normal)** before calling the original function. This is necessary because the game's factory functions independently check `App+0x23C != 0` before spawning entities. On Pipsqueak, these gates would skip all factory calls entirely, making the mod's name replacements useless. The mod restores the original difficulty value immediately after entity creation completes.

## Config Format

```ini
; difficulty_settings.txt
; NOTHING = skip spawning entirely.
; Entity names are case-insensitive. "8ball" = BadBall.
;
; Difficulty: EASY=Pipsqueak, NORMAL=Normal, HARD=Frenzied!

EASY
8ball = NOTHING
Mousetrap = NOTHING

NORMAL
8ball = 8ball
Mousetrap = Mousetrap

HARD
8ball = Bonk
Mousetrap = Mousetrap
```

### Supported Entities

| Config Name | Internal Name | Factory | Notes |
|---|---|---|---|
| `8ball` | `BADBALL` | CreateBadBall | Iterating factory |
| `Mousetrap` | `MOUSETRAP` | CreateMouseTrap | Iterating factory |
| `Bonk` | `BONK` | CreateLevelObjects / Expert | Per-entity |
| `Tipper` | `TIPPER` | CreateLevelObjects | Per-entity |
| `Bridge` | `BRIDGE` | CreateLevelObjects / Expert | Per-entity |
| `Fan` | `FAN` | CreateExpertLevelObjects | Per-entity |
| `Sawblade` | `SAWBLADE` | CreateExpertLevelObjects | Per-entity |
| `Judge` | `JUDGE` | CreateExpertLevelObjects | Per-entity |
| `Bell` | `BELL` | CreateExpertLevelObjects | Per-entity |
| `Bbridge1` | `BBRIDGE1` | CreateLevelObjects | Per-entity |
| `Bbridge2` | `BBRIDGE2` | CreateLevelObjects | Per-entity |
| `PopCylinder` | `POPCYLINDER` | CreateLevelObjects | Per-entity |
| `Blockdawg1` | `BLOCKDAWG1` | CreateLevelObjects | Per-entity |
| `Blockdawg2` | `BLOCKDAWG2` | CreateLevelObjects | Per-entity |
| `Catapult` | `CATAPULT` | CreateLevelObjects | Per-entity |
| `Gluebie` | `GLUEBIE` | CreateLevelObjects | Per-entity |

## Installation

1. Rename the original `bass.dll` to `bass_real.dll`
2. Copy `bass.dll` from this mod into the game folder
3. Edit `difficulty_settings.txt` to customize entity spawns per difficulty
4. The mod auto-generates a default config on first launch

## Config Hot-Reload

The mod re-reads `difficulty_settings.txt` every 2 seconds, so you can edit the file while the game is running and changes take effect on the next race.

## Technical Details

- **Hook target**: `Scene_CreateEntities` at 0x0041C5B0 (__fastcall, ECX=board)
- **Trampoline**: Copies original 7-byte prologue (PUSH -1 + PUSH handler), JMP back to target+7
- **Entity list access**: `board+0x8AC → MeshWorld → +0x480 → entity_list → +0x898 (count) → +0xCA0 → *(data_array)`
- **Difficulty**: `App+0x23C` (0=Pipsqueak, 1=Normal, 2=Frenzied)

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll difficulty_settings.c \
  -I../shared -lwinmm -Wl,--enable-stdcall-fixup -O2 -static \
  -static-libgcc -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
```
