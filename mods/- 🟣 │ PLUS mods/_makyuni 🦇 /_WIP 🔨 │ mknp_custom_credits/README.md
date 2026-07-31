# custom_credits

Editable Credits Screen Text mod for Hamsterball.

## What It Does

Replaces the hard-coded credits strings in the CREDITS screen with user-editable text from `mknp_custom_credits.txt`. The credits screen has 44 lines (30 text + 14 blank separators) that scroll vertically — all of them can be customized.

## Installation

1. Rename the original `mknp_custom_credits.dll` to `bass_real.dll`
2. Copy this mod's `mknp_custom_credits.dll` into the game folder
3. Edit `mknp_custom_credits.txt` to change any of the 44 lines

The mod auto-generates `mknp_custom_credits.txt` with default values on first launch if the file is missing.

## Config Format

```
# Comment lines start with #
# N=text   — Line N (1-44) displays "text" in the credits
# N=       — Empty = blank separator line

1=-RAPTISOFT GAMES PRESENTS
2=HAMSTERBALL
3=
4=
5=-GAME DESIGN
6=JOHN RAPTIS
...
```

- Lines 1-44 correspond to the 44 entries in the credits scroll
- Empty values (`N=`) produce blank separator lines
- Text can be up to 80 characters per line
- Changes require a game restart (config is read at launch)

## Original Credits Layout

| Line | Text |
|------|------|
| 1 | -RAPTISOFT GAMES PRESENTS |
| 2 | HAMSTERBALL |
| 3-4 | *(blank)* |
| 5 | -GAME DESIGN |
| 6 | JOHN RAPTIS |
| 7 | *(blank)* |
| 8 | -PROGRAMMING |
| 9 | NICK RAPTIS |
| 10 | *(blank)* |
| 11 | -PHYSICS |
| 12 | PAUL NETTLE |
| 13 | *www.fluidstudios.com |
| 14 | *(blank)* |
| 15 | -LEVEL AND CREATURE DESIGN |
| 16 | JOHN RAPTIS |
| 17 | GARY CLAIR |
| 18 | *www.garyclair.com |
| 19 | *(blank)* |
| 20 | -MUSIC |
| 21 | SKAVEN |
| 22 | *www.futurecrew.com/skaven |
| 23 | *(blank)* |
| 24 | -RODENT MODELS |
| 25 | ENDBOSSGAMES |
| 26 | *www.endbossgames.com |
| 27 | *(blank)* |
| 28 | -LEVEL MODELS |
| 29 | STEVE WARNER |
| 30 | *(blank)* |
| 31 | -PLAY BALANCING |
| 32 | BRADY WRIGHT |
| 33 | *www.anbsoft.com |
| 34 | *(blank)* |
| 35 | -GAME INSTALLER |
| 36 | BRIAN FIETE |
| 37 | *www.popcap.com |
| 38-39 | *(blank)* |
| 40 | -AND SPECIAL THANKS TO |
| 41 | -ALL THE FINE FOLKS AT |
| 42 | -FLIPCODE.COM! |
| 43-44 | *(blank)* |

## Technical Details

- **Target function**: `CreditsScreen_ctor` at `0x4254E0`
- **Method**: 44 `PUSH imm32` instructions (opcode `0x68`) are patched — each 4-byte string pointer immediate is overwritten to point at a user-controlled string buffer in the DLL's `.data` section
- **String source**: Original strings live in `.rdata` at `0x4D233C`–`0x4D2520`
- **No code caves or hooks** — pure 4-byte data writes via `VirtualProtect`
