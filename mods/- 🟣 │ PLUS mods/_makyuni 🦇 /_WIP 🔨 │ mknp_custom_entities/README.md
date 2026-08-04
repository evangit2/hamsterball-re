# Custom Entities Mod v55m_44u

A mknp_custom_entities.dll proxy mod for Hamsterball that spawns custom objects from MESHWORLD level files.

For version history, see [v_changelog.md](v_changelog.md).

## How It Works

1. Place a `cEnt_XXX` entry in your level's MESHWORLD S1 ref points with an `<ENTITY>` tag
2. The mod reads the entity name and matches it against the AI list (hardcoded in the DLL)
3. The matching AI's `cEnt_` constructor is called to spawn the object at the S1 ref point position
4. If no `<ENTITY>` tag is present, or the name doesn't match any AI, the object does not spawn

## Entity Format

In the level MESHWORLD S1 ref points:
```
cEnt_001 <ENTITY>Swirl</ENTITY>
```

- Entity names are case-insensitive
- If `<ENTITY>` is empty, missing, or doesn't match any AI in the list, the object will not spawn

## Constructor Types

| Type | cEnt_ Constructor       | Game _ctor         | Address  | Size    | Signature                                             |
| ---- | ----------------------- | ------------------ | -------- | ------- | ----------------------------------------------------- |
| 0    | cEnt_PopCylinder_ctor   | PopCylinder_ctor   | 0x436EE0 | 0x10D0  | (this, board, x, y, z, mesh)                          |
| 1    | cEnt_Rotator_ctor       | Rotator_ctor       | 0x435940 | 0x1508  | (this, board, x, y, z, mesh)                          |
| 2    | cEnt_Pendulum_ctor      | Pendulum_ctor      | 0x437700 | 0x1504  | (this, board, x, y, z, mesh)                          |
| 3    | cEnt_Looper_ctor        | Looper_ctor        | 0x437460 | 0x1500  | (this, board, x, y, z, mesh)                          |
| 4    | cEnt_Gear_ctor          | Gear_ctor          | 0x437690 | 0x1514  | (this, board, x, y, z, mesh)                          |
| 5    | cEnt_BigGear_ctor       | BigGear_ctor       | 0x437690 | 0x1514  | (same as type 4)                                      |
| 6    | cEnt_Swirl              | Rotator_ctor       | 0x435940 | 0x1508  | (Rotator, constant rotation)                          |
| 7    | cEnt_DFloor1_ctor       | ArenaStands_ctor   | 0x43E450 | 0x1104  | (this, board, x, y, z, mesh) - Neon DFLOOR1           |
| 8    | cEnt_GameLevel_ctor     | GameLevel_ctor     | 0x4351F0 | 0x1524  | (this, board, x, y, z, mesh)                          |
| 9    | cEnt_Glass_Level_ctor   | Glass_Level_ctor   | 0x4384A0 | 0x113C  | (this, board, mesh) - no position!                    |
| 10   | cEnt_Gear_Level_ctor    | Gear_Level_ctor    | 0x43A150 | 0x1100  | (this, board, x, y, z) - no mesh!                     |
| 11   | cEnt_Secret_ctor        | Secret_ctor        | 0x43DFB0 | 0x10EC  | (this, board, x, y, z, mesh)                          |
| 12   | cEnt_FlagWaver_Ctor     | FlagWaver_Ctor     | 0x46AF30 | 0x8C    | (this, gfx_device) - code-gen mesh                    |
| 13   | cEnt_Sign_ctor          | Sign_ctor          | 0x443B90 | 0x10FC  | (this, board, gfx1, gfx2, x, y, z, ...)               |
| 14   | cEnt_WavyFlag2          | Wavy_ctor          | 0x43AD40 | 0x1AE7C | (this, board, x, y, z, path_str)                      |
| 15   | cEnt_BadBall_ctor       | BadBall_ctor       | 0x40AFE0 | 0xC98   | (this, board) - no mesh, no position                  |
| 16   | cEnt_Bridgeslam         | Custom             | N/A      | 0x10D0  | Isolated Intermediate bridge state machine            |
| 17   | cEnt_DFloor2_ctor       | ArenaStands_ctor   | 0x43E450 | 0x1104  | (this, board, x, y, z, mesh) - Neon DFLOOR2           |
| 18   | cEnt_DFloor3_ctor       | ArenaStands_ctor   | 0x43E450 | 0x1104  | (this, board, x, y, z, mesh) - Neon DFLOOR3           |
| 19   | cEnt_DFloor4_ctor       | ArenaStands_ctor   | 0x43E450 | 0x1104  | ArenaStands + post-config (obj+0x10DC=2, +0x10E0=0)   |
| 20   | cEnt_FlickRing_ctor     | ArenaStands_ctor   | 0x43E450 | 0x1104  | (this, board, x, y, z, mesh) - Neon Arena FLICKRING   |
| 21   | cEnt_Trode_ctor         | ArenaStands_ctor   | 0x43E450 | 0x1104  | (this, board, x, y, z, mesh) - Neon TRODE             |
| 22   | cEnt_Chomper_ctor       | MeshNode_ctor      | 0x471C20 | 0x18    | MeshNode_ctor - Tower Chomper mesh                    |
| 23   | cEnt_Chrome_ctor        | N/A                | N/A      | N/A     | No _ctor, board-level behavior, PopCylinder fallback  |
| 24   | cEnt_Funball_ctor       | N/A                | N/A      | N/A     | No _ctor, board-level behavior, PopCylinder fallback  |
| 25   | cEnt_Tarbubble_ctor     | N/A                | N/A      | N/A     | No _ctor, board-level behavior, PopCylinder fallback  |
| 26   | cEnt_Waterwheel_ctor    | N/A                | N/A      | N/A     | No _ctor, position-only storage, PopCylinder fallback |
| 27   | cEnt_Spinner_Level_ctor | Spinner_Level_ctor | 0x4396F0 | 0x10FC  | Expert Race "BRIDGE" (6 params)                       |
| 28   | cEnt_Cloudscape         | Sprite_ctor        | 0x45D0C0 | 0xD4    | Sprite_ctor - Sky Race clouds, _default fallback      |
| 29   | cEnt_Gear_ctor_real     | Gear_ctor          | 0x437690 | 0x1514  | 9 params: (this,board,x,y,z,x2,y2,z2,mesh)            |
| 30   | cEnt_Bell_ctor          | Bell_ctor          | 0x434D70 | 0x10E8  | (this, board, x, y, z) - Level_ctor, no mesh          |
| 31   | cEnt_Fan_ctor           | Fan_ctor           | 0x438C20 | 0x1188  | (this, board, x, y, z, float) - Level_ctor, no mesh   |
| 32   | cEnt_SawBlade_ctor      | SawBlade_ctor      | 0x434660 | 0x111C  | (this, board, x, y, z) - Level_ctor, no mesh          |
| 33   | cEnt_Bonk_ctor          | Bonk_ctor          | 0x438850 | 0x1200  | (this, board, x, y, z) - self-loads level5-bonk       |

