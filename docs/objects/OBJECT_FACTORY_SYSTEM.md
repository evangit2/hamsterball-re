# Hamsterball Game Object Factory System

## Overview

The factory system creates all game objects from MESHWORLD level data. Each
level file contains named objects (N: and E: prefixed). The factories match
object name strings and instantiate the appropriate C++ object.

## Factory Dispatch Chain

```
Level data → Named objects parsed from MESHWORLD
  ├─ CreateLevelObjects (0x4121D0) — main dispatcher
  │    ├─ BRIDGE, TIPPER, BONK, BBRIDGE1/2, POPCYLINDER
  │    ├─ BLOCKDAWG1/2, CATAPULT, GLUEBIE
  │    └─ Falls through to CreatePlatformOrStands
  ├─ CreatePlatformOrStands (0x4133E0)
  │    ├─ PLATFORM → Platform_ctor (0x371040, 0x10FC bytes)
  │    └─ STANDS → Stands_ctor (0x462850, 0x10D0 bytes)
  ├─ CreateSpinny (0x4143D0) — ROTATOR → Rotator_ctor
  ├─ CreateLifter (0x414A20) — LIFTER → Lifter_ctor + falls through
  ├─ CreateWobbly1 (0x415460) — WOBBLY1-8 → GameLevel_ctor + falls through
  ├─ CreateMechanicalObjects (0x417FE0) — LOOPER, GEAR, BIGGEAR, ROTATOR, PENDULUM
  ├─ CreateBumper (0x40FA20) — BUMPER1-4 → MeshWorld + 8x Scene_CollectByNameFilter
  ├─ CreateSawblade (0x40E250) — Multi-factory for arenas
  │    ├─ BONK → Bonk_ctor (0x1200 bytes)
  │    ├─ "UP?" → TowerLevel_Ctor (0x1188 bytes)
  │    ├─ SAWBLADE1/2 → Sawblade_Level_Ctor (0x111C bytes)
  │    ├─ SPINNER/BRIDGE → Spinner_Level_ctor (0x10FC bytes)
  │    ├─ JUDGE → Gear_Level_ctor (0x1100 bytes)
  │    └─ BELL → Tipper_Level_Ctor (0x10E8 bytes)
  ├─ CreateBadBall (0x40BCA0) — BADBALL → Ball_ctor
  ├─ CreateSecretObjects (0x40BAA0) — SECRET / SECRETUNLOCK
  ├─ CreateMouseTrap (0x40BF50) — MOUSETRAP → MouseTrap_ctor
  ├─ CreateLimit (0x410D00) — E:LIMIT → Level collision
  └─ CreateSpeedCylinder (0x4117B0) — SPEEDCYLINDER
```

## CreateLevelObjects (0x4121D0) — Detailed

Main factory, called per named object in the level mesh. Uses `__strnicmp` for
prefix matching. All created objects are appended to `Scene+0x2578`.

### Object Types

| Object Name | Match Length | Object Size | Constructor | Scene Storage |
|-------------|-------------|-------------|-------------|---------------|
| BRIDGE | 6 | — | Configures mesh | +0x436C (mesh), +0x4370 (collision) |
| TIPPER | 6 | 0x1104 | Tipper_ctor | +0x2578 |
| BONK | 4 | 0x1200 | Bonk_ctor | +0x2578, +0x540C |
| BBRIDGE1 | 8 | 0x1100 | BreakBridge_ctor | +0x2578, +0x5418 |
| BBRIDGE2 | 8 | 0x1100 | BreakBridge_ctor | +0x2578, +0x541C |
| POPCYLINDER | 11 | 0x10E8 | PopCylinder_ctor | +0x2578, +0x5428 |
| BLOCKDAWG1 | 10 | 0x1154 | Blockdawg_ctor | +0x2578 |
| BLOCKDAWG2 | 10 | 0x1154 | Blockdawg_ctor (flag+0x1152=1) | +0x2578 |
| CATAPULT | 8 | 0x1108 | Catapult_ctor | +0x2578, +0x584C |
| GLUEBIE | 7 | 0x110C | Gluebie_ctor | +0x2578, +0x6080 |

### Gating
- TIPPER, BONK, BLOCKDAWG1/2, GLUEBIE are gated behind `App+0x23C != 0`
  (multiplayer/arena mode flag)
- BRIDGE with "(NOCOLLIDE)" in name skips collision mesh setup

### Object Position Source
All objects read position from `param_4` (transform matrix):
- `param_4+0x04`: X position
- `param_4+0x08`: Y position
- `param_4+0x0C`: Z position
- `param_4+0x10`: X rotation
- `param_4+0x14`: Y rotation (also used for scale in some objects)
- `param_4+0x18`: Z rotation

## CreateSawblade (0x40E250) — Arena Multi-Factory

This is actually a combined factory that handles **6 different object types**
for arena levels. All created objects get multiplayer gating (`App+0x23C`).

### Object Types

