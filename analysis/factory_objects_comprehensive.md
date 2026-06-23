# Hamsterball Factory Object System — Comprehensive Analysis

## Overview

The Hamsterball object factory system uses a dispatch pattern: each Board level loads
MeshWorld sub-meshes during construction (in the `BoardLevel*_ctor`), storing MeshWorld
pointers at specific Board offsets. Then, a per-level `Create*Objects` factory function is
called for each named object in the MESHWORLD file, using `__strnicmp` to match object
name prefixes and instantiate the appropriate game object with position/rotation data from
a `param_block`.

### Key Constants
- **Image base**: 0x00400000
- **Board (this/ecx) + 0x878** = App pointer
- **App + 0x23C** = difficulty enum (0 = easy, non-zero = hard)
- **Board + 0x2578** = general active objects list (AthenaList)
- **param_block layout**: Position X/Y/Z at +4/+8/+0xC, Rotation X/Y/Z at +0x10/+0x14/+0x18
- **MeshWorld allocation size**: 0x10D0 bytes
- **MeshNode allocation size**: 0x18 bytes
- **CollisionLevel allocation size**: 0x10D0 bytes

---

## 1. Factory Function: CreateLevelObjects (0x004121D0)

Used by: Dizzy Race (Level3), Master Race, and other levels using Level3-style objects.

| Object Name | Match | Alloc Size | Constructor | Board Mesh Offset | Board Store Offset | AthenaList(s) | Difficulty Gate | Position Fields | Rotation Fields |
|---|---|---|---|---|---|---|---|---|---|
| BRIDGE | strnicmp "BRIDGE", 6 | N/A (no alloc) | None — configures existing mesh | +0x436C | +0x437C, +0x4380, +0x4384 (pos) | — | No | param_4+4/+8/+0xC | — |
| TIPPER | strnicmp "TIPPER", 6 | 0x1104 | Tipper_ctor | +0x4394 (mesh), +0x4398 (visual mesh) | — | +0x2578 | YES (App+0x23C != 0) | obj+0x10D8/+0x10DC/+0x10E0 | obj+0x10E4/+0x10E8/+0x10EC |
| TipperVisual | (sub-alloc) | 0x10D0 | TipperVisual_ctor | +0x4398 | obj+0x10D4 | — | — | — | — |
| BONK | strnicmp "BONK", 4 | 0x1200 | Bonk_ctor | — | +0x540C | +0x2578 | YES (App+0x23C != 0) | param_4+4/+8/+0xC | — |
| BBRIDGE1 | strnicmp "BBRIDGE1", 8 | 0x1100 | BreakBridge_ctor | +0x5410 | +0x5418 | +0x2578 | No | param_4+4/+8/+0xC | — |
| BBRIDGE2 | strnicmp "BBRIDGE2", 8 | 0x1100 | BreakBridge_ctor | +0x5414 | +0x541C | +0x2578 | No | param_4+4/+8/+0xC | — |
| POPCYLINDER | strnicmp "POPCYLINDER", 11 | 0x10E8 | PopCylinder_ctor | +0x5420 | — | +0x2578, +0x5428 | No | param_4+4/+8/+0xC | — |
| BLOCKDAWG1 | strnicmp "BLOCKDAWG1", 10 | 0x1154 | Blockdawg_ctor | +0x5840 | — | +0x2578 | YES (App+0x23C != 0) | param_4+4/+8/+0xC | — |
| BLOCKDAWG2 | strnicmp "BLOCKDAWG2", 10 | 0x1154 | Blockdawg_ctor | +0x5844 | — (sets obj+0x1152=1) | +0x2578 | YES (App+0x23C != 0) | param_4+4/+8/+0xC | — |
| CATAPULT | strnicmp "CATAPULT", 8 | 0x1108 | Catapult_ctor | +0x5848 | — (sets obj+0x1100=1) | +0x2578, +0x584C | No | obj+0x10D8/+0x10DC/+0x10E0 | — |
| GLUEBIE | strnicmp "GLUEBIE", 7 | 0x110C | Gluebie_ctor | +0x607C | — | +0x6080, +0x2578 | YES (App+0x23C != 0, inverted: skips if 0) | obj+0x10D4/+0x10D8/+0x10DC | — |

### BLOCKDAWG Path Lookups
- BLOCKDAWG1: `Level_FindObjectByName(Board+0x8AC, "DAWGPATH1")` → path index passed to ctor
- BLOCKDAWG2: `Level_FindObjectByName(Board+0x8AC, "DAWGPATH2")` → path index passed to ctor

---

## 2. Factory Function: CreateUpLevelObjects (0x004117B0)

Used by: Up Race (LevelUp).