## Entity Table

| Entity        | Type | Mesh Path                       | Notes                                                       |
| ------------- | ---- | ------------------------------- | ----------------------------------------------------------- |
| 8ball         | 15   | meshes\8ball                    | cEnt_BadBall_ctor, 2 params: this+board                     |
| BBridge       | 0    | levels\Level10-Bridge1          | BreakBridge                                                 |
| Bell          | 0    | meshes\bell                     | PopCylinder - cEnt_Bell_ctor crashes (Level_ctor, no mesh)  |
| Blockdawg     | 0    | levels\Level8-BlockDawg1        | cEnt_Blockdawg_ctor                                         |
| Bonk          | 33   | levels\Level5-Bonk              | cEnt_Bonk_ctor (0x438850) - self-loads level5-bonk          |
| Bridge        | 16   | levels\Level2-Bridge            | cEnt_Bridgeslam: isolated Intermediate bridge state machine |
| Bridgeslam    | 16   | levels\Level2-Bridge            | Alias for Bridge                                            |
| Bumper        | 0    | levels\_default                 | N:BUMPER tag, _default mesh                                 |
| Catapult      | 35   | levels\Level4-Catapult          | cEnt_Catapult_ctor: Stands_ctor + Level_RenderCtor + per-frame vtable[11] + proximity trigger |
| Chomper       | 22   | meshes\chomper                  | cEnt_Chomper_ctor (cEnt_MeshNode_ctor, 0x471C20)            |
| Chrome        | 23   | levels\_default                 | cEnt_Chrome_ctor: no _ctor, PopCylinder fallback            |
| Cloudscape    | 28   | levels\Cloudscape               | cEnt_Cloudscape (cEnt_Sprite_ctor, 0x45D0C0)                |
| Drawbridge    | 9    | levels\Level4-Drawbridge        | cEnt_Glass_Level_ctor                                       |
| Droplifter    | 0    | levels\Level6-Lifter            | Odd Race model                                              |
| Fan           | 0    | meshes\fanbody                  | PopCylinder - cEnt_Fan_ctor crashes (Level_ctor, no mesh)   |
| Flag          | 12   | NULL                            | cEnt_FlagWaver_Ctor, code-gen mesh                          |
| Flag2         | 14   | levels\Flag                     | cEnt_WavyFlag2: Wavy_ctor copy, Flag.MESHWORLD or _default  |
| Flickfloor1   | 7    | levels\LevelDark-DFloor1        | cEnt_DFloor1_ctor (ArenaStands_ctor)                        |
| Flickfloor2   | 19   | levels\LevelDark-DFloor4        | cEnt_DFloor4_ctor (ArenaStands + post-config)               |
| Flickring     | 20   | levels\LevelDark-Flickring      | cEnt_FlickRing_ctor (ArenaStands_ctor)                      |
| Funball       | 24   | meshes\funball                  | cEnt_Funball_ctor: no _ctor, PopCylinder fallback           |
| Gear          | 29   | levels\LevelImpossible-Gear     | cEnt_Gear_ctor_real (0x437690, 9 params!)                   |
| Glassbreaker  | 11   | meshes\GlassBonus               | cEnt_Secret_ctor                                            |
| Gluebie       | 0    | levels\Level3-Gluebie           | cEnt_Gluebie_ctor                                           |
| Judge         | 10   | meshes\hammyjudge               | cEnt_Gear_Level_ctor, no mesh param                         |
| Lifter        | 0    | levels\LevelUp-Lifter           | Up Race model                                               |
| Looper        | 3    | levels\LevelImpossible-Looper   | cEnt_Looper_ctor (0x437460, 0x1500)                         |
| Mace          | 0    | levels\Level4-Mace              | cEnt_Mace_ctor                                              |
| Mag           | 0    | meshes\magnifyingglass          | .MESH, cEnt_Magnifier_ctor                                  |
| Mousetrap     | 0    | levels\MouseTrap                | cEnt_MouseTrap_ctor                                         |
| Neonplatform  | 0    | levels\LevelDark-NeonPlatform   | cEnt_NeonPlatform_ctor                                      |
| Pendulum      | 2    | levels\LevelImpossible-Pendulum | cEnt_Pendulum_ctor                                          |
| Popcylinder   | 0    | levels\Level9-PopCylinder1      | cEnt_PopCylinder_ctor                                       |
| Rotator       | 1    | levels\LevelImpossible-Rotator  | cEnt_Rotator_ctor, constant rotation                        |
| Saw           | 0    | levels\Level8-Saw               | cEnt_Saw_ctor                                               |
| Sawblade      | 0    | meshes\sawblade                 | PopCylinder - cEnt_SawBlade_ctor crashes (Level_ctor)       |
| Sign          | 13   | levels\PopupSign                | cEnt_Sign_ctor, complex signature                           |
| Speedcylinder | 0    | levels\LevelUp-SpeedCylinder    | cEnt_SpeedCylinder_ctor                                     |
| Spinner       | 27   | levels\Level8-Spinny            | cEnt_Spinner_Level_ctor (0x4396F0, 0x10FC)                  |
| Swirl         | 6    | levels\Level3-Swirl             | cEnt_Rotator_ctor_Impossible                                |
| Tarbubble     | 25   | meshes\tarbubble                | cEnt_Tarbubble_ctor: no _ctor, PopCylinder fallback         |
| Tarpit        | 0    | levels\_default                 | N:TARPIT tag, _default mesh                                 |
| Timebutton    | 0    | levels\LevelUp-Button           | cEnt_TimeButton_ctor                                        |
| Tipper        | 0    | levels\Level3-Tipper            | cEnt_Tipper_ctor                                            |
| Trapdoor      | 0    | levels\Level4-Trapdoor1         | cEnt_Trapdoor_ctor                                          |
| Trode         | 21   | levels\LevelDark-Trode          | cEnt_Trode_ctor (ArenaStands_ctor)                          |
| Waterwheel    | 26   | levels\Waterwheel (fallback levels\_default) | cEnt_Waterwheel_ctor: no _ctor, PopCylinder fallback        |
| Wavy          | 0    | levels\Level7-Wavy1             | cEnt_Wavy_ctor                                              |
| Windmill      | 0    | levels\Level4-Windmill          | Tower: Level_RenderCtor + TipperVisual_Attach               |
| Wobbly        | 8    | levels\Level7-Wobbly1           | cEnt_GameLevel_ctor                                         |

