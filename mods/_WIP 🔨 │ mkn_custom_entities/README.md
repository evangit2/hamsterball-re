# Custom Entities Mod v51

A bass.dll proxy mod for Hamsterball that spawns custom objects from MESHWORLD level files.

## How It Works

1. Place a `cEnt_XXX` entry in your level's MESHWORLD S1 ref points with an `<ENTITY>` tag
2. The mod reads the entity name and matches it against the AI list (hardcoded in the DLL)
3. The matching AI's `_ctor` function is called to spawn the object at the S1 ref point position
4. If no `<ENTITY>` tag is present, or the name doesn't match any AI, the object does not spawn

## Entity Format

In the level MESHWORLD S1 ref points:
```
cEnt_001 <ENTITY>Swirl</ENTITY>
```

- Entity names are case-insensitive
- If `<ENTITY>` is empty, missing, or doesn't match any AI in the list, the object will not spawn

## AI List

The AI list is hardcoded in the DLL, sorted alphabetically. Each AI maps an entity string to a native game constructor:

| Entity String | Constructor | Address | Mesh Path |
|---|---|---|---|
| 8ball | BadBall_ctor | 0x0040AFE0 | — |
| BBridge | BreakBridge_ctor | 0x00436D70 | — |
| Bell | Bell_ctor | 0x00434D70 | — |
| Blockdawg | Blockdawg_ctor | 0x0043C310 | levels\Level8-Blockdawg1 |
| Bonk | Bonk_ctor | 0x00438850 | — |
| Bridge | PopCylinder_ctor | 0x00436EE0 | levels\Level2-Bridge |
| Bumper | PopCylinder_ctor | 0x00436EE0 | — |
| Catapult | Catapult_ctor | 0x00437E10 | levels\Level4-Catapult |
| Chomper | PopCylinder_ctor | 0x00436EE0 | levels\Level4-Windmill |
| Chrome | PopCylinder_ctor | 0x00436EE0 | — |
| Drawbridge | PopCylinder_ctor | 0x00436EE0 | levels\Level4-Drawbridge |
| Fan | Fan_ctor | 0x00438C20 | — |
| Flag | FlagWaver_Ctor | 0x0046AF30 | — |
| Flickfloor1 | PopCylinder_ctor | 0x00436EE0 | levels\LevelDark-DFloor1 |
| Flickfloor2 | PopCylinder_ctor | 0x00436EE0 | levels\LevelDark-DFloor4 |
| Flickring | PopCylinder_ctor | 0x00436EE0 | levels\LevelDark-Flickring |
| Funball | PopCylinder_ctor | 0x00436EE0 | — |
| Gear | Gear_ctor | 0x00437590 | levels\LevelImpossible-Gear |
| Glassbreaker | PopCylinder_ctor | 0x00436EE0 | — |
| Gluebie | Gluebie_ctor | 0x00437CB0 | levels\Level3-Gluebie |
| Judge | PopCylinder_ctor | 0x00436EE0 | — |
| Lifter | Lifter_ctor | 0x00436920 | levels\Level6-Lifter |
| Looper | Looper_ctor | 0x00435800 | levels\LevelImpossible-Looper |
| Mace | Mace_ctor | 0x00438750 | levels\Level4-Mace |
| Mag | Magnifier_ctor | 0x00436250 | — |
| Mousetrap | MouseTrap_ctor | 0x00437880 | — |
| Neonplatform | NeonPlatform_ctor | 0x0043E110 | levels\LevelDark-NeonPlatform |
| Pendulum | Pendulum_ctor | 0x00437700 | levels\LevelImpossible-Pendulum |
| Popcylinder | PopCylinder_ctor | 0x00436EE0 | — |
| Rotator | Rotator_ctor | 0x004366F0 | levels\LevelImpossible-Rotator |
| Saw | Saw_ctor | 0x0043B780 | levels\Level8-Saw |
| Sawblade | SawBlade_ctor | 0x00434660 | — |
| Speedcylinder | SpeedCylinder_ctor | 0x00436A20 | — |
| Spinner | Spinner_Level_ctor | 0x004396F0 | — |
| Swirl | Rotator_ctor_Impossible | 0x00435940 | levels\Level3-Swirl |
| Tarbubble | PopCylinder_ctor | 0x00436EE0 | — |
| Tarpit | PopCylinder_ctor | 0x00436EE0 | — |
| Timebutton | TimeButton_ctor | 0x00436C10 | — |
| Tipper | Tipper_ctor | 0x00437960 | levels\Level3-Tipper |
| Trapdoor | Trapdoor_ctor | 0x00438290 | levels\Level4-Trapdoor1 |
| Trode | PopCylinder_ctor | 0x00436EE0 | levels\LevelDark-Trode |
| Waterwheel | PopCylinder_ctor | 0x00436EE0 | levels\Level3-WaterWheel |
| Wavy | Wavy_ctor | 0x0043AD40 | levels\Level7-Wavy1 |
| Wobbly | PopCylinder_ctor | 0x00436EE0 | levels\Level7-Wobbly1 |

## Usage Examples

### Spawn a SWIRL platform
```
cEnt_001 <ENTITY>Swirl</ENTITY>
```

### Spawn a Pendulum
```
cEnt_002 <ENTITY>Pendulum</ENTITY>
```

### Spawn a Mace
```
cEnt_003 <ENTITY>Mace</ENTITY>
```

## Installation

1. Backup your original `bass.dll`
2. Copy `bass.dll` from this mod to the game folder
3. Rename the original `bass.dll` to `bass_real.dll`

## Log File

The mod writes to `custom_entities.log` in the game directory for debugging.