| Object Name | Match | Alloc Size | Constructor | Board Mesh Offset | AthenaList(s) | Difficulty Gate | Position Fields | Notes |
|---|---|---|---|---|---|---|---|---|
| LIFTER | strnicmp "LIFTER", 6 | 0x10F4 | Rotator_ctor_sound | +0x4784 | +0x2578 | No | param_4+4/+8/+0xC | Parses _atol(param_1+6) for numeric arg |
| SPEEDCYLINDER | strnicmp "SPEEDCYLINDER", 13 | 0x150C | Pendulum_ctor | +0x4788 | +0x2578 | No | param_4+4/+8/+0xC | Uses __ftol2 for float→int conversion |
| TIMEBUTTON | strnicmp "TIMEBUTTON", 10 | 0x10E8 | Rotator_ctor_nosound | +0x478C | +0x2578 | No | param_4+4/+8/+0xC | Returns obj via param_2/param_3 |

---

## 3. Factory Function: CreateMechanicalObjects (0x00417FE0)

Used by: Impossible Race and other mechanical levels.

| Object Name | Match | Alloc Size | Constructor | Board Mesh Offset | AthenaList(s) | Difficulty Gate | Position Fields | Rotation Fields | Notes |
|---|---|---|---|---|---|---|---|---|---|
| LOOPER | strnicmp "LOOPER", 6 | 0x1500 | Looper_ctor | +0x436C | +0x2578 | No | param_4+4/+8/+0xC | — | — |
| GEAR | strnicmp "GEAR", 4 | 0x1514 | Gear_ctor | +0x4370 | +0x2578 | No | param_4+4/+8/+0xC (pos1) + param_4+0x10/+0x14/+0x18 (pos2) | Both position AND rotation from param_block | — |
| BIGGEAR | strnicmp "BIGGEAR", 7 | 0x1514 | Gear_ctor | +0x4374 | +0x2578 | No | param_4+4/+8/+0xC (pos1) + param_4+0x10/+0x14/+0x18 (pos2) | Sets obj+0x10F4=0x3F000000 (0.5f scale). Checks strstr("TOUCH")→obj+0x1510=1 | — |
| ROTATOR | strnicmp "ROTATOR", 7 | 0x1508 | Rotator_ctor | +0x4378 | +0x2578 | No | param_4+4/+8/+0xC | Sets obj+0x10E8=±1.0f (RNG_Rand direction). obj+0x10D4 = result | — |
| PENDULUM | strnicmp "PENDULUM", 8 | 0x1504 | Pendulum_ctor | +0x437C | +0x2578 | No | param_4+4/+8/+0xC | — | Returns obj via param_2/param_3 |

---

## 4. Factory Function: CreateExpertLevelObjects (0x0040E250)

Used by: Expert Race (Level5).

| Object Name | Match | Alloc Size | Constructor | Board Mesh Offset | Board Store Offset | AthenaList(s) | Difficulty Gate | Position Fields | Notes |
|---|---|---|---|---|---|---|---|---|---|
| BONK | strnicmp "BONK", 4 | 0x1200 | Bonk_ctor | — | +0x436C | +0x2578 | YES (App+0x23C != 0) | param_4+4/+8/+0xC | — |
| FAN | strnicmp "FAN", 3 (via PTR_DAT_0x4CFA48) | 0x1188 | TowerLevel_Ctor | — | — | +0x2578 | YES (App+0x23C != 0) | param_4+4/+8/+0xC + param_4+0x14 (extra float) | Checks strstr("SLOW")→obj+0x10EC=1, strstr("SUPER")→obj+0x10ED=1, strstr("UP")→Sound_InitChannels |
| SAWBLADE | strnicmp "SAWBLADE", 8 | 0x111C | Sawblade_Level_Ctor | — | — | +0x2578 | YES (App+0x23C != 0) | param_4+4/+8/+0xC | strstr("1")→Board+0x4370=Sawblade, Sawblade_SetBreakSound(1). strstr("2")→Board+0x4374=Sawblade, Sawblade_SetBreakSound(2) |
| BRIDGE (Expert) | strnicmp "BRIDGE", 6 | 0x10FC | Spinner_Level_ctor | — | — | +0x2578 (conditional) | No | param_4+4/+8/+0xC + param_4+0x14 | strstr("1")→AthenaList+0x4380. strstr("2")→AthenaList+0x4798. strstr("NEG")→obj+0x10F8=0xBF800000 (-1.0f) |
| JUDGE | strnicmp "JUDGE", 5 | 0x1100 | Gear_Level_ctor | — | — | +0x4BBC | No | param_4+4/+8/+0xC | — |
| BELL | strnicmp "BELL", 4 | 0x10E8 | Tipper_Level_Ctor | — | +0x4FD4 | +0x2578 | No | param_4+4/+8/+0xC | — |

---

## 5. Factory Function: CreateSpinny (0x004143D0)

Used by: Toob Race (Level8), Master Race, and general-purpose spinny levels.

