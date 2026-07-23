# mkn_plus_custom_credits

**Author:** MAKYUNI  
**Type:** Hamsterball Plus (HB+) mod  
**Ported from:** bass.dll proxy mod `custom_credits`

## Description

Replaces the hard-coded credits screen text with custom text loaded from a config file. The credits screen has 44 lines (30 text + 14 blank separators). Each line is a PUSH imm32 instruction in CreditsScreen_ctor (0x4254e0); this mod overwrites the string pointers to point to user-defined text.

## Installation

1. Place `mkn_plus_custom_credits.dll` in the Hamsterball `Mods\` folder
2. On first launch, the mod auto-generates `mkn_plus_custom_credits.txt` in the same folder
3. Edit the text file to customize credits text

## Config Format

```
# Custom Credits Configuration
# Edit the text after = on each line (1-44).
# Empty lines (N=) become blank separator lines in the credits scroll.
# Lines starting with # are comments.

1=-RAPTISOFT GAMES PRESENTS
2=HAMSTERBALL
3=
4=
5=-GAME DESIGN
6=JOHN RAPTIS
...
```

- `N=text` — Line N (1-44) gets the text after `=`
- `N=` — Empty text = blank separator line
- `#` — Comment line, ignored

## Build

```bash
cd source && bash build.sh
```

Requires `i686-w64-mingw32-g++` (MinGW cross-compiler).
