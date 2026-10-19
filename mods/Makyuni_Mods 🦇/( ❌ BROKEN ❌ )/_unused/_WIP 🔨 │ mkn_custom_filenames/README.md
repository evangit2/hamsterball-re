# mkn_custom_filenames — Custom Race & Arena Level Loader

Customize which MESHWORLD file is loaded for each race and arena slot, and rearrange the tournament/arena order. Edit a simple text file — no code changes needed.

## How It Works

The game has 30 `PUSH imm32` instructions (0x68 opcode) that push level path strings like `"levels\level1"` or `"levels\arena-WarmUp"` onto the stack before calling `LoadMeshWorld`. This mod:

1. Reads `mkn_custom_filenames.txt` from the game directory at startup
2. Allocates new strings in DLL memory (e.g. `"levels\my_custom_level"`)
3. Overwrites the 4-byte string pointer in each PUSH instruction to point to the new string

## Installation

1. Rename the original `bass.dll` to `bass_real.dll`
2. Copy this mod's `mkn_custom_filenames.dll` to the game folder as `bass.dll` (if the txt is missing, the mod creates a default one next to the DLL on first launch)
3. Launch the game

## Uninstallation

1. Delete `bass.dll`
2. Rename `bass_real.dll` back to `bass.dll`

## Config File Format

Edit `mkn_custom_filenames.txt`:

```
TOURNAMENT_RACES:
1=level1
2=levelcascade
3=level2
...
15=levelimpossible

PRACTICE_RACES:
1=level1
2=levelcascade
3=level2
...
15=levelimpossible

ARENAS:
1=arena-WarmUp
2=arena-beginner
...
15=arena-impossible
```

### Renaming a level
Change the filename to load a different MESHWORLD file:
```
1=my_custom_level
```
This makes the game load `levels\my_custom_level.meshworld` for race slot 1.

### Rearranging order
Swap entries to change the tournament progression:
```
1=level3
2=level1
3=levelcascade
```
Now Dizzy Race (level3) is the first race, Warm-up (level1) is second, Beginner (levelcascade) is third.

### Default Level Mapping

| Slot | Tournament (default file) | Practice (default file) | Arena (default file) |
|------|---------------------------|-------------------------|----------------------|
| 1 | Warm-up (`level1`) | Warm-up (`level1`) | Warm-up (`arena-WarmUp`) |
| 2 | Beginner (`levelcascade`) | Beginner (`levelcascade`) | Beginner (`arena-beginner`) |
| 3 | Intermediate (`level2`) | Intermediate (`level2`) | Intermediate (`arena-intermediate`) |
| 4 | Dizzy (`level3`) | Dizzy (`level3`) | Dizzy (`arena-dizzy`) |
| 5 | Tower (`level4`) | Tower (`level4`) | Tower (`arena-tower`) |
| 6 | Up (`levelup`) | Up (`levelup`) | Up (`arena-up`) |
| 7 | Neon (`leveldark`) | Neon (`leveldark`) | Neon (`arena-neon`) |
| 8 | Expert (`level5`) | Expert (`level5`) | Expert (`arena-expert`) |
| 9 | Odd (`level6`) | Odd (`level6`) | Odd (`arena-Odd`) |
| 10 | Toob (`level8`) | Toob (`level8`) | Toob (`arena-Toob`) |
| 11 | Wobbly (`level7`) | Wobbly (`level7`) | Wobbly (`arena-Wobbly`) |
| 12 | Glass (`levelglass`) | Glass (`levelglass`) | Glass (`arena-glass`) |
| 13 | Sky (`level9`) | Sky (`level9`) | Sky (`arena-Sky`) |
| 14 | Master (`level10`) | Master (`level10`) | Master (`arena-Master`) |
| 15 | Impossible (`levelimpossible`) | Impossible (`levelimpossible`) | Impossible (`arena-impossible`) |

### Important Notes

- **Delete `.cached` files**: If `.cached` files exist in the `Levels/` directory, the game loads those instead of `.MESHWORLD` files. Delete them to force re-reading.
- **Comment lines**: Lines starting with `#` are ignored
- **Missing entries**: If a slot is missing from the config, the default level is used
- **Case sensitivity**: The game's file system may be case-sensitive. Use the exact filenames from the defaults.
- **No `.meshworld` extension**: Don't include the `.meshworld` extension in the config — the game adds it automatically.

## Technical Details

- **Mod type**: BASS.dll proxy (lazy loader pattern)
- **Patch count**: 30 PUSH instruction immediate overwrites (15 tournament races + 15 practice races + 15 arenas — but note: tournament and practice share the same 15 level files, so 30 total patches)
- **Config parser**: Simple line-based text parser with `TOURNAMENT_RACES:` / `PRACTICE_RACES:` / `ARENAS:` sections
- **Memory**: Custom strings allocated in DLL's static memory (persists for game lifetime)
- **Logging**: Writes `mkn_custom_filenames_log.txt` to the game directory on load
- **Crash test**: Passed — game runs 87+ seconds on Wine/Xvfb with all 30 patches applied

### Patch Addresses (RVA)

Each patch overwrites the 4-byte string pointer at `PUSH_addr + 1`:

| Slot | Race RVA | Arena RVA |
|------|----------|-----------|
| 1 | 0x0d202 | 0x13c62 |
| 2 | 0x11115 | 0x13d25 |
| 3 | 0x0d2c2 | 0x141c2 |
| 4 | 0x0d3d4 | 0x14282 |
| 5 | 0x0d712 | 0x144f2 |
| 6 | 0x1158b | 0x149a2 |
| 7 | 0x162b5 | 0x16f83 |
| 8 | 0x0e1d2 | 0x14b52 |
| 9 | 0x0ead3 | 0x14d22 |
| 10 | 0x0fa65 | 0x14f45 |
| 11 | 0x0f3a2 | 0x153e2 |
| 12 | 0x17683 | 0x17e32 |
| 13 | 0x10874 | 0x15904 |
| 14 | 0x11fa2 | 0x160c2 |
| 15 | 0x17f62 | 0x18582 |
