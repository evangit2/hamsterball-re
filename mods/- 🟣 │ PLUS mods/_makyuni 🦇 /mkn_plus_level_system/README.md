# Level System (HB+ Mod)

Extended vtables with custom per-frame dispatch for all 15 levels.

## Installation

Place `mkn_plus_level_system.dll` in the `Mods\` folder.

## Features

- Patches ALL 15 level allocations to MAX_STRUCT_SIZE (0x6498)
- Creates extended vtables (128 entries per level)
- Slots 0-35: game-native dispatch (called by hardcoded game code)
- Slots 36-127: custom dispatch (called every frame via onGameUpdate)
- Config file: `mkn_level_functions.txt`

## Config Format

```
VTABLE 1 36 0x0041B130
VTABLE 1 37 MyCustomFunc
SWAPGEO WarmUp Beginner
```