| Object Name | Match | Alloc Size | Constructor | Board Mesh Offset | AthenaList(s) | Difficulty Gate | Position Fields | Notes |
|---|---|---|---|---|---|---|---|---|
| SPINNY | strnicmp "SPINNY", 6 | 0x1508 | Rotator_ctor | +0x47E0 | +0x2578 | No | param_4+4/+8/+0xC | Falls through to CreatePlatformOrStands if no match |

---

## 6. Factory Function: CreateLifter (0x00414A20)

Used by: Lifter levels (Level6, Impossible).

| Object Name | Match | Alloc Size | Constructor | Board Mesh Offset | AthenaList(s) | Difficulty Gate | Position Fields | Notes |
|---|---|---|---|---|---|---|---|---|
| LIFTER | strnicmp "LIFTER", 6 | 0x10F4 | Rotator_ctor_sound | +0x47E0 | +0x2578 | No | param_4+4/+8/+0xC | Parses _atol(param_1+6) for numeric arg. Falls through to CreatePlatformOrStands if no match |

---

## 7. Factory Function: Scene_CreateObject_Gear (0x00418760)

Used by: Impossible Race and other gear levels.

| Object Name | Match | Alloc Size | Constructor | Board Mesh Offset | AthenaList(s) | Difficulty Gate | Position Fields | Rotation Fields | Notes |
|---|---|---|---|---|---|---|---|---|---|
| GEAR | strnicmp "GEAR", 4 | 0x1514 | Gear_ctor | +0x47E0 | +0x2578 | No | param_4+4/+8/+0xC (pos1) + param_4+0x10/+0x14/+0x18 (pos2) | Both position AND rotation from param_block | Sets obj+0x10F4=0x40000000 (2.0f scale). Falls through to CreatePlatformOrStands |

---

## 8. Board Level Constructors — Mesh Loading & Board Offsets

### BoardLevel3_ctor (0x0041D060) — "Dizzy Race"
- Vtable: PTR_BoardLevel3_Dizzy_dtor_004d0890
- AthenaLists: +0x4378, +0x4790
- Race data: "DIZZYRACE", Display: "Dizzy!", Race name: "DIZZY RACE"
- Gravity: (0, 1.0, 0)

| Mesh Path | VA (String) | Board Offset | Collision Offset | Object Type |
|---|---|---|---|---|
| Levels\Level3-Tipper | 0x4D07E8 | +0x436C | +0x4370 | TIPPER mesh |
| Levels\Level3-WaterWheel | 0x4D0794 | +0x4BA8 | +0x4BAC | WATERWHEEL (spinning floor) |
| Levels\Level3-Swirl | 0x4CFFE0 | +0x4BC4 | +0x4BC8 | SWIRL mesh |
| Levels\Level3-Gluebie | 0x4D0728 | +0x4374 | (no separate collision) | GLUEBIE mesh |

### BoardLevel5_Tower_ctor (0x0041E340) — "Tower Race"
- Vtable: PTR_BoardLevel5_Tower_dtor_004d0a08
- AthenaLists: +0x43B8, +0x47D0, +0x4BE8, +0x5000
- Race data: "TOWERRACE", Display: "Happy Rush", Race name: "TOWER RACE"
- Gravity: (1.0, 0.5, 0)

| Mesh Path | VA (String) | Board Offset | Collision Offset | Object Type |
|---|---|---|---|---|
| Levels\Level4-Catapult | 0x4D09B8 | +0x436C | (none) | CATAPULT mesh |
| Levels\Level4-Drawbridge | 0x4D099C | +0x4370 | (none) | DRAWBRIDGE/BREAKBRIDGE mesh |
| Meshes\YellowLink | 0x4D0988 | +0x4374 | (MeshNode, 0x18 bytes) | YellowLink visual |
| Levels\Level4-Mace | 0x4D0974 | +0x4378 | (none) | MACE mesh |
| Levels\Level4-Windmill | 0x4D095C | +0x437C | (none) | WINDMILL mesh |
| Meshes\Chomper | 0x4D094C | +0x4390 | (MeshNode, 0x18 bytes) | CHOMPER visual mesh |
| Levels\Level4-Turret | 0x4D0932* | +0x43B4 | (none) | TURRET mesh |

*Note: String at 0x4D0932 has a prefix byte "pB" before "Levels\Level4-Turret" (likely Ghidra parsing artifact; actual string starts at the "L").

### BoardLevel_Up_Ctor (0x00420390) — "Up Race"
- Vtable: PTR_BoardLevel_Generic_dtor3_004d11a0
- AthenaLists: +0x436C
- Race data: "UPRACE", Display: "Up Race", Race name: "UP RACE"
- Gravity: (1.0, 0, 1.0)

| Mesh Path | VA (String) | Board Offset | Object Type |
|---|---|---|---|
| levels\levelup-lifter | 0x4D1160 | +0x4784 | LIFTER mesh |
| levels\levelup-speedcylinder | 0x4D1140 | +0x4788 | SPEEDCYLINDER mesh |
| levels\levelup-button | 0x4D1128 | +0x478C | TIMEBUTTON mesh |

