# Hamsterball — Object Catalog for Modders

## What This Document Is

A comprehensive catalog of **game objects** found in `Hamsterball.exe` through Ghidra decompilation. Each object includes its constructor address, approximate size, vtable, key fields (where known), and how it fits into the game's object hierarchy.

Objects already documented in depth elsewhere are referenced rather than duplicated.

---

## Table of Contents

1. [Object Hierarchy Overview](#object-hierarchy-overview)
2. [Core Singletons](#core-singletons)
3. [Base Classes (Inheritance Root)](#base-classes)
4. [Player Objects](#player-objects)
5. [Level / Scene System](#level-scene-system)
6. [Level Objects — Mechanical](#level-objects-mechanical)
7. [Level Objects — Hazards](#level-objects-hazards)
8. [Level Objects — Environment](#level-objects-environment)
9. [UI / Menu System](#ui-menu-system)
10. [Graphics / Render Objects](#graphics-render-objects)
11. [Audio Objects](#audio-objects)
12. [Score / GameState Objects](#score-gamestate-objects)
13. [Utility / Container Objects](#utility-container-objects)
14. [Objects Not Yet Documented](#objects-not-yet-documented)
15. [How to Instantiate Objects at Runtime](#how-to-instantiate-objects-at-runtime)

---

## Object Hierarchy Overview

```
SceneObject (vtable 0x4D934C, size 0xD4)
  └─ Gadget (vtable 0x4D9170, size 0x870)
       ├─ Board (vtable 0x4D0260, size ~0x4368)
       │    └─ ArenaBoard (vtable 0x4D1358, size ~0x47D4)
       │    └─ GameLevel (vtable varies, size ~0x4368)
       │         ├─ Glass_Level
       │         ├─ Spinner_Level
       │         ├─ Gear_Level
       │         └─ BoardLevel3
       ├─ Platform (vtable varies, size 0x10FC)
       ├─ Stands (vtable varies, size 0x10D0)
       ├─ Secret (vtable varies, size 0x10EC)
       ├─ Looper (vtable varies, size 0x1500)
       ├─ Gear / BigGear (vtable varies, size 0x1514)
       ├─ Rotator (vtable varies, size 0x1508)
       ├─ Pendulum (vtable varies, size 0x1504)
       ├─ Tipper (vtable varies, size 0x1104)
       │    └─ TipperVisual (visual component)
       ├─ Bonk (vtable varies, size 0x1200)
       ├─ BreakBridge (vtable varies, size 0x1100)
       ├─ PopCylinder (vtable varies, size 0x10E8)
       ├─ Blockdawg (vtable varies, size 0x1154)
       ├─ Catapult (vtable varies, size 0x1108)
       ├─ Gluebie (vtable varies, size 0x110C)
       └─ Bumper / SpeedCylinder / Spinner / Sawblade / MouseTrap

Ball (vtable 0x4CF3A0, size 0xC98) — NOT in Gadget hierarchy
  └─ Ball_Split (vtable 0x4CF560)

Graphics (vtable 0x4D88A0, size ~0x540+)
MeshWorld (vtable 0x4D9CDC, size 0x488)
Level (vtable 0x4D8FB0, size 0x10D4)
CollisionLevel (vtable varies, size varies)
Scene (inherits Gadget, vtable 0x4D0260)

UI / Menu hierarchy:
  SimpleMenu
       ├─ MainMenu
       ├─ OptionsMenu
       ├─ PracticeMenu
       ├─ ConfirmMenu
       ├─ GameSelectionScreen
       ├─ HighScoreEntry
       ├─ RaceResultsMenu
       └─ SaveTourneyDialog
  UIListItem
  RaceResultPopup
  OkayDialog
  QuitDialog
  RegisterDialog
  ArenaScoreParticle
  Sprite
  ScoreObject
```

---

## Core Singletons

### App
- **Address:** `0x004FD680` (global pointer `g_App`)
- **Type:** `App*` — singleton holding all subsystem pointers
- **Size:** ~0xA00+ bytes
- **Vtable:** `0x004CE400`
- **Constructor:** `App_ctor` (0x46DC40)
- **Deep documentation:** See [`APP_OBJECT.md`](APP_OBJECT.md)
- **Key subsystems:** `graphics` (+0x174), `audioSystem` (+0x17C), `musicHandle` (+0x534), `registryKey` (+0x54), `gameMode1-5` (+0x550 to +0x55C)

### Graphics
- **Constructor:** `Graphics_ctor` (0x4542C0)
- **Vtable:** `0x4D88A0`
- **Size:** ~0x540+ bytes
- **Deep documentation:** See [`D3D8_RENDERING_PIPELINE.md`](D3D8_RENDERING_PIPELINE.md)
- **Key fields:** D3D device, texture cache, viewport, frustum, render mode

---

## Base Classes

These are **abstract base classes** you don't typically instantiate directly, but they define the field layout for all derived objects.

### SceneObject
- **Constructor:** `SceneObject_ctor` (0x46B4F0)
- **Vtable:** `0x4D934C` (destructor), `0x4D9170` (scalar dtor)
- **Size:** 0xD4 bytes (212 bytes)
- **Fields:**
  - `+0x00` vtable
  - `+0x08` app_ptr
  - `+0x14` visibility flag
  - `+0x18` field_18
  - `+0x1C` field_1c
  - `+0x20` field_20
  - `+0x24` field_24
  - `+0x28` field_28
  - `+0x2C` byte_2c
  - `+0x30` field_30
  - `+0x34` AthenaList (child objects)
  - `+0x44` AthenaList (another child list)
  - `+0x54` field_54
  - `+0x58` field_58
  - `+0x5C` field_5c
  - `+0x60` field_60
  - `+0x64` field_64
  - `+0x68` field_68
  - `+0x6C` field_6c
  - `+0x70` field_70
  - `+0x74` field_74
  - `+0x78` field_78
  - `+0x7C` field_7c
  - `+0x80` field_80
  - `+0x84` field_84
  - `+0x88` field_88
  - `+0x8C` field_8c
  - `+0x90` field_90
  - `+0x94` base_scale matrix (4x4)
  - `+0xA8` rotation matrix (4x4)
  - `+0xBC` world transform matrix (4x4)
  - `+0xD0` field_d0
- **Deep documentation:** See [`SCENE_STRUCT.md`](SCENE_STRUCT.md)

### Gadget
- **Constructor:** `Gadget_ctor` (0x4690F0)
- **Vtable:** `0x4D9170` (inherits from SceneObject)
- **Size:** 0x870 bytes
- **Parent of:** Board, Platform, Stands, Secret, all mechanical/hazard objects
- **Key fields:**
  - `+0x00` vtable → `0x4D9170`
  - `+0x014` app_ptr (App*)
  - `+0x018` second vtable → `0x004CF584`
  - `+0x034` AthenaList (children)
  - `+0x44C` AthenaList (siblings)
  - `+0x868` name string (e.g., `"Generic Gadget"`)
- **Deep documentation:** See [`SCENE_STRUCT.md`](SCENE_STRUCT.md)

### GameObject
- **Vtable:** `0x4CF314` (used as initial vtable before `Ball` is fully constructed)
- **Size:** ~0x810+ bytes
- **Parent of:** Ball (Ball is NOT in Gadget hierarchy — it's a separate branch)
- **Used by:** `Ball_ctor` (0x40AFE0) calls `GameObject_ctor` first

---

## Player Objects

### Ball
- **Constructor:** `Ball_ctor` (0x40AFE0), `Ball_ctor2` (0x4039E0)
- **Vtable:** `0x4CF3A0` (9 method pointers)
- **Size:** 0xC98 bytes (3224 bytes)
- **Deep documentation:** See [`BALL_OBJECT_MODDING.md`](BALL_OBJECT_MODDING.md)
- **Key fields:**
  - `+0x00` vtable
  - `+0x08` collision_result
  - `+0x0C` string_timer
  - `+0x10` app_state
  - `+0x14` scene ptr
  - `+0x18` player_index (-1 = AI)
  - `+0x164` pos_x/y/z
  - `+0x170` vel_x/y/z
  - `+0x188` max_speed (5000.0f)
  - `+0x18C` speed_scale (1.0f)
  - `+0x1A8` gravity vector
  - `+0x284` radius (27.0f)
  - `+0x2DC` lgp_x/y/z (Last Grounded Position)
  - `+0x768` cam_active
  - `+0xC4C` is_shrunk flag (Odd Race E:SHRINK/E:GROW)
  - `+0xC88` world transform matrix (4x4)
- **Methods (vtable slots):**
  - `[0x10]` Ball_Update (0x405E00) — main tick
  - `[0x14]` Ball_ApplyForceWithMultipliers (0x401590)
  - `[0x18]` Ball_Render (0x4027F0)
  - `[0x1C]` Ball_CollisionHandler (0x402DE0)
  - `[0x20]` Ball_InputHandler (0x402A70)
  - `[0x24]` Ball_PhysicsUpdate (0x405100)
  - `[0x28]` Ball_SpawnHandler (0x402C10)
  - `[0x2C]` Ball_AIUpdate (0x408390)
  - `[0x30]` Ball_SpecialUpdate (0x409480)

### Ball_Split
- **Constructor:** `Ball_Split_ctor` (0x408D10)
- **Vtable:** `0x4CF560`
- **Description:** Split-ball variant (multiplayer / tournament split-screen)

---

## Level / Scene System

### Scene / Board
- **Constructor:** `Board_ctor` (0x419020), `Scene` inherits from `Gadget`
- **Vtable:** `0x4D0260`
- **Size:** ~0x4368 bytes (Board), Scene is the same structure
- **Deep documentation:** See [`SCENE_STRUCT.md`](SCENE_STRUCT.md) and [`SCENE_OBJECT_MODDING.md`](SCENE_OBJECT_MODDING.md)
- **Key fields:**
  - `+0x000` Gadget base (0x870 bytes)
  - `+0x221` ArenaBoard timer data
  - `+0x335` Ball list (AthenaList)
  - `+0x43B` Effect list (AthenaList)
  - `+0x361C` Ball pointer (first player ball)
  - `+0x29B0` Gravity parameter
  - `+0x29C0` Camera orbit distance
  - `+0x3AAC` Player ball colors (4 RGB entries, 0x14 bytes each)

### Level
- **Constructor:** `Level_ctor` (0x461740)
- **Vtable:** `0x4D8FB0`
- **Size:** 0x10D4 bytes
- **Description:** Container for level geometry, objects, and state. Inherits from Gadget.
- **Fields:** 4 AthenaLists, Timer, LevelState

### GameLevel
- **Constructor:** `GameLevel_ctor` (0x4351F0)
- **Description:** Wraps Level with game-specific logic (Stands init, clone, sound channel)

### MeshWorld
- **Constructor:** `MeshWorld_ctor` (0x4706E0, 0x46F3D0)
- **Vtable:** `0x4D9CDC`
- **Size:** 0x488 bytes (small ctor), larger version handles strips
- **Description:** Level geometry parser and container. Loads `.meshworld` files.
- **Deep documentation:** See [`MESHWORLD_FORMAT.md`](MESHWORLD_FORMAT.md)

### CollisionLevel
- **Constructor:** `CollisionLevel_ctor` (0x4652E0)
- **Description:** Collision-only level data (`.meshcollision` format)

---

## Level Objects — Mechanical

### Platform
- **Constructor:** `Platform_ctor` (0x437040)
- **Size:** 0x10FC bytes
- **Description:** Static or animated platform. Most common level object.
- **Created by:** `CreatePlatformOrStands` factory

### Stands
- **Constructor:** `SceneObject_ctor` (0x462850)
- **Size:** 0x10D0 bytes
- **Description:** Audience / stadium stands geometry

### Looper
- **Constructor:** `Looper_ctor` (0x435800)
- **Size:** 0x1500 bytes
- **Description:** Loop-de-loop ramp structure

### Gear / BigGear
- **Constructor:** `Gear_ctor` (0x437590)
- **Size:** 0x1514 bytes
- **Description:** Rotating gear obstacles. Uses `N:BOUNCE` (bounce off gear surface), `N:ONGEAR` (attach to gear rotation via `Catapult_AddObjectConditional`), and `N:ONROTATOR` (attach via `Rotator_AddBall`). Found in `LevelImpossible-Gear.MESHWORLD` (8 N:BOUNCE triggers). Gear rotation handled by `Catapult_Update` (0x43E600) which applies rotation matrix to tracked balls each frame. See [Rotator System](../physics/COLLISION_SYSTEM_DEEP.md#rotator-system) for mechanics.

### Rotator
- **Constructor:** `Rotator_ctor` (0x435940)
- **Size:** 0x1508 bytes
- **Description:** Rotating platform / arm. Uses `N:ONROTATOR` event to attach balls via `Rotator_AddBall` (0x43B6F0). Ball position and velocity rotated each frame by `Catapult_Update` (0x43E600). 10-frame grace period after ball leaves surface before release.

### Pendulum
- **Constructor:** `Pendulum_ctor` (0x437700)
- **Size:** 0x1504 bytes
- **Description:** Swinging pendulum obstacle

### Tipper
- **Constructor:** `Tipper_ctor` (0x437960)
- **Size:** 0x1104 bytes
- **Description:** Tipping platform that tilts when ball rolls on it
- **Visual component:** `TipperVisual_ctor` (0x4661A0)

### Catapult
- **Constructor:** `Catapult_ctor` (0x437E10)
- **Size:** 0x1108 bytes
- **Description:** Spring-loaded launcher platform

### BreakBridge
- **Constructor:** `BreakBridge_ctor` (0x436D70)
- **Size:** 0x1100 bytes
- **Description:** Bridge that breaks when ball crosses

### PopCylinder
- **Constructor:** `PopCylinder_ctor` (0x436EE0)
- **Size:** 0x10E8 bytes
- **Description:** Cylinder that pops up from ground

---

## Level Objects — Hazards

### Bonk (Hammer)
- **Constructor:** `Bonk_ctor` (0x438850)
- **Size:** 0x1200 bytes
- **Description:** Giant swinging hammer obstacle
- **Popup feedback:** `CreateBonkPopup` (0x438B30)

### Blockdawg
- **Constructor:** `Blockdawg_ctor` (0x43C310)
- **Size:** 0x1154 bytes
- **Description:** Block creature / moving block hazard

### Gluebie
- **Constructor:** `Gluebie_ctor` (0x437CB0)
- **Size:** 0x110C bytes
- **Description:** Glue blob that slows the ball

### Spinner
- **Factory:** `HandleArenaCollisionEvents` (0x412850)
- **Description:** Spinning blade / propeller hazard

### Sawblade
- **Factory:** `CreateExpertLevelObjects` (0x40E250)
- **Description:** Rotating saw blade hazard

### MouseTrap
- **Factory:** `CreateMouseTrap` (0x40BF50)
- **Description:** Snap-shut trap hazard

### Bumper
- **Factory:** `CreateBumper` (0x40FA20), `CreateBumper2` (0x413CE0)
- **Description:** Pinball-style bumper that knocks ball away

### SpeedCylinder
- **Factory:** `CreateUpLevelObjects` (0x4117B0)
- **Description:** Cylinder that accelerates or decelerates ball

### NoDizzy
- **Factory:** `DispatchCollisionEvents` (0x40C5D0)
- **Description:** Grants dizzy immunity for TIME frames and calls Ball_DizzyImmunity — grants dizzy immunity

---

## Level Objects — Environment

### Secret
- **Constructor:** `Secret_ctor` (0x43DFB0)
- **Size:** 0x10EC bytes
- **Description:** Hidden collectible / secret area object
- **Factory:** `CreateSecretObjects` (0x40BAA0)

### Glass_Level
- **Constructor:** `Glass_Level_ctor` (0x4384A0)
- **Description:** Glass-themed level variant (transparent bridges, walls)

### Spinner_Level
- **Constructor:** `Spinner_Level_ctor` (0x4396F0)
- **Description:** Level variant with spinner obstacles

### Gear_Level
- **Constructor:** `Gear_Level_ctor` (0x43A150)
- **Description:** Level variant with gear obstacles

### BoardLevel3
- **Constructor:** `LevelBoard_Dizzy_ctor` (0x41D060)
- **Description:** Tournament level 3 board state

---

## UI / Menu System

### SimpleMenu (base)
- **Constructor:** `SimpleMenu_ctor` (0x448F20)
- **Description:** Base class for all menu screens. Handles item list, up/down scrollers, selection.

### MainMenu
- **Constructor:** `MainMenu_ctor` (0x42DE50)
- **Description:** Title screen — LET'S PLAY, HIGH SCORES, OPTIONS, CREDITS, EXIT

### OptionsMenu
- **Constructor:** `OptionsMenu_ctor` (0x442CE0)
- **Description:** Settings screen — Resolution, Fullscreen, Color depth, Safe Mode, Volume, Key Remap, Mouse, Pause

### PracticeMenu
- **Constructor:** `PracticeMenu_ctor` (0x42EA30)
- **Description:** "CHOOSE A PRACTICE RACE!" — 14 race items with thumbnails

### ConfirmMenu
- **Constructor:** `ConfirmMenu_ctor` (0x42B190)
- **Description:** BACK / BACK2TOURNAMENT / DONE confirmation dialog

### GameSelectionScreen
- **Constructor:** `GameSelectionScreen_ctor` (0x42E060)
- **Description:** Difficulty selector for tournament mode

### HighScoreEntry
- **Constructor:** `HighScoreEntry_ctor` (0x42B470)
- **Description:** Name input + score display after race

### RaceResultsMenu
- **Constructor:** `RaceResultsMenu_ctor` (0x44CB10)
- **Description:** Post-race screen with title, subtitle, player entries, timer

### RaceResults
- **Constructor:** `RaceResults_ctor` (0x44B8A0)
- **Description:** Race result data object (timers, congratulatory text, score thresholds)

### RaceResultPopup
- **Constructor:** `RaceResultPopup_ctor` (0x44C260)
- **Description:** Small popup showing rank + "TIME'S UP!" / "OUT OF TIME!" text

### SaveTourneyDialog
- **Constructor:** `SaveTourneyDialog_ctor` (0x44FD60)
- **Description:** "Save tournament progress?" dialog

### RegisterDialog
- **Constructor:** `RegisterDialog_ctor` (0x4476B0)
- **Description:** Purchase / registration reminder dialog

### OkayDialog
- **Constructor:** `OkayDialog_ctor` (0x440E70)
- **Description:** Simple "OKAY!" button dialog with caption

### QuitDialog
- **Constructor:** `QuitDialog_ctor` (0x443E30)
- **Description:** "YES" / "NO" quit confirmation dialog

### UIListItem
- **Constructor:** `UIListItem_ctor` (0x4490A0)
- **Size:** 0x444 bytes
- **Description:** Single selectable item in a menu list. Contains Vec3 + AthenaList.

---

## Graphics / Render Objects

### RenderContext
- **Constructor:** `RenderContext_Init` (0x457FA0)
- **Size:** 0x50 bytes
- **Vtable:** `0x4D8E68`
- **Description:** Per-object render state (material, transform, visibility)

### MeshBuffer
- **Factory:** `CreateMeshBuffer` (0x458970)
- **Description:** D3D8 vertex/index buffer wrapper

### Texture
- **Loader:** `Graphics_LoadTexture` (0x4542C0)
- **Constructor:** implicit via D3D8 `CreateTexture`
- **Description:** GPU texture object. Loaded from PNG/BMP files.

### Sprite
- **Constructor:** `Sprite_ctor` (0x45D0C0)
- **Description:** 2D screen-space sprite with texture + RenderContext + material defaults

### MeshNode
- **Constructor:** `MeshNode_ctor` (0x471C20)
- **Vtable:** `0x4D9C48`
- **Description:** Scene graph node that holds a loaded mesh file

### MeshArchive
- **Constructor:** `MeshArchive_ctor` (0x478E70)
- **Description:** Mesh resource pool. Manages D3D resources, mesh groups, position tracking.

---

## Audio Objects

### BASS Music
- **Handle:** `App.musicHandle` (+0x534) — `music\music.mo3`
- **Channels:** `musicChannel1` (+0x538), `musicChannel2` (+0x53C)
- **Loader:** `LoadMusicFile` (0x46A020)
- **Deep documentation:** See [`AUDIO_SYSTEM.md`](AUDIO_SYSTEM.md)

### SFX / 3D Sound
- **Handle:** `Ball.sound_3d_handle` (+0x700)
- **System:** DirectSound8 + BASS_SampleLoad
- **Deep documentation:** See [`AUDIO_SYSTEM_SFX.md`](AUDIO_SYSTEM_SFX.md)

---

## Score / GameState Objects

### ArenaScoreParticle
- **Constructor:** `ArenaScoreParticle_ctor` (0x44AD50)
- **Description:** Floating score popup in Rumble mode. Difficulty scale: 0.02/0.03/0.04.

### ScoreObject
- **Description:** SceneObject subclass. Despite the name, its `SetScore` method (now renamed `Rotator_AddBall` at 0x43B6F0) does NOT set a score — it registers balls on a rotator's tracking list for physical rotation. The `ScoreObject_ctor` (0x44BE80) creates a SceneObject with vtable `PTR_RaceGoalReached_Render` (0x4D6C70), used for race goal rendering.
- **Used by:** `Rotator_AddBall` called from `ImpossibleCollisionEvents` (N:ONROTATOR), `ToobCollisionEvents` (N:SPINNY), `DizzyArenaCollisionEvents` (N:SWIRL)

### ArenaBoard
- **Constructor:** `ArenaBoard_ctor` (0x4217B0)
- **Size:** ~0x47D4 bytes
- **Description:** Rumble mode game board. Inherits from Board. Timer-based scoring.
- **Embedded timer:** `ToggleTimer_Init` / `ToggleTimer_Cleanup`

---

## Utility / Container Objects

### AthenaList
- **Function:** `AthenaList_Append` (0x453780)
- **Description:** Dynamic array/list container used throughout the engine. Resizable, append-only.
- **Used by:** Almost every object for child/sibling tracking.

### AthenaHashTable
- **Constructor:** `AthenaHashTable_ctor` (0x472C20)
- **Vtable:** `0x4CF584`
- **Description:** Hash table for string→object lookups (e.g., mesh names)

### Vec3List
- **Description:** Array of `Vec3` structures. Used for position/path data.

### Timer
- **Function:** `Timer_Init` (various)
- **Description:** Countdown / animation timer used by objects for state transitions.

---

## Objects Not Yet Documented

These objects have constructors or factories in the binary but lack deep reverse-engineering documentation:

| Object | Constructor/Factory | Notes |
|--------|---------------------|-------|
| E:LIMIT boundary | `NeonCollisionEvents` (0x410D00) | Invisible race boundary |
| E:NODIZZY dizzy immunity zone | `DispatchCollisionEvents` (0x40C5D0) | Grants dizzy immunity for TIME frames |
| E:JUMP trigger | *(inline in collision)* | Launch pad trigger object |
| SAFESPOT | *(inline in factory)* | Safe zone / checkpoint |
| CAMERALOOKAT | `CameraLookAt` (0x413280) | Camera target marker |
| FLAG checkpoints | *(inline in factory)* | Race checkpoint objects |
| BRIDGE (tipper variant) | `CreateLevelObjects` (0x4121D0) | Breakable bridge sub-type |
| TIPPER (via factory) | `CreateLevelObjects` (0x4121D0) | Tipping platform sub-type |
| BBRIDGE1/2 | `CreateLevelObjects` (0x4121D0) | Bridge variants |
| CATAPULT (via factory) | `CreateLevelObjects` (0x4121D0) | Catapult sub-type |
| GLUEBIE (via factory) | `CreateLevelObjects` (0x4121D0) | Glue blob sub-type |
| BLOCKDAWG1/2 | `CreateLevelObjects` (0x4121D0) | Block creature variants |
| POPCYLINDER (via factory) | `CreateLevelObjects` (0x4121D0) | Pop-up cylinder sub-type |

---

## How to Instantiate Objects at Runtime

All level objects are created through the **object factory system** in `Scene/Level`. The typical pattern is:

```cpp
// Inside Scene::LoadLevel or Board::Init
void* obj = operator_new(0x1104);     // Allocate Tipper size
Tipper_ctor(obj, scene_ptr, param2);  // Call constructor
AthenaList_Append(scene->object_list, obj);  // Add to scene
```

### Factory Pattern

For MESHWORLD-defined objects, the factory is `CreateLevelObjects` (0x4121D0):

```cpp
// Pseudocode from decompilation
if (strcmp(type, "TIPPER") == 0) {
    obj = operator_new(0x1104);
    Tipper_ctor(obj, scene, params);
}
else if (strcmp(type, "BONK") == 0) {
    obj = operator_new(0x1200);
    Bonk_ctor(obj, scene, params);
}
// ... etc
```

### Modding Approach

To add custom objects:
1. Hook `CreateLevelObjects` (0x4121D0)
2. Add your own type string check
3. Allocate your object size
4. Call your custom constructor
5. Append to the scene's object list

See [`LEVEL_OBJECT_FACTORY.md`](LEVEL_OBJECT_FACTORY.md) for the full factory breakdown.

---

## Document Revision

- **Sources:** Ghidra decompilations (`analysis/ghidra/decompilations/`), `FUNCTION_MAP.md`, `renames_backup.json`, struct headers (`analysis/ghidra/structs/`)
- **Total objects cataloged:** 57 constructors + 14 factories + 6 base classes + 3 singletons
- **Coverage:** All major gameplay objects, UI screens, graphics/audio subsystems
- **Next additions:** Deeper field analysis for undocumented mechanical objects (TipperVisual, BreakBridge, PopCylinder internals), factory parameter formats

*For memory layouts of Ball, App, Scene, and Gadget, see the individual struct documents in `analysis/ghidra/structs/` and the markdown docs referenced above.*
