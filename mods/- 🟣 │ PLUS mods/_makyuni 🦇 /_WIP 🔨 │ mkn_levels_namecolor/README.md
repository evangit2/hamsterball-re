# mkn_levels_namecolor

**Author:** MAKYUNI 🦇  
**Type:** bass.dll proxy mod (MinGW)  
**Config file:** `mkn_levels_namecolor.txt` (auto-generated next to DLL)

## Features

Customizes per-level and per-arena **names** and **colors** in Hamsterball:

### LEVELS (Tournament / Time Trials / 2P Party)
- **Timer blot color** — the colored oval + text during gameplay (`board:LevelName=`)
- **Menu text color** — race selection menu name color (`menu:LevelName=`)
- **Both at once** — `LevelName=#RRGGBB`
- **Custom name strings** — `name:LevelName="CUSTOM TEXT"`

### ARENAS (4P Rumble)
- **Timer blot color** — same as levels, for arena gameplay
- **Menu text color** — arena selection menu name color
- **Both at once** — `ArenaName=#RRGGBB`
- **Custom name strings** — `name:ArenaName="CUSTOM ARENA"`

## Installation

1. Rename original `bass.dll` → `bass_real.dll` in the Hamsterball game folder
2. Copy `mkn_levels_namecolor.dll` as `bass.dll` into the game folder
3. Copy `mkn_levels_namecolor.txt` next to `bass.dll`
4. Launch Hamsterball — the mod auto-creates the config if missing

## Config Format

```ini
# ============== LEVELS ==============
name:WarmUp=        "WARM-UP RACE"
WarmUp=             #FF00FF

# ============== ARENAS ==============
name:WarmUp=        "WARM-UP ARENA"
WarmUp=             #FF00FF
```

### Prefixes
| Prefix | Effect |
|--------|--------|
| `LevelName=#RRGGBB` | Both timer blot AND menu text |
| `board:LevelName=#RRGGBB` | Timer blot only |
| `menu:LevelName=#RRGGBB` | Menu text only |
| `name:LevelName="text"` | Custom display name |

### Level/Arena Names (config keys)
`WarmUp`, `Beginner`, `Intermediate`, `Dizzy`, `Tower`, `Up`, `Neon`, `Expert`, `Odd`, `Toob`, `Wobbly`, `Glass`, `Sky`, `Master`, `Impossible`

## How It Works

1. **Timer blot colors**: Background thread polls `board+0x1508` (R/G/B floats) every 100ms, matching the current level/arena by `board+0x29B4` name string.

2. **Menu text colors**: Patches `PUSH` float immediates in `PracticeMenu_ctor` (0x42EA30) and `ArenaMenu_ctor` (0x42FC40). Levels with `push 0` (6a 00) instructions use code caves.

3. **Custom names**: Allocates writable string buffers and patches the `PUSH <string_ptr>` operands in both menu constructors to point to these buffers. Updated on config reload.

## Technical Details

- **PracticeMenu_ctor** at 0x42EA30: 15 levels, each with 4 PUSH floats (A,B,G,R) + PUSH string + CALL UIList_AddItem
- **ArenaMenu_ctor** at 0x42FC40: 15 arenas, same structure
- **Code caves**: Neon, Odd, Impossible (both levels and arenas) use `6a 00` (push 0) for zero-valued color components — code caves replace these with proper `push imm32` sequences
- **Config re-read**: Every 2 seconds, runtime hot-reloadable
- **Board color polling**: Every 100ms (game writes colors once in board ctor)

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll mkn_levels_namecolor.c \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```
