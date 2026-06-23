# Neon Race Objects: Complete Reverse Engineering

## Overview

Neon Race (Level7) contains 6 unique object types that can be spawned globally:
- **NEONPLATFORM** — Disappearing neon floor
- **DFLOOR1-4** — Four different disappearing floor sections
- **TRODE** — Neon electrode/tube objects

## Factory Function

**`Scene_SetupLevelDark`** @ `0x416270` (vtable[0x48] of Neon Board vtable @ `0x4D1DF0`)

This function:
1. Loads main level mesh `Levels\LevelDark` → stores at `Board+0x8AC`
2. Creates CollisionLevel from the main mesh → `Board+0x8B0`
3. Calls `Board_NeonRace_ctor` (0x424440) to init the Board
4. Calls vtable[0x80] (0x416910) = **CreateNeonObjects** factory

## Mesh Loading (Board_NeonRace_ctor @ 0x424440)

All 6 meshes are loaded into Board offsets:

| Board Offset | Mesh Path | String Address |
|---|---|---|
| +0x4374 | `Levels\LevelDark-NeonPlatform` | 0x4D1D9C |
| +0x4378 | `Levels\LevelDark-DFloor1` | 0x4D1D80 |
| +0x437C | `Levels\LevelDark-DFloor2` | 0x4D1D64 |
| +0x4380 | `Levels\LevelDark-DFloor3` | 0x4D1D48 |
| +0x4384 | `Levels\LevelDark-DFloor4` | 0x4D1D2C |
| +0x4388 | `Levels\LevelDark-Trode` | 0x4D1D14 |

**These meshes are NOT pre-loaded at App level.** They are only loaded by `Board_NeonRace_ctor`. For global spawning, they must be loaded via `MeshWorld_ctor` (0x461510).

## Object Creation (CreateNeonObjects factory @ 0x416910)

The factory iterates the level's MESHWORLD object list and matches object names via `strnicmp`:

### NEONPLATFORM (string at 0x4D00F0, length 12)
```
alloc = operator_new(0x10EC)
Stands_CtorRotator(alloc, Board, pos_X, pos_Y, pos_Z, mesh=Board+0x4374)
AthenaList_Append(Board+0x2578, obj)
sub-object = [obj+0x10D4]  ; CollisionLevel (created internally)
```

### DFLOOR1-4 (strings at 0x4D00E8/E0/D8/D0, length 7)
```
alloc = operator_new(0x1104)
DFloor_ctor(alloc, Board, pos_X, pos_Y, pos_Z, mesh=Board+0x4378/7C/80/84)
AthenaList_Append(Board+0x2578, obj)
sub-object = [obj+0x10E8]  ; CollisionLevel (created internally)
```

### TRODE (string at 0x4D00C8, length 5)
```
alloc = operator_new(0x1104)
DFloor_ctor(alloc, Board, pos_X, pos_Y, pos_Z, mesh=Board+0x4388)
AthenaList_Append(Board+0x2578, obj)
sub-object = [obj+0x10E8]  ; CollisionLevel (created internally)
```

## Constructors

### Stands_CtorRotator (NEONPLATFORM) @ 0x43E110
- **Calling convention:** `ecx=this`, ret 0x14 (5 stack params)
- **Params:** Board, pos_X, pos_Y, pos_Z, mesh
- **Vtable:** 0x4D5A10
- **Internal:** Calls Stands_ctor(0x462850) with mesh as source → creates CollisionLevel at obj+0x10D4 via CollisionLevel_ctorWithLevel(0x465080)
- **Position stored at:** obj+0x10D8/X/DC/E0

### DFloor/Trode ctor @ 0x43E450
- **Calling convention:** `ecx=this`, ret 0x14 (5 stack params)
- **Params:** Board, pos_X, pos_Y, pos_Z, mesh
- **Vtable:** 0x4D5A70
- **Internal:** Calls Stands_ctor(0x462850) with mesh as source → creates CollisionLevel at obj+0x10E8 via CollisionLevel_ctorWithLevel(0x465080)
- **Position stored at:** obj+0x10D0/D4/D8
- **Board stored at:** obj+0x10E4

## Collision System

### Physical Collision
Both constructors create a CollisionLevel sub-object internally via `CollisionLevel_ctorWithLevel` (0x465080). This gives the object its own spatial collision tree built from the mesh geometry. The ball will physically collide with the geometry through the generic spatial tree collision system.

### Neon-Specific Collision Events (Level_HandleCollision @ 0x416CA0)
The Neon Board vtable has a custom collision handler at vtable[0x74] = 0x416CA0. This handles:

| Event String | Address | Action |
|---|---|---|
| `N:NEONPLATFORM` | 0x4D0120 | Calls `[obj+0x47C]` then `NeonPlatform_Activate (0x437300)` |
| `E:ZOOP` | 0x4D0118 | Sets `obj+0x7F0 = 100` (timer), creates 3D sound at ball position |
| `E:LIGHTSOFF` | 0x4D010C | Sets `obj+0x7B4 = 100`, calls `vtable[4]`, removes object from list |
| `E:LIGHTSON` | 0x4D0100 | Sets `obj+0x7B8`, calls `vtable[4]`, decrements `Board+0x4390` |

**On non-Neon levels:** These events will NOT fire because the level's collision handler is different. Objects will have physical collision but no special behaviors (lights, zoop, disappear).

Finally calls `DispatchCollisionEvents` (0x40C5D0) at the end.

## Global Spawn Requirements

1. **Load meshes** via `MeshWorld_ctor(0x461510)` into Board+0x4374..+0x4388 (cached, load once)
2. **No AthenaList init needed** — Board+0x2578 is initialized by Board_ctor on all levels
3. **No mesh+0x878 fix needed** — Neon constructors call Stands_ctor directly, which does NOT read mesh+0x878 (unlike Glass_Level_ctor for Drawbridge)
4. **Constructor call:** `push mesh; sub esp,0xC; fill pos; push Board; mov ecx,alloc; call ctor; push obj; Append(Board+0x2578)`

## Addresses Summary

| Function | Address | Ret |
|---|---|---|
| MeshWorld_ctor | 0x461510 | 8 |
| operator_new | 0x4BA57B | cdecl |
| Stands_CtorRotator (NEONPLATFORM) | 0x43E110 | 0x14 |
| DFloor/Trode ctor | 0x43E450 | 0x14 |
| AthenaList_Append | 0x453810 | 4 |
| CreateNeonObjects factory | 0x416910 | 0x10 |
| Level_HandleCollision (Neon) | 0x416CA0 | 8 |
| Board_NeonRace_ctor | 0x424440 | 4 |
| Scene_SetupLevelDark | 0x416270 | — |
