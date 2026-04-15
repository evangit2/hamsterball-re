# Level Object Factory - Complete Reference

## Overview
Hamsterball creates level objects through a layered factory system:
1. **CreateLevelObjects** (0x4121D0) - Main dispatcher, matches mesh names via strnicmp
2. **CreateMechanicalObjects** (0x417FE0) - Mechanical hazard factory
3. **CreateSawblade** (0x40E250) - Multi-factory for arena obstacles
4. **CreatePlatformOrStands** (0x4133E0) - Platform/stands factory (16 xrefs)
5. **Scene_CreateDynamicObjects** (0x40C430) - Dynamic object scanner

All created objects are appended to **Scene+0x2578** (AthenaList, the master object list).

## CreateLevelObjects Factory Map (0x4121D0)

| Mesh Name | Object Created | Struct Size | Scene Offset | Notes |
|-----------|---------------|-------------|-------------|-------|
| BRIDGE | Scene+0x436C (bridge mesh) | - | 0x437C | Checks (NOCOLLIDE) suffix; stores pos at +0x437C/80/84 |
| TIPPER | Tipper | 0x1104 | +0x2578 | + TipperVisual (0x10D0), attached via TipperVisual_Attach |
| BONK | Bonk | 0x1200 | +0x540C | Arena bonk popup; stored at Scene+0x540C |
| BBRIDGE1 | BreakBridge | 0x1100 | +0x5418 | Breakable bridge 1; Scene+0x5418 |
| BBRIDGE2 | BreakBridge | 0x1100 | +0x541C | Breakable bridge 2; Scene+0x541C |
| POPCYLINDER | PopCylinder | 0x10E8 | +0x5428 | Pop-up cylinder; also appended to Scene+0x5428 list |
| BLOCKDAWG1 | Blockdawg | 0x1154 | +0x2578 | Searches for "DAWGPATH1" nav path |
| BLOCKDAWG2 | Blockdawg | 0x1154 | +0x2578 | Searches for "DAWGPATH2" nav path; flag+0x1152=1 |
| CATAPULT | Catapult | 0x1108 | +0x584C | Scene+0x584C; flag+0x440=1 |
| GLUEBIE | Gluebie | 0x110C | +0x6080 | Sticky trap; appended to Scene+0x6080 list |

## Additional Level Object Factories

| Address | Factory | Creates | Notes |
|---------|---------|---------|-------|
| 0x40BCA0 | CreateBadBall | Enemy ball | Only in tournament/demo mode |
| 0x40BF50 | CreateMouseTrap | Mouse trap | Only in tournament/demo mode |
| 0x40BAA0 | CreateSecretObjects | Secret collectibles | Hidden items |
| 0x40C0F0 | Scene_CreateFlags | Finish flags | Race completion markers |
| 0x40C270 | Scene_CreateSigns | Direction signs | Arrow signposts |
| 0x40C430 | Scene_CreateDynamicObjects | Dynamic objects | Moving platforms, etc. |
| 0x40C5D0 | CreateNoDizzy | No-dizzy pickup | 27 xrefs — common collectible |
| 0x40E250 | CreateSawblade | BONK/TOWER/SAWBLADE/BRIDGE/JUDGE/BELL | Multi-factory for arena |
| 0x40FA20 | CreateBumper | Bumper | Standard collision bumper |
| 0x410D00 | CreateLimit | Level boundary | Invisible wall/limit |
| 0x4117B0 | CreateSpeedCylinder | Speed boost | Acceleration cylinder |
| 0x412850 | CreateSpinner | Spinning obstacle | Rotating hazard |
| 0x4133E0 | CreatePlatformOrStands | Platform/Stands | 16 xrefs - most common factory |
| 0x413CE0 | CreateBumper2 | Bumper variant | Second bumper type |
| 0x4143D0 | CreateSpinny | Spinny obstacle | Rotating hazard |
| 0x414A20 | CreateLifter | Lifter platform | Elevator/lift |
| 0x415460 | CreateWobbly1 | Wobbly bridge | Swaying bridge |
| 0x4173B0 | CreateFlickRing | Flick ring | Launch ring |
| 0x417FE0 | CreateMechanicalObjects | MECH objects | Mechanical hazard factory |
| 0x418760 | Scene_CreateObject_Gear | Gear object | Gear mechanism |
| 0x418870 | Scene_CreateObject4f | Generic object | 25 xrefs - versatile factory |
| 0x438B30 | CreateBonkPopup | Bonk popup | Score popup effect |

## CreateSawblade Arena Multi-Factory (0x40E250)

Called for arena levels. Creates different object types based on mesh name:

| Name Pattern | Object Type | Created By |
|---------------|-------------|------------|
| BONK_xxx | Bonk | Bonk_ctor |
| TOWER_xxx | Tower | Tower_ctor |
| SAWBLADE_xxx | Sawblade | Sawblade_ctor |
| BRIDGE_xxx | Bridge | Bridge_ctor (arena) |
| JUDGE_xxx | Judge | Judge_ctor |
| BELL_xxx | Bell | Bell_ctor |

## Scene Object List Offsets

| Offset | List | Description |
|--------|------|-------------|
| 0x2578 | AthenaList | Master object list (all objects) |
| 0x540C | Bonk* | Bonk popup pointer |
| 0x5410 | BreakBridge* | Bridge 1 mesh |
| 0x5414 | BreakBridge* | Bridge 2 mesh |
| 0x5418 | BreakBridge* | Active bridge 1 |
| 0x541C | BreakBridge* | Active bridge 2 |
| 0x5420 | PopCylinder* | Pop cylinder mesh |
| 0x5428 | AthenaList | Pop cylinder list |
| 0x5840 | Blockdawg* | Blockdawg1 path |
| 0x5844 | Blockdawg* | Blockdawg2 path |
| 0x584C | AthenaList | Catapult list |
| 0x607C | Gluebie* | Gluebie mesh |
| 0x6080 | AthenaList | Gluebie list |
| 0x878 | App* | Application pointer |
| 0x8AC | LevelMesh* | Level mesh data |
| 0x8AC | LevelMesh | Mesh object for FindObjectByName |

## Object Creation Pattern

All object factories follow this pattern:
```c
void* obj = operator_new(STRUCT_SIZE);  // Allocate
Object_ctor(obj, scene, mesh, ...);      // Initialize
obj->position = mesh_position;           // Copy position from MESHWORLD data
AthenaList_Append(&scene->objects, obj); // Add to master list
scene->specific_list = obj;              // Store in type-specific pointer
```