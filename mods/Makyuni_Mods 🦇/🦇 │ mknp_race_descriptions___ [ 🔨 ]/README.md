# mknp_race_descriptions (HB+ v2.1)

Custom tournament race texts for Hamsterball Plus.

## What It Does

Replaces the 15 tournament race description texts on the Tourney Menu screen with text from `mknp_race_descriptions.txt`. Missing levels keep the stock game description. The game word-wraps long texts automatically (375px).

## Installation (Hamsterball Plus required)

1. Place `mknp_race_descriptions.dll` in the game's `Mods\` folder.
2. Launch the game. The mod auto-loads via HB+ and auto-generates `mknp_race_descriptions.txt` next to the DLL (prefilled with the stock texts).
3. Edit any `[Level N]` section (game restart required).

## Configuration

Edit `mknp_race_descriptions.txt` (next to the DLL in `Mods\`, NOT the game root):

```
[Level 1]
Take your time on the Warm-Up race! ...

[Level 2]
Now let's try something a little more interesting...
```

- `[Level N]`, N = 1-15. Everything between headers is the description (multi-line joined with spaces).
- Lines starting with `#` or `;` are comments.

## Technical Details

- **Target**: description pointer table at `0x4F7148` (15 `char*`, `.data`). Pointers overwritten one-shot in `Initialize()` — no code patching.
- **No threads** — one-shot apply, nothing per-tick.

## Files

- `mknp_race_descriptions.dll` — compiled HB+ mod
- `mknp_race_descriptions.cpp` — source code (MinGW, nocrt, manual 17-entry vtable)
- `mknp_race_descriptions.txt` — auto-generated config file (lives next to the DLL)

## Building

- **MinGW (Linux):** `cd source && ./build.sh` → `mknp_race_descriptions.dll`

## Testing note

The HB+ loading chain (scanning `Mods\`, calling `CreateModInstance`, firing `Initialize()`) cannot be verified under Wine/hbtestd. Only real Windows with HB+ installed can confirm the full load + in-game behavior.

## Author

MAKYUNI / Hamsterbot
