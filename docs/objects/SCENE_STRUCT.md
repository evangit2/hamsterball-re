# Scene Struct Deep Dive

## Overview
The Scene struct is the central game object that holds all runtime state for a level.
It's created during level loading and contains references to all objects, physics state,
camera configuration, input handling, and scene graph data.

## Scene Struct Layout (partial, from decompilation)

| Offset | Type | Name | Description |
|--------|------|------|-------------|
| +0x00 | vtable* | vtable | Scene vtable (0x4D0260) |
| +0x04 | App* | app | Application pointer |
| +0x08 | Scene* | scene | Self-reference (circular) |
| +0x0C-0x14 | fields | - | Reserved / flags |
| +0x18 | AthenaList | gadget_list | Main gadget list |
| +0x1C | int | gadget_count | Number of gadgets |
| +0x20-0x28 | int[] | gadget_iters | Iterator indices |
| +0x42C | AthenaList | - | Secondary list |
| +0x424 | AthenaList* | data_ptr | Data array pointer |
| +0x540 | void* | bonk_popup | Bonk popup object pointer |
| +0x5410 | void* | bridge1_mesh | BreakBridge 1 mesh |
| +0x5414 | void* | bridge2_mesh | BreakBridge 2 mesh |
| +0x5418 | BreakBridge* | bridge1 | Active BreakBridge 1 |
| +0x541C | BreakBridge* | bridge2 | Active BreakBridge 2 |
| +0x5420 | void* | popcyl_mesh | PopCylinder mesh |
| +0x5428 | AthenaList | popcyl_list | PopCylinder list |
| +0x5840 | void* | blockdawg1_path | "DAWGPATH1" path |
| +0x5844 | void* | blockdawg2_path | "DAWGPATH2" path |
| +0x584C | AthenaList | catapult_list | Catapult list |
| +0x607C | void* | gluebie_mesh | Gluebie mesh |
| +0x6080 | AthenaList | gluebie_list | Gluebie list |
| +0x858 | AthenaList | objects | All scene objects |
| +0x864 | void* | active_input | Currently active input gadget |
| +0x870 | int | input_state | Input state flags |
| +0x878 | App* | app_ptr | Application pointer (alternate) |
| +0x87C | SoundChannel* | sound | Level sound channel |
| +0x8AC | LevelMesh* | level_mesh | Level mesh data |
| +0x8B4 | int | object_count | Incremented on spawn |
| +0x2578 | AthenaList | all_objects | Master object list (all types) |
| +0x29BC | float | orbit_angle | Camera orbit angle |
| +0x29C0 | float | orbit_distance | Camera orbit distance |
| +0x3F1C | int | path_active | Camera path rail flag |
| +0x3F20 | Path* | path_obj | Camera path object |
| +0x3F24 | float | path_t | Path parameter (0-1) |
| +0x3F28 | int | snap_flag | Camera snap flag |
| +0x3F34 | Vec3 | snap_position | Camera snap target |
| +0x3F38 | int | snap_flag2 | Alternate snap flag |
| +0x3F2C | int | snap_countdown | Camera snap countdown (frames) |
| +0x434C | Vec3 | cam_offset | Camera offset from ball |
| +0x43AC | Vec3 | cam_initial | Camera initial position (from CAMERALOOKAT) |
| +0x43BC | Vec3 | cam_target | Camera target position (from CAMERALOOKAT) |
| +0x43C0 | float | cam_height | Camera height (800 default) |
| +0x43C4 | float | cam_distance | Camera distance (800 default) |
| +0x43C8 | float | cam_max_height | Camera max height |
| +0x43CC | int | cam_mode | Camera mode (1=orbit) |
| +0x4344 | char* | music_path | Level music path |
| +0x436C | void* | bridge_mesh | Bridge mesh reference |
| +0x4370 | int | bridge_collision | Bridge collision flag |
| +0x437C | Vec3 | bridge_pos | Bridge position |
| +0x4394 | void* | tipper_mesh | Tipper mesh |
| +0x4398 | void* | tipper_visual | TipperVisual mesh |
| +0x710 | void*[8] | render_passes | 8 render pass slots |

## Key Scene Functions

| Address | Function | Description |
|---------|----------|-------------|
| 0x419FA0 | Scene_SetCamera | 5-mode camera system |
| 0x41C5B0 | Scene_SpawnBallsAndObjects | Ball creation + object spawning |
| 0x40ACA0 | Level_SelectCameraProfile | Camera profile per level |
| 0x45E0E0 | Scene_RenderAllObjects | Render all scene objects |
| 0x4692F0 | Scene_HandleInput | Dispatch input to gadgets |

## Scene Creation Flow

```
1. LoadRaceData (0x40A120)
   └─> MeshWorld_ctor: Parse MESHWORLD file
   └─> Level_InitScene (0x40B090)
       ├─ Create SoundChannel (0x80 bytes)
       ├─ Set volume (-50 dB)
       ├─ Level mesh reset
       ├─ Set random ambient colors
       ├─ Find "CAMERALOCUS" / "CAMERALOOKAT"
       ├─ Set camera path (orbit mode)
       ├─ Level_SelectCameraProfile
       └─ Play level music (2x speed / 4x intro)

2. Scene_SpawnBallsAndObjects (0x41C5B0)
   ├─ Create Ball(s) with start positions
   ├─ Scan for SAFESPOT/SAFEPOS objects
   ├─ CreateBadBall (tournament only)
   ├─ CreateMouseTrap (tournament only)
   ├─ CreateSecretObjects
   ├─ Scene_CreateFlags (goal markers)
   ├─ Scene_CreateSigns (direction arrows)
   └─ Scene_CreateDynamicObjects

3. CreateLevelObjects (0x4121D0)
   ├─ BRIDGE → bridge mesh reference
   ├─ TIPPER → Tipper object + TipperVisual
   ├─ BONK → Bonk popup
   ├─ BBRIDGE1/2 → BreakBridge
   ├─ POPCYLINDER → PopCylinder
   ├─ BLOCKDAWG1/2 → Blockdawg (with path)
   ├─ CATAPULT → Catapult
   └─ GLUEBIE → Gluebie sticky trap
```