## Mod Functions (cEnt_ prefix)

All mod-side functions use the `cEnt_` prefix to distinguish them from game originals:

| Function                   | Purpose                                             |
| -------------------------- | --------------------------------------------------- |
| cEnt_spawn_rotater_at      | Main spawn function - dispatches to correct ctor    |
| cEnt_load_mesh_file        | Loads .MESH or .MESHWORLD files                     |
| cEnt_despawn_object        | Removes object from all board lists                 |
| cEnt_despawn_all_rotaters  | Despawn all tracked objects                         |
| cEnt_process_custom_tags   | Parses `<MESH>` and `<SPEEDMULT>` tags              |
| cEnt_apply_s1_rotater_tags | Applies ROT_Y/ROS_Y from S1 ref points              |
| cEnt_bridgeslam_update     | Per-frame state machine for Bridge entities         |
| cEnt_find_grid_points      | Scans S1 list for GRID reference points             |
| cEnt_get_level             | Gets Level pointer from board                       |
| cEnt_get_sceneobj          | Gets SceneObject pointer from board                 |
| cEnt_DFloor1_ctor          | Wrapper: calls ArenaStands_ctor for Neon DFLOOR1    |
| cEnt_DFloor2_ctor          | Wrapper: calls ArenaStands_ctor for Neon DFLOOR2    |
| cEnt_DFloor3_ctor          | Wrapper: calls ArenaStands_ctor for Neon DFLOOR3    |
| cEnt_DFloor4_ctor          | Wrapper: ArenaStands_ctor + post-config for DFLOOR4 |
| cEnt_FlickRing_ctor        | Wrapper: calls ArenaStands_ctor for FLICKRING       |
| cEnt_Trode_ctor            | Wrapper: calls ArenaStands_ctor for TRODE           |