### BoardLevel8_Expert_ctor (0x0041EA40) — "Expert Race"
- Vtable: PTR_BoardLevel8_Expert_dtor_004d0b00
- AthenaLists: +0x4380, +0x4798, +0x4BBC
- Race data: "EXPERTRACE", Display: "Fight!", Race name: "EXPERT RACE"
- Gravity: (1.0, 0, 0)

| Mesh Path | VA (String) | Board Offset | Collision Offset | Object Type |
|---|---|---|---|---|
| Levels\Level5-Bridge | 0x4D0ABC | +0x4378 | +0x437C | BRIDGE/SPINNER mesh |
| meshes\hammyjudge | 0x4D0AA8 | +0x4BB0 | (MeshNode, 0x18 bytes) | JUDGE visual (1) |
| meshes\hammyjudge | 0x4D0AA8 | +0x4BB4 | (MeshNode, 0x18 bytes) | JUDGE visual (2) |
| meshes\hammyjudge | 0x4D0AA8 | +0x4BB8 | (MeshNode, 0x18 bytes) | JUDGE visual (3) |

### BoardLevel_Toob_Ctor (0x0041F4B0) — "Toob Race"
- Vtable: PTR_BoardLevel_Toob_dtor_004d0e78
- Vec3 array at +0x438C (8 elements of 0x418 each)
- Race data: "TOOBRACE", Display: "Rodenthood", Race name: "TOOB RACE"
- Gravity: (0.5, 0.5, 1.0)

| Mesh Path | VA (String) | Board Offset | Object Type |
|---|---|---|---|
| Levels\Level8-Spinny | 0x4D0E38 | +0x436C | SPINNY mesh |
| Levels\Level8-Saw | 0x4D0E24 | +0x4370 | SAW/SAWBLADE mesh |
| Levels\Level8-Fallout | 0x4D0E0C | +0x4374 | FALLOUT mesh |
| Levels\Level8-Blockdawg1 | 0x4D0DF0 | +0x4378 | BLOCKDAWG1 mesh |
| Levels\Level8-Blockdawg2 | 0x4D0DD4 | +0x437C | BLOCKDAWG2 mesh |

### BoardLevel_Master_Ctor (0x004206D0) — "Master Race"
- Vtable: PTR_BoardLevel_Generic_dtor4_004d12b0
- Vec3 array at +0x439C (4 elements of 0x418 each)
- AthenaLists: +0x5428, +0x584C, +0x5C64, +0x6080
- Race data: "MASTERRACE", Display: "Master Theme", Race name: "MASTER RACE"
- Gravity: (0.5, 0.5, 0.5)

| Mesh Path | VA (String) | Board Offset | Collision Offset | Object Type |
|---|---|---|---|---|
| Levels\Level2-Bridge | 0x4D055C | +0x436C | +0x4370 | BRIDGE mesh |
| Levels\Level10-2PBridge | 0x4D127C | +0x4374 | +0x4378 | BBRIDGE/2P mesh |
| Levels\Level3-Tipper | 0x4D07E8 | +0x4394 | +0x4398 | TIPPER mesh + visual |
| Levels\Level10-Bridge1 | 0x4D1264 | +0x5410 | (none) | BBRIDGE1 mesh |
| Levels\Level10-Bridge2 | 0x4D124C | +0x5414 | (none) | BBRIDGE2 mesh |
| levels\level9-popcylinder1 | 0x4D0F5C | +0x5420 | (none) | POPCYLINDER1 mesh |
| levels\level9-popcylinder2 | 0x4D0F40 | +0x5424 | (none) | POPCYLINDER2 mesh |
| Levels\Level8-Blockdawg1 | 0x4D0DF0 | +0x5840 | (none) | BLOCKDAWG1 mesh |
| Levels\Level8-Blockdawg2 | 0x4D0DD4 | +0x5844 | (none) | BLOCKDAWG2 mesh |
| Levels\Level4-Catapult | 0x4D09B8 | +0x5848 | (none) | CATAPULT mesh |
| Levels\Level3-Gluebie | 0x4D0728 | +0x607C | (none) | GLUEBIE mesh |

### RumbleBoard_Tower_Ctor (0x004228C0) — "Tower Arena"
- Vtable: PTR_RumbleBoard_OddArena_DeletingDtor_004d1740
- AthenaLists: +0x47E4, +0x4C00
- Race name: "TOWER ARENA", Display: "Happy Rush"

| Mesh Path | VA (String) | Board Offset | Object Type |
|---|---|---|---|
| Levels\Level4-Mace | 0x4D0974 | +0x47E0 | MACE mesh |
| Levels\Level4-Catapult | 0x4D09B8 | +0x4BFC | CATAPULT mesh |
| Levels\Level4-Turret | 0x4D0932 | +0x5018 | TURRET mesh |

