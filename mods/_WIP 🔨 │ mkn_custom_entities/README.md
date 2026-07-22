# Custom Entities Mod v53d

A bass.dll proxy mod for Hamsterball that spawns custom objects from MESHWORLD level files.

## v53d Changes

- Fixed 8ball/BadBall: now loads 8ball.MESH via MeshNode_ctor and stores at ball+0x10
- Fixed BadBall alloc size: 0xC98 (was 0xC70)
- Added Bonk_ctor (type 33, 0x438850, 0x1200) — Warm-Up Bonk, self-loads level5-bonk
- Bell/Fan/SawBlade: reverted to PopCylinder (type 0) — their ctors call Level_ctor (no mesh)
  and crash during board update because the vtable update method calls LoadMesh with invalid state

## v53c Changes

- Isolated and cloned behaviors: Chomper, Chrome, Funball, Tarbubble, Waterwheel
- Fixed Gear_ctor (9 params, was using 6-param Rotator typedef — would crash!)
- Fixed Looper size (0x1500, was 0x1508)
- Added Spinner_Level_ctor (Expert Race "BRIDGE")
- Added Cloudscape entity (Sky Race clouds, Sprite_ctor)
- Named _ctor wrappers for all Neon Race objects (DFloor1-4, FlickRing, Trode)

## v53b Changes

- **Deep Ghidra decompilation** of every `CreateDynamicObjects` function in the game
- Added 7 new constructor types with correct alloc sizes and calling conventions
- Fixed 10 entities that were using the wrong `_ctor`
- NULL mesh paths now use `levels\_default` as placeholder (your `_default.MESHWORLD` file)

### Entities fixed in v54

| Entity       | Old _ctor   | New _ctor            | Address  | Size   |
| ------------ | ----------- | -------------------- | -------- | ------ |
| Drawbridge   | PopCylinder | **Glass_Level_ctor** | 0x4384A0 | 0x113C |
| Flag         | PopCylinder | **FlagWaver_Ctor**   | 0x46AF30 | 0x8C   |
| Flickfloor1  | PopCylinder | **ArenaStands_ctor** | 0x43E450 | 0x1104 |
| Flickfloor2  | PopCylinder | **ArenaStands_ctor** | 0x43E450 | 0x1104 |
| Flickring    | PopCylinder | **ArenaStands_ctor** | 0x43E450 | 0x1104 |
| Glassbreaker | PopCylinder | **Secret_ctor**      | 0x43DFB0 | 0x10EC |
| Judge        | PopCylinder | **Gear_Level_ctor**  | 0x43A150 | 0x1100 |
| Sign         | PopCylinder | **Sign_ctor**        | 0x443B90 | 0x10FC |
| Trode        | PopCylinder | **ArenaStands_ctor** | 0x43E450 | 0x1104 |
| Wobbly       | PopCylinder | **GameLevel_ctor**   | 0x4351F0 | 0x1524 |

### Entities with no _ctor (PopCylinder is correct fallback)

- **Bridge** — Intermediate: returns pre-loaded mesh, no _ctor called
- **Chomper** — Tower: stores position at board+0x4394, no _ctor
- **Chrome** — Odd: CHROMESHADOW hash table lookup, no _ctor
- **Funball** — Sky: created by board update function, no _ctor
- **Tarbubble** — Dizzy: collected into board+0x11E4 list, no _ctor
- **Waterwheel** — Dizzy: stores position at board+0x4BB0, no _ctor
- **Windmill** — Tower: Level_RenderCtor + TipperVisual_Attach (Fan_ctor is NOT used)

### Collision tag entities (S6 tags, not spawned objects)

- **Bumper** → `levels\_default` (N:BUMPER%d is a meshbuffer collision tag)
- **Tarpit** → `levels\_default` (N:TARPIT is a meshbuffer collision tag)

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

## AI List (v53b)

### Constructor Types

