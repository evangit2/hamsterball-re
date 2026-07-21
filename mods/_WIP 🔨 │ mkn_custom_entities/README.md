# Custom Entities Mod v52

A bass.dll proxy mod for Hamsterball that spawns custom objects from MESHWORLD level files.

## v52 Changes

- Fixed 20+ wrong/NULL mesh paths that caused Swirl fallback for many entities
- Added new AIs: **Sign**, **Windmill**, **Droplifter**
- **Lifter** now uses Up Race model (`levels\LevelUp-Lifter`)
- **Droplifter** (new) uses Odd Race model (`levels\Level6-Lifter`)
- **Rotator** now uses constant rotation (ROS_Y=0) instead of oscillation
- **Gear** and **Looper** AI types changed to PopCylinder (0) to prevent crashes
  - Gear crashed at 0x478EDD (MeshArchive_ctor)
  - Looper crashed at 0x468E91
- Fixed Swirl duplication on Dizzy Race and Arena levels
  - Swirl entities are now skipped on levels that natively have SWIRL
- .MESH file entities (8ball, Bell, Chomper, Fan, etc.) now try loading via
  MeshWorld_ctor with `meshes\` path, falling back to Swirl if unsupported

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

## AI List (v52)

| Entity String | AI Type | Mesh Path | Notes |
|---|---|---|---|
| 8ball | PopCylinder | meshes\8ball | .MESH file (Swirl fallback) |
| BBridge | PopCylinder | levels\Level10-Bridge1 | Fixed (was NULL) |
| Bell | PopCylinder | meshes\bell | .MESH file (Swirl fallback) |
| Blockdawg | PopCylinder | levels\Level8-BlockDawg1 | Correct |
| Bonk | PopCylinder | levels\Level5-Bonk | Fixed (was NULL) |
| Bridge | PopCylinder | levels\Level2-Bridge | Correct |
| Bumper | PopCylinder | levels\Level9-PopCylinder1 | Fixed (was NULL, no _default) |
| Catapult | PopCylinder | levels\Level4-Catapult | Correct |
| Chomper | PopCylinder | meshes\chomper | Fixed (was Windmill, now .MESH) |
| Chrome | PopCylinder | levels\Level9-PopCylinder1 | Fixed (was NULL) |
| Drawbridge | PopCylinder | levels\Level4-Drawbridge | Correct |
| Droplifter | PopCylinder | levels\Level6-Lifter | **NEW** — Odd Race model |
| Fan | PopCylinder | meshes\fanbody | Fixed (was NULL, now .MESH) |
| Flag | PopCylinder | levels\Level9-PopCylinder1 | Fixed (was NULL) |
| Flickfloor1 | PopCylinder | levels\LevelDark-DFloor1 | Correct |
| Flickfloor2 | PopCylinder | levels\LevelDark-DFloor4 | Correct |
| Flickring | PopCylinder | levels\LevelDark-FlickRing | Fixed (case) |
| Funball | PopCylinder | meshes\funball | Fixed (was NULL, now .MESH) |
| Gear | PopCylinder | levels\LevelImpossible-Gear | **Fixed crash** (was AI 4) |
| Glassbreaker | PopCylinder | levels\LevelGlass | Fixed (was NULL) |
| Gluebie | PopCylinder | levels\Level3-Gluebie | Correct |
| Judge | PopCylinder | meshes\hammyjudge | Fixed (was NULL, now .MESH) |
| Lifter | PopCylinder | levels\LevelUp-Lifter | **Fixed** (was Level6=Odd, now Up) |
| Looper | PopCylinder | levels\LevelImpossible-Looper | **Fixed crash** (was AI 3) |
| Mace | PopCylinder | levels\Level4-Mace | Correct |
| Mag | PopCylinder | meshes\magnifyingglass | Fixed (was NULL, now .MESH) |
| Mousetrap | PopCylinder | levels\MouseTrap | Fixed (was NULL) |
| Neonplatform | PopCylinder | levels\LevelDark-NeonPlatform | Correct |
| Pendulum | Pendulum_ctor | levels\LevelImpossible-Pendulum | Correct |
| Popcylinder | PopCylinder | levels\Level9-PopCylinder1 | Fixed (was NULL) |
| Rotator | Rotator_ctor | levels\LevelImpossible-Rotator | **Fixed** (constant rotation) |
| Saw | PopCylinder | levels\Level8-Saw | Correct |
| Sawblade | PopCylinder | meshes\sawblade | Fixed (was NULL, now .MESH) |
| Sign | PopCylinder | levels\PopupSign | **NEW** |
| Speedcylinder | PopCylinder | levels\LevelUp-SpeedCylinder | Fixed (was NULL) |
| Spinner | PopCylinder | levels\Level8-Spinny | Fixed (was NULL) |
| Swirl | Rotator_ctor_Impossible | levels\Level3-Swirl | Correct, no duplicate on Dizzy/Arena |
| Tarbubble | PopCylinder | meshes\tarbubble | Fixed (was NULL, now .MESH) |
| Tarpit | PopCylinder | levels\Level9-PopCylinder1 | Fixed (was NULL, no _default) |
| Timebutton | PopCylinder | levels\LevelUp-Button | Fixed (was NULL) |
| Tipper | PopCylinder | levels\Level3-Tipper | Correct |
| Trapdoor | PopCylinder | levels\Level4-Trapdoor1 | Correct |
| Trode | PopCylinder | levels\LevelDark-Trode | Correct |
| Waterwheel | PopCylinder | levels\Level3-WaterWheel | Correct |
| Wavy | PopCylinder | levels\Level7-Wavy1 | Correct |
| Windmill | PopCylinder | levels\Level4-Windmill | **NEW** |
| Wobbly | PopCylinder | levels\Level7-Wobbly1 | Correct |

## Known Limitations

- **.MESH file entities** (8ball, Bell, Chomper, Fan, Funball, Judge, Mag, Sawblade, Tarbubble):
  These use a different mesh format (.MESH) that `MeshWorld_ctor` cannot load. The mod tries
  loading them anyway, but if it fails, it falls back to the Swirl mesh. To get the correct
  model, a future version needs to use `MeshNode_ctor` (0x471C20) for .MESH files.
- **Static objects**: Many entities (Tipper, Bridge, Gluebie, etc.) spawn with correct model
  but no animation/behavior. Adding native game behaviors requires calling the correct
  per-level update functions, which is future work.
- **Swirl on Dizzy/Arena**: Swirl entities are skipped on Dizzy Race, Master Race, and all
  Arena levels to prevent duplication with natively-spawned SWIRL objects.

## Installation

1. Backup your original `bass.dll`
2. Copy `bass.dll` from this mod to the game folder
3. Rename the original `bass.dll` to `bass_real.dll`

## Log File

The mod writes to `custom_entities.log` in the game directory for debugging.
