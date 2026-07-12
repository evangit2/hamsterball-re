# Level Colors Mod

Changes the per-level base colors (timer oval, timer text, and race selection menu text) based on a `colors.txt` config file.

## Installation

1. Rename original `bass.dll` → `bass_real.dll` in the Hamsterball game folder
2. Copy the mod `bass.dll` into the game folder
3. Launch Hamsterball — the mod auto-creates `colors.txt` on first run

## Config File

The mod reads `colors.txt` next to `bass.dll`. Edit it at runtime — changes apply within 2 seconds.

### Format

```
; LevelName=RRGGBB (hex RGB, like HTML colors)
; Lines starting with ; or # are comments
;
; Prefixes:
;   LevelName=RRGGBB       — applies to BOTH timer and menu
;   board:LevelName=RRGGBB — only timer oval/text during gameplay
;   menu:LevelName=RRGGBB  — only race selection menu text

WarmUp=FF00FF
Beginner=FFBF40
Intermediate=0000FF
Dizzy=00FF00
Tower=FFBF00
Up=FF00FF
Neon=FFFF00
Expert=FF0000
Odd=FF8000
Toob=8080FF
Wobbly=9ED64D
Glass=FF00FF
Sky=0080FF
Master=808080
Impossible=FF0000
```

### Level Names

| Name | Level |
|------|-------|
| WarmUp | Warm-Up Race |
| Beginner | Beginner Race |
| Intermediate | Intermediate Race |
| Dizzy | Dizzy Race |
| Tower | Tower Race |
| Up | Up Race |
| Neon | Neon Race |
| Expert | Expert Race |
| Odd | Odd Race |
| Toob | Toob Race |
| Wobbly | Wobbly Race |
| Glass | Glass Race |
| Sky | Sky Race |
| Master | Master Race |
| Impossible | Impossible Race |

## What It Changes

### Board Colors (timer oval + timer text)

During gameplay, the timer oval background (`timerblot.png`) and the countdown numbers are tinted with each level's color. The mod writes to `board+0x1508` (R), `+0x150C` (G), `+0x1510` (B) at runtime via a background thread polling every 100ms.

When the timer is running low, the game's own code zeros the G and B channels, making the timer turn red as a warning — this behavior is preserved.

### Menu Colors (race selection text)

In the Practice/Time Trial race selection menu, each level name is displayed in its own color. The mod patches the hardcoded float constants in `PracticeMenu_ctor` (0x0042EA30) using `VirtualProtect` to make the `.text` section writable, then overwrites the `PUSH` immediate operands.

These patches are applied once at startup and persist for the session.

## How It Works

The mod uses two independent mechanisms:

1. **Board colors**: Background thread polls the active board struct and writes RGB floats. The game sets these once in the BoardLevel constructor via `Vec3_Init`, so the mod must re-apply them continuously.

2. **Menu colors**: Code patching. Each `Matrix_Scale4x4` call in `PracticeMenu_ctor` has four `PUSH` instructions with float immediates (A, B, G, R in cdecl reverse order). The mod patches the 4-byte float operands directly in the `.text` section.

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll level_colors.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```

## Technical Details

### Board Color Patch Points

| Offset | Field | Type |
|--------|-------|------|
| board+0x1508 | R | float |
| board+0x150C | G | float |
| board+0x1510 | B | float |
| board+0x1514 | A | float (always 1.0) |
| board+0x4340 | Scale | float (always 1.0) |

### Menu Color Patch Addresses

Each level has three patch points (R, G, B float operands in PUSH instructions):

| Level | R addr | G addr | B addr |
|-------|--------|--------|--------|
| WarmUp | 0x0042EE94 | 0x0042EE8F | 0x0042EE8A |
| Beginner | 0x0042EED3 | 0x0042EECE | 0x0042EEC9 |
| Intermediate | 0x0042EF0D | 0x0042EF08 | 0x0042EF03 |
| Dizzy | 0x0042EF60 | 0x0042EF5B | 0x0042EF56 |
| Tower | 0x0042EFDA | 0x0042EFD5 | 0x0042EFD0 |
| Up | 0x0042F054 | 0x0042F04F | 0x0042F04A |
| Neon | 0x0042F0D4 | 0x0042F0CF | 0x0042F0CD |
| Expert | 0x0042F14E | 0x0042F149 | 0x0042F144 |
| Odd | 0x0042F1C5 | 0x0042F1C0 | 0x0042F1BE |
| Toob | 0x0042F248 | 0x0042F243 | 0x0042F23E |
| Wobbly | 0x0042F2C2 | 0x0042F2BD | 0x0042F2B8 |
| Glass | 0x0042F33C | 0x0042F337 | 0x0042F332 |
| Sky | 0x0042F3BF | 0x0042F3BA | 0x0042F3B5 |
| Master | 0x0042F439 | 0x0042F434 | 0x0042F42F |
| Impossible | 0x0042F4AD | 0x0042F4AB | 0x0042F4A9 |

See `docs/rendering/LEVEL_COLOR_SYSTEM.md` for the full RE analysis.