| Type | _ctor                 | Address  | Size    | Signature                               |
| ---- | --------------------- | -------- | ------- | --------------------------------------- |
| 0    | PopCylinder_ctor      | 0x436EE0 | 0x10D0  | (this, board, x, y, z, mesh)            |
| 1    | Rotator_ctor          | 0x435940 | 0x1508  | (this, board, x, y, z, mesh)            |
| 2    | Pendulum_ctor         | 0x437700 | 0x1504  | (this, board, x, y, z, mesh)            |
| 3    | Looper_ctor           | 0x437460 | 0x1500  | (this, board, x, y, z, mesh)            |
| 4    | Gear_ctor             | 0x437690 | 0x1514  | (this, board, x, y, z, mesh)            |
| 5    | BigGear_ctor          | 0x437690 | 0x1514  | (same as 4)                             |
| 6    | Swirl                 | 0x435940 | 0x1508  | (Rotator, constant rotation)            |
| 7    | DFloor1_ctor           | 0x43E450 | 0x1104  | (this, board, x, y, z, mesh) — Neon DFLOOR1 |
| 17   | DFloor2_ctor           | 0x43E450 | 0x1104  | (this, board, x, y, z, mesh) — Neon DFLOOR2 |
| 18   | DFloor3_ctor           | 0x43E450 | 0x1104  | (this, board, x, y, z, mesh) — Neon DFLOOR3 |
| 19   | DFloor4_ctor           | 0x43E450 | 0x1104  | ArenaStands + post-config (obj+0x10DC=2, obj+0x10E0=0) |
| 20   | FlickRing_ctor         | 0x43E450 | 0x1104  | (this, board, x, y, z, mesh) — Neon Arena FLICKRING |
| 21   | Trode_ctor             | 0x43E450 | 0x1104  | (this, board, x, y, z, mesh) — Neon TRODE |
| 8    | GameLevel_ctor        | 0x4351F0 | 0x1524  | (this, board, x, y, z, mesh)            |
| 9    | Glass_Level_ctor      | 0x4384A0 | 0x113C  | (this, board, mesh) — no position!      |
| 10   | Gear_Level_ctor       | 0x43A150 | 0x1100  | (this, board, x, y, z) — no mesh!       |
| 11   | Secret_ctor           | 0x43DFB0 | 0x10EC  | (this, board, x, y, z, mesh)            |
| 12   | FlagWaver_Ctor        | 0x46AF30 | 0x8C    | (this, gfx_device) — code-gen mesh      |
| 13   | Sign_ctor             | 0x443B90 | 0x10FC  | (this, board, gfx1, gfx2, x, y, z, ...) |
| 14   | WavyFlag2 (Wavy_ctor) | 0x43AD40 | 0x1AE7C | (this, board, x, y, z, path_str)        |
| 15   | BadBall_ctor          | 0x40AFE0 | 0xC70   | (this, board) — no mesh, no position    |
| 16   | Bridgeslam            | Custom  | 0x10D0  | Isolated Intermediate bridge state machine |
| 22   | Chomper_ctor          | 0x471C20| 0x18    | MeshNode_ctor — Tower Chomper mesh |
| 23   | Chrome_ctor            | N/A     | N/A     | No _ctor, board-level behavior, PopCylinder fallback |
| 24   | Funball_ctor           | N/A     | N/A     | No _ctor, board-level behavior, PopCylinder fallback |
| 25   | Tarbubble_ctor         | N/A     | N/A     | No _ctor, board-level behavior, PopCylinder fallback |
| 26   | Waterwheel_ctor        | N/A     | N/A     | No _ctor, position-only storage, PopCylinder fallback |
| 27   | Spinner_Level_ctor    | 0x4396F0| 0x10FC  | Expert Race "BRIDGE" (6 params: this,board,x,y,z,mesh,float) |
| 28   | Cloudscape             | 0x45D0C0| 0xD4    | Sprite_ctor — Sky Race clouds, _default fallback |
| 29   | Gear_ctor              | 0x437690| 0x1514  | 9 params: (this,board,x,y,z,x2,y2,z2,mesh) |
| 30   | Bell_ctor              | 0x434D70| 0x10E8  | (this, board, x, y, z) — Level_ctor, no mesh |
| 31   | Fan_ctor               | 0x438C20| 0x1188  | (this, board, x, y, z, float) — Level_ctor, no mesh |
| 32   | SawBlade_ctor          | 0x434660| 0x111C  | (this, board, x, y, z) — Level_ctor, no mesh |
| 33   | Bonk_ctor              | 0x438850| 0x1200  | (this, board, x, y, z) — self-loads level5-bonk |

### Entity Table