---

## 9. Complete String Address Table (.rdata section)

### Object Name Strings (for strnicmp matching)
| VA | File Offset | String |
|---|---|---|
| 0x4CF678 | 0xCF678 | BRIDGE |
| 0x4CF680 | 0xCF680 | GLUEBIE |
| 0x4CF688 | 0xCF688 | SWIRL |
| 0x4CF690 | 0xCF690 | WATERWHEEL |
| 0x4CF69C | 0xCF69C | TIPPER |
| 0x4CF818 | 0xCF818 | PIPEBONK |
| 0x4CF900 | 0xCF900 | N:BRIDGE |
| 0x4CF928 | 0xCF928 | N:SWIRL |
| 0x4CF930 | 0xCF930 | N:WHEELEMBED |
| 0x4CF940 | 0xCF940 | N:WATERWHEEL |
| 0x4CF960 | 0xCF960 | TURRET |
| 0x4CF968 | 0xCF968 | CHOMPER |
| 0x4CF970 | 0xCF970 | TRAPDOOR |
| 0x4CF97C | 0xCF97C | WINDMILL |
| 0x4CF988 | 0xCF988 | DRAWBRIDGE |
| 0x4CF994 | 0xCF994 | MACE |
| 0x4CF99C | 0xCF99C | CATAPULT |
| 0x4CF9A0 | 0xCF9A0 | CATAPULT (alt offset) |
| 0x4CFA0C | 0xCFA0C | BELL |
| 0x4CFA14 | 0xCFA14 | JUDGE |
| 0x4CFA28 | 0xCFA28 | SAWBLADE |
| 0x4CFA48 | 0xCFA48 | FAN |
| 0x4CFA4C | 0xCFA4C | BONK |
| 0x4CFB1C | 0xCFB1C | LIFTER |
| 0x4CFCC0 | 0xCFCC0 | BLOCKDAWG3 |
| 0x4CFCD8 | 0xCFCD8 | BLOCKDAWG2 |
| 0x4CFCF0 | 0xCFCF0 | BLOCKDAWG1 |
| 0x4CFD2C | 0xCFD2C | SPINNY |
| 0x4CFD4C | 0xCFD4C | N:SPINNY |
| 0x4CFDB8 | 0xCFDB8 | POPCYLINDER |
| 0x4CFE40 | 0xCFE40 | TIMEBUTTON |
| 0x4CFE4C | 0xCFE4C | SPEEDCYLINDER |
| 0x4CFEB4 | 0xCFEB4 | BBRIDGE2 |
| 0x4CFEC0 | 0xCFEC0 | BBRIDGE1 |
| 0x4CFF04 | 0xCFF04 | BONKBASH |
| 0x4CFF10 | 0xCFF10 | BONKPOPUP |
| 0x4CFF98 | 0xCFF98 | levels\arena-WarmUp |
| 0x4CFEF8 | 0xCFEF8 | N:SPINNER |
| 0x4D01C0 | 0xD01C0 | PENDULUM |
| 0x4D01CC | 0xD01CC | ROTATOR |
| 0x4D01DC | 0xD01DC | BIGGEAR |
| 0x4D01E4 | 0xD01E4 | GEAR |
| 0x4D01EC | 0xD01EC | LOOPER |
| 0x4D0144 | 0xD0144 | FLICKRING |
| 0x4CFF5C | 0xCFF5C | STANDS |
| 0x4CFF64 | 0xCFF64 | PLATFORM |

