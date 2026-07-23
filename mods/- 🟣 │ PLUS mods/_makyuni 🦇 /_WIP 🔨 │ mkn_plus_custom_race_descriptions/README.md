# mkn_plus_custom_race_descriptions

**Author:** MAKYUNI  
**Type:** Hamsterball Plus (HB+) mod  
**Ported from:** bass.dll proxy mod `custom_race_descriptions`

## Description

Allows customizing the 15 tournament race description texts shown on the Tourney Menu screen. The original game stores these as a hardcoded pointer table at 0x4F7148 (15 char* entries, one per race). This mod overwrites those pointers at runtime to point to strings loaded from a config file.

## Installation

1. Place `mkn_plus_custom_race_descriptions.dll` in the Hamsterball `Mods\` folder
2. On first launch, the mod auto-generates `mkn_plus_custom_race_descriptions.txt` in the same folder
3. Edit the text file to customize race descriptions

## Config Format

```
# Race Descriptions Configuration File
# Format:
#   [Level N]
#   Your description text here.
#   Multi-line descriptions are joined with spaces.
#
# Lines starting with # or ; are comments.
# Missing levels will use the original game description.

[Level 1]  # Warm-Up Race
TAKE YOUR TIME ON THE WARM-UP RACE! THIS EASY LITTLE...

[Level 2]  # Beginner Race
NOW LET'S TRY SOMETHING A LITTLE MORE INTERESTING...
```

The game calls Font_WordWrap with the description string, which word-wraps to a 375-pixel width. Long descriptions will wrap automatically.

## Build

```bash
cd source && bash build.sh
```

Requires `i686-w64-mingw32-g++` (MinGW cross-compiler).
