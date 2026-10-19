# mknp_namestrip_colors (HB+ v2.1)

Per-level name-strip colors for Hamsterball Plus.

## What It Does

Custom RGBA colors for the horizontal strip behind the Race/Arena name shown at the start of each race. Stock game uses one global yellow strip for everything — this mod gives each of the 15 races + 15 arenas its own color from `mknp_namestrip_colors.txt`. Config re-read ~every 2 seconds — edit at runtime, no restart needed.

## Installation (Hamsterball Plus required)

1. Place `mknp_namestrip_colors.dll` in the game's `Mods\` folder.
2. Launch the game. The mod auto-loads via HB+, installs its hook, and auto-generates `mknp_namestrip_colors.txt` next to the DLL.
3. Edit per-level RGBA values (see below).

## Configuration

Edit `mknp_namestrip_colors.txt` (next to the DLL in `Mods\`, NOT the game root):

```ini
Level 1 =
R = 1.0, G = 0.4, B = 0.2, A = 0.9

Arena 3 =
R = 0.2, G = 0.6, B = 1.0, A = 0.9
```

- `Level N` (1-15) = races, `Arena N` (1-15) = arenas. Values may sit on the header line or the next line.
- Floats 0.0-1.0. Levels missing from the file keep stock yellow (R=1.0 G=1.0 B=0.0 A=0.75).

## Technical Details

- **Hook site**: push sequence at `0x41B763` in `Scene_DrawNameStrip`, verified byte-for-byte before install (silent skip on mismatch — mod simply stays inactive).
- **Method**: 5-byte JMP to a `VirtualAlloc`'d cave holding 4 patchable `PUSH imm32` floats → `CALL Matrix_Scale4x4` → JMP back. Cave floats updated per-tick from the current scene name (`Scene.name` via HB+ `GetScene()`).
- **No threads** — cave install in `Initialize()`, updates in `onGameUpdate()`.

## Files

- `mknp_namestrip_colors.dll` — compiled HB+ mod
- `mknp_namestrip_colors.cpp` — source code (MinGW, nocrt, manual 17-entry vtable)
- `mknp_namestrip_colors.txt` — auto-generated config file (lives next to the DLL)

## Building

- **MinGW (Linux):** `cd source && ./build.sh` → `mknp_namestrip_colors.dll`

## Testing note

The HB+ loading chain (scanning `Mods\`, calling `CreateModInstance`, firing `Initialize()`) cannot be verified under Wine/hbtestd. Only real Windows with HB+ installed can confirm the full load + in-game behavior.

## Author

MAKYUNI / Hamsterbot