### Mesh Path Strings
| VA | File Offset | String | Used By |
|---|---|---|---|
| 0x4CF8E0 | 0xCF8E0 | levels\level1 | Beginner Race |
| 0x4CF8F0 | 0xCF8F0 | levels\level2 | Intermediate Race |
| 0x4CF918 | 0xCF918 | levels\level3 | Dizzy Race (main mesh) |
| 0x4CF950 | 0xCF950 | levels\level4 | Tower Race (main mesh) |
| 0x4CFC1C | 0xCFC1C | levels\level7 | Wobbly Race (main mesh) |
| 0x4CFCA8 | 0xCFCA8 | levels\level8 | Toob Race (main mesh) |
| 0x4CFDA8 | 0xCFDA8 | levels\level9 | Sky Race (main mesh) |
| 0x4CFE30 | 0xCFE30 | levels\levelup | Up Race (main mesh) |
| 0x4CFEA4 | 0xCFEA4 | levels\level10 | Master/2P Race (main mesh) |
| 0x4D01A8 | 0xD01A8 | levels\levelimpossible | Impossible Race (main mesh) |
| 0x4D055C | 0xD055C | Levels\Level2-Bridge | Intermediate BRIDGE mesh |
| 0x4D07E8 | 0xD07E8 | Levels\Level3-Tipper | Dizzy TIPPER mesh |
| 0x4D0794 | 0xD0794 | Levels\Level3-WaterWheel | Dizzy WATERWHEEL (spinning floor) mesh |
| 0x4CFFE0 | 0xCFFE0 | Levels\Level3-Swirl | Dizzy SWIRL mesh |
| 0x4D0728 | 0xD0728 | Levels\Level3-Gluebie | Dizzy GLUEBIE mesh |
| 0x4D09B8 | 0xD09B8 | Levels\Level4-Catapult | Tower CATAPULT mesh |
| 0x4D099C | 0xD099C | Levels\Level4-Drawbridge | Tower DRAWBRIDGE mesh |
| 0x4D0974 | 0xD0974 | Levels\Level4-Mace | Tower MACE mesh |
| 0x4D095C | 0xD095C | Levels\Level4-Windmill | Tower WINDMILL mesh |
| 0x4D0932 | 0xD0932 | Levels\Level4-Turret | Tower TURRET mesh (note: prefixed with "pB" in raw) |
| 0x4D094C | 0xD094C | Meshes\Chomper | Tower CHOMPER visual mesh (MeshNode) |
| 0x4D0988 | 0xD0988 | Meshes\YellowLink | Tower YellowLink visual (MeshNode) |
| 0x4D33EC | 0xD33EC | Levels\Level4-Trapdoor2 | Tower TRAPDOOR2 mesh |
| 0x4D3404 | 0xD3404 | Levels\Level4-Trapdoor1 | Tower TRAPDOOR1 mesh |
| 0x4D0ABC | 0xD0ABC | Levels\Level5-Bridge | Expert BRIDGE/SPINNER mesh |
| 0x4D0AA8 | 0xD0AA8 | meshes\hammyjudge | Expert JUDGE visual (MeshNode, ×3) |
| 0x4D0E38 | 0xD0E38 | Levels\Level8-Spinny | Toob SPINNY mesh |
| 0x4D0E24 | 0xD0E24 | Levels\Level8-Saw | Toob SAWBLADE mesh |
| 0x4D0E0C | 0xD0E0C | Levels\Level8-Fallout | Toob FALLOUT mesh |
| 0x4D0DF0 | 0xD0DF0 | Levels\Level8-Blockdawg1 | Toob/Master BLOCKDAWG1 mesh |
| 0x4D0DD4 | 0xD0DD4 | Levels\Level8-Blockdawg2 | Toob/Master BLOCKDAWG2 mesh |
| 0x4D0F28 | 0xD0F28 | levels\level9-trapdoor | Sky TRAPDOOR mesh |
| 0x4D0F40 | 0xD0F40 | levels\level9-popcylinder2 | Sky/Master POPCYLINDER2 mesh |
| 0x4D0F5C | 0xD0F5C | levels\level9-popcylinder1 | Sky/Master POPCYLINDER1 mesh |
| 0x4D1160 | 0xD1160 | levels\levelup-lifter | Up LIFTER mesh |
| 0x4D1140 | 0xD1140 | levels\levelup-speedcylinder | Up SPEEDCYLINDER mesh |
| 0x4D1128 | 0xD1128 | levels\levelup-button | Up TIMEBUTTON mesh |
| 0x4D124C | 0xD124C | Levels\Level10-Bridge2 | Master BBRIDGE2 mesh |
| 0x4D1264 | 0xD1264 | Levels\Level10-Bridge1 | Master BBRIDGE1 mesh |
| 0x4D127C | 0xD127C | Levels\Level10-2PBridge | Master 2P BRIDGE mesh |
| 0x4D20DC | 0xD20DC | Levels\LevelImpossible-Pendulum | Impossible PENDULUM mesh |
| 0x4D20FC | 0xD20FC | Levels\LevelImpossible-Rotator | Impossible ROTATOR mesh |
| 0x4D211C | 0xD211C | Levels\LevelImpossible-BigGear | Impossible BIGGEAR mesh |
| 0x4D213C | 0xD213C | Levels\LevelImpossible-Gear | Impossible GEAR mesh |
| 0x4D2158 | 0xD2158 | Levels\LevelImpossible-Looper | Impossible LOOPER mesh |
| 0x4D3308 | 0xD3308 | Levels\Level6-Lifter | Impossible LIFTER mesh |
| 0x4D3390 | 0xD3390 | Meshes\sawblade | Expert SAWBLADE visual (MeshNode) |
| 0x4D3468 | 0xD3468 | Meshes\Bell | Expert BELL visual (MeshNode) |
| 0x4D5C10 | 0xD5C10 | levels\level5-bonk | Expert BONK mesh |

### Sound Path Strings
| VA | File Offset | String |
|---|---|---|
| 0x4D2BC8 | 0xD2BC8 | sounds\breakbridge |
| 0x4D2C00 | 0xD2C00 | sounds\speedcylinder |
| 0x4D2D14 | 0xD2D14 | sounds\bell |
| 0x4D2DE8 | 0xD2DE8 | sounds\catapult |
| 0x4D2E54 | 0xD2E54 | sounds\bridgeslam |
| 0x4D2E68 | 0xD2E68 | sounds\gearclank |