| Object Name | Match | Object Size | Constructor | Scene Storage |
|-------------|-------|-------------|-------------|---------------|
| BONK | 4 | 0x1200 | Bonk_ctor | +0x2578, +0x436C |
| "UP?" | 3 | 0x1188 | TowerLevel_Ctor | +0x2578 |
| SAWBLADE | 8 | 0x111C | Sawblade_Level_Ctor | +0x2578, +0x4370 or +0x4374 |
| SPINNER/BRIDGE | 6 | 0x10FC | Spinner_Level_ctor | +0x2578, +0x4380 or +0x4798 |
| JUDGE | 5 | 0x1100 | Gear_Level_ctor | +0x2578, +0x4BBC |
| BELL | 4 | 0x10E8 | Tipper_Level_Ctor | +0x2578, +0x4FD4 |

### Special Flag Handling
- **SLOW** in name: `obj+0x43B = 1` (TowerLevel slow mode)
- **SUPER** in name: `obj+0x10ED = 1` (TowerLevel super mode)
- **UP** in name: `Sound_InitChannels(obj, 1)` (TowerLevel with sound)
- **SAWBLADE1/2**: stored at Scene+0x4370/0x4374, `Sawblade_SetBreakSound(n)`
- **SPINNER1/2**: stored in list at Scene+0x4380/0x4798
- **NEG** in SPINNER name: `obj+0x43E = -1.0` (0xBF800000, reverse rotation)
- **JUDGE**: added to judge list at Scene+0x4BBC
- **BELL**: stored as single reference at Scene+0x4FD4

## CreateBumper (0x40FA20)

Creates a dedicated bumper level with 8 named bumper objects:
1. `new MeshWorld(0x10D0, graphics, "levels\\level8")`
2. `new CollisionLevel(0x10D0, meshWorld)` — clone for collision
3. `Level_InitScene(this)`
4. 8 iterations: `Scene_CollectByNameFilter("N:BUMPER%d", base+0x10E3)`
5. `vtable[0x80]()` — post-init callback

Stored: `this+0x22B` = MeshWorld, `this+0x22C` = CollisionLevel

## Scene Object List Offsets

| Offset | Type | Description |
|--------|------|-------------|
| +0x2578 | AthenaList | All active game objects |
| +0x436C | void* | Bonk/Hammer reference |
| +0x4370 | void* | Sawblade 1 |
| +0x4374 | void* | Sawblade 2 |
| +0x4378 | AthenaList | (level list 1) |
| +0x4380 | AthenaList | Spinner list 1 |
| +0x4790 | AthenaList | (level list 2) |
| +0x4798 | AthenaList | Spinner list 2 |
| +0x4BBC | AthenaList | Judge (Gear) list |
| +0x4FD4 | void* | Bell reference |
| +0x540C | void* | Bonk reference (TowerLevel) |
| +0x5410 | void* | BreakBridge mesh 1 |
| +0x5418 | void* | BreakBridge object 1 |
| +0x5414 | void* | BreakBridge mesh 2 |
| +0x541C | void* | BreakBridge object 2 |
| +0x5420 | void* | PopCylinder mesh |
| +0x5428 | AthenaList | PopCylinder list |
| +0x5840 | void* | Blockdawg1 mesh |
| +0x5844 | void* | Blockdawg2 mesh |
| +0x5848 | void* | Catapult mesh |
| +0x584C | AthenaList | Catapult list |
| +0x607C | void* | Gluebie mesh |
| +0x6080 | AthenaList | Gluebie list |

## Scene Spawn Flow (0x41C5B0)

`Scene_SpawnBallsAndObjects` creates player balls and game objects for a race:

1. Create GameObject (0xC60 bytes) for each ball
2. Set start positions via "START%d-%d" naming pattern
3. Scan "SAFESPOT"/"SAFEPOS" objects for safe landing zones
4. Create BadBall/MouseTrap/SecretObjects/Flags/Signs
5. Create dynamic objects (spinning platforms, traps)

## CreateMechanicalObjects (0x417FE0)

| Object | Constructor | Size | Notes |
|--------|-------------|------|-------|
| LOOPER | Looper_ctor | 0x1500 | Looping animated platform |
| GEAR | Gear_ctor | 0x1514 | Spinning gear (with path) |
| BIGGEAR | Gear_ctor | 0x1514 | Larger gear variant |
| ROTATOR | Rotator_ctor | 0x1504 | Rotating platform |
| PENDULUM | Pendulum_ctor | 0x1504 | Swinging pendulum |

## Board Construction Pattern

Every Board (race level) follows the same construction pattern in BoardLevelX_ctor:

1. `Board_ctor(this, app)` — base class, vtable 0x4D0260
2. Set level-specific vtable (e.g., 0x4D0890 for Dizzy)
3. Init AthenaLists at +0x4378 and +0x4790
4. Set title string and race name
5. Load sub-levels as MeshWorld + CollisionLevel pairs
6. Allocate sound channel
7. Set physics: gravity vector, Matrix_Identity
8. `LoadRaceData(this, "RACENAME")`
9. Set display string