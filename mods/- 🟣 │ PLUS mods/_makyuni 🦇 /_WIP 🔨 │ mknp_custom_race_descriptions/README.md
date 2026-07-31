# Custom Race Descriptions

Customize the 15 tournament race description texts shown on the Tourney Menu screen.

## How It Works

The original game stores race descriptions as a hardcoded pointer table at `0x4F7148` (15 `char*` entries, one per race). `TourneyMenu_ctor` (at `0x44FDA0`) indexes this table by `race_idx` and passes the result to `Font_WordWrap()` for display.

This mod overwrites those pointers at runtime to point to strings loaded from a `.txt` file. No code patching needed — just pointer table replacement.

## Installation

1. Rename your original `bass.dll` to `bass_real.dll`
2. Copy the modded `bass.dll` into your Hamsterball game folder
3. On first launch, `race_descriptions.txt` will be auto-generated next to `bass.dll` with the original descriptions as a template
4. Edit the `.txt` file and restart the game to see your custom descriptions

## Config File Format

```
[Level 1]  # Warm-Up Race
Take your time on the Warm-Up race! This easy little...

[Level 2]  # Beginner Race
Now let's try something a little more interesting...

# Lines starting with # or ; are comments
# Missing levels use the original game description
# Multi-line descriptions are joined with spaces
```

## Race Order

| Level | Race | Original Description (truncated) |
|-------|------|-----------------------------------|
| 1 | Warm-Up | TAKE YOUR TIME ON THE WARM-UP RACE!... |
| 2 | Beginner | NOW LET'S TRY SOMETHING A LITTLE MORE INTERESTING... |
| 3 | Intermediate | OKAY, YOU'RE READY FOR THE INTERMEDIATE RACE... |
| 4 | Dizzy | ROUND AND ROUND SHE GOES. THE DIZZY RACE... |
| 5 | Tower | IT'S NOT A MEDIEVAL TORTURE DEVICE... |
| 6 | Up | UP, UP AND AWAY! AND NOW FOR SOMETHING COMPLETELY DIFFERENT... |
| 7 | Neon | IT'S THE BIG BLACKOUT! HAMSTERS MIGHT BE NOCTURNAL... |
| 8 | Expert | OKAY, YOU'VE PASSED THE KIDDIE RACES... |
| 9 | Odd | IT'S CALLED THE 'ODD RACE' BECAUSE... |
| 10 | Toob | TOOBS AND HAMSTERS GO TOGETHER LIKE PEANUT BUTTER... |
| 11 | Wobbly | CREAK TO THE LEFT.... CREAK TO THE RIGHT... |
| 12 | Glass | FROM THE DERANGED MIND OF A MAD GLASSBLOWER... |
| 13 | Sky | UP HERE IN THE SKY, YOUR BIGGEST PROBLEM... |
| 14 | Master | ARE YOU READY FOR THE MASTER'S CHALLENGE?... |
| 15 | Impossible | THE IMPOSSIBLE RACE. WELL, IT'S NOT REALLY IMPOSSIBLE... |

## Technical Details

- **Pointer table**: `0x4F7148` (`.data` section, 15 × 4 bytes)
- **Consumer**: `TourneyMenu_ctor` at `0x44FDA0`, specifically `Font_WordWrap(font, desc_table[race_idx], 0x177, dest_buffer)`
- **Word wrap width**: 375 pixels (0x177)
- **Approach**: Overwrite pointers in the table to point to heap-allocated custom strings. No code cave, no hooks, no threads.

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll custom_race_descriptions.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc -Wl,--add-stdcall-alias
```

## Files

- `custom_race_descriptions.c` — Source code
- `bass.dll` — Compiled mod DLL
- `race_descriptions.txt` — Default config file with original descriptions