---

## 10. Board Offset Summary (Consolidated)

| Board Offset | Type | Description | Used By |
|---|---|---|---|
| +0x2578 | AthenaList | Active game objects list (ALL objects) | All factories |
| +0x436C | MeshWorld* | Bridge/Tipper/Catapult/Spinny/Looper mesh (varies by level) | Dizzy, Tower, Expert, Master, Impossible |
| +0x4370 | CollisionLevel*/MeshWorld* | Collision for +0x436C / or secondary mesh | Dizzy, Tower, Expert, Master |
| +0x4374 | MeshWorld* | Gluebie mesh (Dizzy) / 2PBridge (Master) / Fallout (Toob) | Dizzy, Master, Toob |
| +0x4378 | AthenaList*/MeshWorld* | Dizzy: AthenaList / Tower: Mace / Expert: Bridge / Toob: Blockdawg1 | Dizzy, Tower, Expert, Toob |
| +0x437C | CollisionLevel*/MeshWorld* | Dizzy: collision / Tower: Windmill / Expert: collision / Toob: Blockdawg2 | Dizzy, Tower, Expert, Toob |
| +0x4380 | AthenaList | Expert: Spinner bridge list 1 | Expert |
| +0x4388 | float | Master: 0x42340000 (40.0f) | Master |
| +0x438C | Vec3List[8] | Toob: vector array (0x418 × 8) | Toob |
| +0x4390 | MeshNode* | Tower: Chomper visual mesh | Tower |
| +0x4394 | MeshWorld* | Tipper mesh | Dizzy, Master |
| +0x4398 | CollisionLevel* | Tipper visual/collision | Dizzy, Master |
| +0x439C | Vec3List[4] | Master: vector array (0x418 × 4) | Master |
| +0x43A0-A8 | float[3] | Tower: zero-init (0, 0, 0) | Tower |
| +0x43B4 | MeshWorld* | Tower: Turret mesh | Tower |
| +0x43B8 | AthenaList | Tower: object list | Tower |
| +0x4790 | AthenaList | Dizzy: secondary list | Dizzy |
| +0x4798 | AthenaList | Expert: Spinner bridge list 2 | Expert |
| +0x47D0 | AthenaList | Tower: list 2 | Tower |
| +0x47E0 | MeshWorld* | Spinny/Gear/Lifter mesh (Impossible, Spinny, Lifter factories) | Impossible, Spinny, Lifter |
| +0x47E4 | AthenaList | Tower Arena: list | Tower Arena |
| +0x4784 | MeshWorld* | Up: Lifter mesh | Up |
| +0x4788 | MeshWorld* | Up: SpeedCylinder mesh | Up |
| +0x478C | MeshWorld* | Up: TimeButton mesh | Up |
| +0x4BA8 | MeshWorld* | Dizzy: WaterWheel mesh | Dizzy |
| +0x4BAC | CollisionLevel* | Dizzy: WaterWheel collision | Dizzy |
| +0x4BC4 | MeshWorld* | Dizzy: Swirl mesh | Dizzy |
| +0x4BC8 | CollisionLevel* | Dizzy: Swirl collision | Dizzy |
| +0x4BE8 | AthenaList | Tower: list 3 | Tower |
| +0x4BFC | MeshWorld* | Tower Arena: Catapult mesh | Tower Arena |
| +0x4FD4 | void* | Expert: BELL object pointer | Expert |
| +0x5000 | AthenaList | Tower: list 4 | Tower |
| +0x5018 | MeshWorld* | Tower Arena: Turret mesh | Tower Arena |
| +0x540C | void* | Bonk object pointer (Level3/Master) | Dizzy, Master |
| +0x5410 | MeshWorld* | BreakBridge1 mesh | Master |
| +0x5414 | MeshWorld* | BreakBridge2 mesh | Master |
| +0x5418 | void* | BreakBridge1 object pointer | Master |
| +0x541C | void* | BreakBridge2 object pointer | Master |
| +0x5420 | MeshWorld* | PopCylinder mesh | Master |
| +0x5424 | MeshWorld* | PopCylinder2 mesh (Master only) | Master |
| +0x5428 | AthenaList | PopCylinder list | Master |
| +0x5840 | MeshWorld* | BlockDawg1 mesh | Master, Toob |
| +0x5844 | MeshWorld* | BlockDawg2 mesh | Master, Toob |
| +0x5848 | MeshWorld* | Catapult mesh | Master |
| +0x584C | AthenaList | Catapult list | Master |
| +0x5C64 | AthenaList | (unknown - Master) | Master |
| +0x607C | MeshWorld* | Gluebie mesh | Master |
| +0x6080 | AthenaList | Gluebie list | Master |
| +0x868 | char* | Board display name string | All |
| +0x870 | int | Race ID from App+0x14+0x1DC | All |
| +0x878 | App* | App pointer | All |
| +0x8AC | Scene* | Scene pointer (for Level_FindObjectByName) | All |
| +0x29B4 | char* | Race name string | All |
| +0x4344 | char* | Display/theme string | All |