| Entity        | Type | Mesh Path                       | Notes                                                 |
| ------------- | ---- | ------------------------------- | ----------------------------------------------------- |
| 8ball         | 15   | meshes\8ball                    | BadBall_ctor, 2 params: this+board                    |
| BBridge       | 0    | levels\Level10-Bridge1          | BreakBridge                                           |
| Bell          | 0    | meshes\bell                     | PopCylinder — Bell_ctor crashes (Level_ctor, no mesh)  |
| Blockdawg     | 0    | levels\Level8-BlockDawg1        | Blockdawg_ctor                                        |
| Bonk          | 33   | levels\Level5-Bonk              | Bonk_ctor (0x438850) — self-loads level5-bonk         |
| Bridge        | 16   | levels\Level2-Bridge            | Bridgeslam: isolated Intermediate bridge state machine |
| Bridgeslam    | 16   | levels\Level2-Bridge            | Alias for Bridge                                       |
| Bumper        | 0    | levels\_default                 | N:BUMPER tag, _default mesh                           |
| Catapult      | 0    | levels\Level4-Catapult          | Catapult_ctor                                         |
| Chomper       | 22   | meshes\chomper                  | Chomper_ctor (MeshNode_ctor, 0x471C20)                |
| Chrome        | 23   | levels\_default                 | Chrome_ctor: no _ctor, PopCylinder fallback           |
| Cloudscape    | 28   | levels\Cloudscape              | Sprite_ctor (0x45D0C0), _default fallback            |
| Drawbridge    | 9    | levels\Level4-Drawbridge        | Glass_Level_ctor                                      |
| Droplifter    | 0    | levels\Level6-Lifter            | Odd Race model                                        |
| Fan           | 0    | meshes\fanbody                  | PopCylinder — Fan_ctor crashes (Level_ctor, no mesh)   |
| Flag          | 12   | NULL                            | FlagWaver_Ctor, code-gen mesh                         |
| Flag2         | 14   | levels\Flag                     | WavyFlag2: Wavy_ctor copy, Flag.MESHWORLD or _default |
| Flickfloor1   | 7    | levels\LevelDark-DFloor1        | DFloor1_ctor (ArenaStands_ctor)                       |
| Flickfloor2   | 19   | levels\LevelDark-DFloor4        | DFloor4_ctor (ArenaStands + post-config)              |
| Flickring     | 20   | levels\LevelDark-Flickring      | FlickRing_ctor (ArenaStands_ctor)                     |
| Funball       | 24   | meshes\funball                  | Funball_ctor: no _ctor, PopCylinder fallback          |
| Gear          | 29   | levels\LevelImpossible-Gear     | Gear_ctor (0x437690, 9 params!)                       |
| Glassbreaker  | 11   | meshes\GlassBonus               | Secret_ctor                                           |
| Gluebie       | 0    | levels\Level3-Gluebie           | Gluebie_ctor                                          |
| Judge         | 10   | meshes\hammyjudge               | Gear_Level_ctor, no mesh param                        |
| Lifter        | 0    | levels\LevelUp-Lifter           | Up Race model                                         |
| Looper        | 3    | levels\LevelImpossible-Looper   | Looper_ctor (0x437460, 0x1500)                       |
| Mace          | 0    | levels\Level4-Mace              | Mace_ctor                                             |
| Mag           | 0    | meshes\magnifyingglass          | .MESH, Magnifier_ctor                                 |
| Mousetrap     | 0    | levels\MouseTrap                | MouseTrap_ctor                                        |
| Neonplatform  | 0    | levels\LevelDark-NeonPlatform   | NeonPlatform_ctor                                     |
| Pendulum      | 2    | levels\LevelImpossible-Pendulum | Pendulum_ctor                                         |
| Popcylinder   | 0    | levels\Level9-PopCylinder1      | PopCylinder_ctor                                      |
| Rotator       | 1    | levels\LevelImpossible-Rotator  | Rotator_ctor, constant rotation                       |
| Saw           | 0    | levels\Level8-Saw               | Saw_ctor                                              |
| Sawblade      | 0    | meshes\sawblade                 | PopCylinder — SawBlade_ctor crashes (Level_ctor, no mesh) |
| Sign          | 13   | levels\PopupSign                | Sign_ctor, complex signature                          |
| Speedcylinder | 0    | levels\LevelUp-SpeedCylinder    | SpeedCylinder_ctor                                    |
| Spinner       | 27   | levels\Level8-Spinny            | Spinner_Level_ctor (0x4396F0, 0x10FC)                |
| Swirl         | 6    | levels\Level3-Swirl             | Rotator_ctor_Impossible                               |
| Tarbubble     | 25   | meshes\tarbubble                | Tarbubble_ctor: no _ctor, PopCylinder fallback        |
| Tarpit        | 0    | levels\_default                 | N:TARPIT tag, _default mesh                           |
| Timebutton    | 0    | levels\LevelUp-Button           | TimeButton_ctor                                       |
| Tipper        | 0    | levels\Level3-Tipper            | Tipper_ctor                                           |
| Trapdoor      | 0    | levels\Level4-Trapdoor1         | Trapdoor_ctor                                         |
| Trode         | 21   | levels\LevelDark-Trode          | Trode_ctor (ArenaStands_ctor)                         |
| Waterwheel    | 26   | levels\Level3-WaterWheel        | Waterwheel_ctor: no _ctor, PopCylinder fallback       |
| Wavy          | 0    | levels\Level7-Wavy1             | Wavy_ctor                                             |
| Windmill      | 0    | levels\Level4-Windmill          | Tower: Level_RenderCtor + TipperVisual_Attach         |
| Wobbly        | 8    | levels\Level7-Wobbly1           | GameLevel_ctor                                        |

## Known Limitations

- **.MESH file entities** (8ball, Bell, Chomper, Fan, Funball, Judge, Mag, Sawblade, Tarbubble):
  These use a different mesh format. The mod tries loading via MeshWorld_ctor, falling back
  to Swirl if unsupported.
- **Gear/Looper**: PopCylinder fallback used because their native _ctors crashed
  (Gear at 0x478EDD, Looper at 0x468E91).
- **Static objects**: Many entities spawn with correct model but no animation/behavior.
  Adding native game behaviors requires calling per-level update functions.
- **Swirl on Dizzy/Arena**: Skipped to prevent duplication with natively-spawned SWIRL.
- **_default.MESHWORLD**: User-provided file used as placeholder for Bumper, Tarpit, Chrome.
  Must exist in `levels\` directory.

## Installation

1. Backup your original `bass.dll`
2. Copy `bass.dll` from this mod to the game folder
3. Rename the original `bass.dll` to `bass_real.dll`
4. Place `_default.MESHWORLD` in the `levels\` directory

## Log File

The mod writes to `custom_entities.log` in the game directory for debugging.
