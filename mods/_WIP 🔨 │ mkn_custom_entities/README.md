# Custom Entities Mod v50

A bass.dll proxy mod for Hamsterball that spawns custom objects from MESHWORLD level files.

## How It Works

1. Place a `cEnt_XXX` entry in your level's MESHWORLD S1 ref points with an `<ENTITY>` tag
2. The mod reads the entity name and matches it against the AI list
3. The matching AI's `_ctor` function is called to spawn the object

## Entity Format

In the level MESHWORLD S1 ref points:
```
cEnt_001 <ENTITY>Swirl</ENTITY>
```

- If `<ENTITY>` is empty, missing, or doesn't match any AI in the list, the object **will not spawn**
- Entity names are case-insensitive

## AI List

The AI list is hardcoded in the DLL. Each AI maps an entity string to a native game constructor:

| Entity String | AI Type | Constructor | Mesh | Behavior |
|---|---|---|---|---|
| `Swirl` | 6 | Rotator_ctor_Impossible (0x435940) | `levels\Level3-Swirl` | Y-axis oscillating rotation (±2.0 rad) |
| `Pendulum` | 2 | Pendulum_ctor (0x437700) | `levels\LevelImpossible-Pendulum` | X-axis sine wave swing |

## Usage Examples

### Spawn a SWIRL platform
```
cEnt_001 <ENTITY>Swirl</ENTITY>
```

### Spawn a Pendulum
```
cEnt_002 <ENTITY>Pendulum</ENTITY>
```

## Installation

1. Backup your original `bass.dll`
2. Copy `bass.dll` from this mod to the game folder
3. Rename the original `bass.dll` to `bass_real.dll`

## Log File

The mod writes to `custom_entities.log` in the game directory for debugging.