---

## 11. Object Allocation Size Summary

| Object Type | Alloc Size | Constructor | Called From |
|---|---|---|---|
| Tipper | 0x1104 | Tipper_ctor | CreateLevelObjects |
| TipperVisual | 0x10D0 | TipperVisual_ctor | CreateLevelObjects |
| Bonk | 0x1200 | Bonk_ctor | CreateLevelObjects, CreateExpertLevelObjects |
| BreakBridge (BBRIDGE1/2) | 0x1100 | BreakBridge_ctor | CreateLevelObjects |
| PopCylinder | 0x10E8 | PopCylinder_ctor | CreateLevelObjects |
| BlockDawg | 0x1154 | Blockdawg_ctor | CreateLevelObjects |
| Catapult | 0x1108 | Catapult_ctor | CreateLevelObjects |
| Gluebie | 0x110C | Gluebie_ctor | CreateLevelObjects |
| Lifter | 0x10F4 | Rotator_ctor_sound | CreateUpLevelObjects, CreateLifter |
| SpeedCylinder | 0x150C | Pendulum_ctor | CreateUpLevelObjects |
| TimeButton | 0x10E8 | Rotator_ctor_nosound | CreateUpLevelObjects |
| Looper | 0x1500 | Looper_ctor | CreateMechanicalObjects |
| Gear | 0x1514 | Gear_ctor | CreateMechanicalObjects, Scene_CreateObject_Gear |
| BigGear | 0x1514 | Gear_ctor (same, with scale 0.5) | CreateMechanicalObjects |
| Rotator | 0x1508 | Rotator_ctor | CreateMechanicalObjects, CreateSpinny |
| Pendulum | 0x1504 | Pendulum_ctor | CreateMechanicalObjects |
| Spinny | 0x1508 | Rotator_ctor | CreateSpinny |
| Fan (TowerLevel) | 0x1188 | TowerLevel_Ctor | CreateExpertLevelObjects |
| Sawblade | 0x111C | Sawblade_Level_Ctor | CreateExpertLevelObjects |
| Spinner (Expert Bridge) | 0x10FC | Spinner_Level_ctor | CreateExpertLevelObjects |
| Judge | 0x1100 | Gear_Level_ctor | CreateExpertLevelObjects |
| Bell | 0x10E8 | Tipper_Level_Ctor | CreateExpertLevelObjects |
| MeshWorld | 0x10D0 | MeshWorld_ctor | All Board ctors |
| MeshNode | 0x18 | MeshNode_ctor | Tower, Expert Board ctors |
| CollisionLevel | 0x10D0 | CollisionLevel_ctorWithLevel | All Board ctors |

---

## 12. Difficulty Gate Summary

Objects that ONLY spawn on Hard difficulty (App+0x23C != 0):
- **TIPPER** (CreateLevelObjects)
- **BONK** (CreateLevelObjects, CreateExpertLevelObjects)
- **BLOCKDAWG1** (CreateLevelObjects)
- **BLOCKDAWG2** (CreateLevelObjects)
- **GLUEBIE** (CreateLevelObjects — inverted: skipped when App+0x23C == 0)
- **FAN** (CreateExpertLevelObjects)
- **SAWBLADE** (CreateExpertLevelObjects)

Objects that spawn regardless of difficulty:
- BRIDGE, BBRIDGE1, BBRIDGE2, POPCYLINDER, CATAPULT (CreateLevelObjects)
- LIFTER, SPEEDCYLINDER, TIMEBUTTON (CreateUpLevelObjects)
- LOOPER, GEAR, BIGGEAR, ROTATOR, PENDULUM (CreateMechanicalObjects)
- SPINNY (CreateSpinny)
- GEAR (Scene_CreateObject_Gear)
- BRIDGE(Expert), JUDGE, BELL (CreateExpertLevelObjects)

---

## 13. param_block Layout (Confirmed)

The `param_4` pointer points to a structure with:
```
+0x00: (header/flags)
+0x04: float position.X
+0x08: float position.Y  
+0x0C: float position.Z
+0x10: float rotation.X (or secondary position for GEAR/BIGGEAR)
+0x14: float rotation.Y (or extra float for FAN/Spinner)
+0x18: float rotation.Z
```

For most objects, position is copied from param_4+4/+8/+0xC.
For GEAR/BIGGEAR, both position (param_4+4/+8/+0xC) AND rotation (param_4+0x10/+0x14/+0x18) are used.
For FAN and Expert BRIDGE (Spinner), param_4+0x14 is passed as an additional float parameter.