## Entities with no _ctor (PopCylinder is correct fallback)

- **Bridge** - Intermediate: returns pre-loaded mesh, no _ctor called
- **Chomper** - Tower: stores position at board+0x4394, no _ctor
- **Chrome** - Odd: CHROMESHADOW hash table lookup, no _ctor
- **Funball** - Sky: created by board update function, no _ctor
- **Tarbubble** - Dizzy: collected into board+0x11E4 list, no _ctor
- **Waterwheel** - Dizzy: stores position at board+0x4BB0, no _ctor. v55m_44c: reverse rotation (+0.5°/frame) + WheelCreak sound via native channel (slot App+0x490 → Sound_GetChannel 0x459810 → Sound_Play3D per frame). v55m_44d: stops rotating + creaking while paused (board+0x874 gate). v55m_44i: collision object disabled (PopCylinder's CollisionLevel renders 0x7C component meshbuffers as if full-level → strip-array OOB crash at 0x46578C; native waterwheel collision comes from level geometry). v55m_44j: 44i was insufficient — PopCylinder_ctor itself creates the CollisionLevel and the game's 0x436FC0 registration never runs on the wheel, so the CollisionLevel stays alive; now render-neutralized (+0x430=1 skips the broken meshbuffer walk, component count zeroed, child CollisionLevels in +0x18 sub-lists neutralized) → crash 0x465789 gone. v55m_44k: 44j's shallow 2-level scan missed deeper tree nodes (each child copies +0x430 from its parent at build time) — now RECURSIVE full-tree walk + PER-FRAME re-neutralization in the polling thread (the game re-registers/re-builds nodes during FinishLoad after the spawn-time patch) → crash 0x46578C gone. v55m_44l: 44k's background-thread re-neutralization still lost the race — the 44k crash log shows 4 nodes neutralized yet the crash persisted at 0x465789 during Background/FinishLoad (nodes registered between the thread's 16ms ticks). Now ALSO re-neutralizes on the MAIN THREAD every frame via the Present hook (App_ResetFrame 0x46C200, before any render) AND covers board+0x8B0's CollisionLevel tree → the race is closed. v55m_44m: 44l's blanket board-walk neutralized the board's OWN level-geometry CollisionLevel tree (11 nodes → infinite fall + pale wheel) — now SELECTIVE: only neutralize board nodes that are the wheel's recorded nodes or probe-broken (component meshbuffer +0x418 strip pointer OOB); legit geometry left intact. **v55m_44n (FINAL): the whole PopCylinder approach was wrong — the native game NEVER wraps the waterwheel in a PopCylinder (it stores the loaded mesh at board+0x4374 and the board renders it directly). The crash was the mod's OWN PopCylinder→CollisionLevel. 44n removes PopCylinder entirely: the wheel is now a plain MeshWorld (vtable 0x4D8FB0, [18]=0x470150) hooked with the rotation matrix, registered in the render list + scene tree. NO CollisionLevel exists → the crash path (0x465650 walking component meshbuffer strip arrays) is structurally impossible. Exactly what MAKYUNI asked: "load the code that worked with Level3-WaterWheel, use my Waterwheel.MESHWORLD instead" — same loader (0x461510), same native-style wrapper, their file.**
- **Windmill** - Tower: Level_RenderCtor + TipperVisual_Attach (Fan_ctor is NOT used)

## Collision tag entities (S6 tags, not spawned objects)

- **Bumper** → `levels\_default` (N:BUMPER%d is a meshbuffer collision tag)
- **Tarpit** → `levels\_default` (N:TARPIT is a meshbuffer collision tag)

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

1. Backup your original `mknp_custom_entities.dll`
2. Copy `mknp_custom_entities.dll` from this mod to the game folder
3. Rename the original `mknp_custom_entities.dll` to `bass_real.dll`
4. Place `_default.MESHWORLD` in the `levels\` directory

## Log File

The mod writes to `custom_entities.log` in the game directory for debugging.
