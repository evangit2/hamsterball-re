# Universal Ref Loader — Design Document

**Created:** 2026-06-23  
**Session:** RodentRacer investigation into ref-loading system  
**Purpose:** Enable loading ANY object ref into ANY race level via a DLL mod, and fix the multi-instance problem for static-mesh objects.

---

## Table of Contents

1. [Architecture Overview](#1-architecture-overview)
2. [How Ref Loading Works](#2-how-ref-loading-works)
3. [Two Board Systems: Race vs Arena](#3-two-board-systems-race-vs-arena)
4. [Factory Dispatch via vtable[33]](#4-factory-dispatch-via-vtable33)
5. [Board Constructor Mesh Pre-Loading](#5-board-constructor-mesh-pre-loading)
6. [Object Categories: Allocating vs Static vs Configuring](#6-object-categories-allocating-vs-static-vs-configuring)
7. [The Multi-Instance Problem](#7-the-multi-instance-problem)
8. [The Mesh Dependency Problem](#8-the-mesh-dependency-problem)
9. [The Difficulty Gate Problem](#9-the-difficulty-gate-problem)
10. [The Universal Ref Loader Design (Option B)](#10-the-universal-ref-loader-design-option-b)
11. [Clone-on-Return for Static Mesh Objects (Option A)](#11-clone-on-return-for-static-mesh-objects-option-a)
12. [Complete Object Dependency Table](#12-complete-object-dependency-table)
13. [Complete vtable[33] Mapping](#13-complete-vtable33-mapping)
14. [Complete Board Slot → Mesh File Mapping](#14-complete-board-slot--mesh-file-mapping)
15. [Key Function Addresses](#15-key-function-addresses)
16. [Implementation Plan](#16-implementation-plan)
17. [Build & Test](#17-build--test)

---

## 1. Architecture Overview

Hamsterball loads level objects in a two-layer pipeline:

```
MESHWORLD file (Section 1: ref points)
  ↓
Scene_CreateDynamicObjects (0x40C430) — iterates ref points
  ↓  For each ref:
  ↓  calls board->vtable[33](refName, &outObj, &outCol, refEntry) at 0x40C4BA
  ↓
Factory Function (level-specific, via vtable[33])
  ↓  Matches ref name via __strnicmp
  ↓  Allocates object, calls constructor, sets position
  ↓  Returns object pointer in outObj
  ↓
Scene adds object to active list (+0x335), sets position from MESHWORLD data
  ↓  Calls object vtable[22] (Init) and vtable[21] (SetTimer)
```

### Scene_CreateDynamicObjects (0x40C430) — Full Decompiled Flow

```c
void __fastcall Scene_CreateDynamicObjects(int *param_1)
{
    // Iterate through MESHWORLD Section 1 ref points
    // For each ref (puVar4 = ref entry with name, x, y, z, ...):
    
    local_58 = NULL;  // output object pointer
    local_54 = 0;     // output secondary object pointer (trigger zones)
    
    // Call the Board's factory function (vtable[33], offset +0x84)
    (*(board->vtable[33]))(*puVar4, &local_58, &local_54, puVar4);
    
    if (local_58 != NULL) {
        // Object was created successfully
        Gfx_ScaleX(_DAT_004cf44c - puVar4[5]);           // set scale
        Gfx_SetPosition(puVar4[1], puVar4[2], puVar4[3]); // set position from MW data
        
        AthenaList_Append(scene + 0x335, local_58);       // add to dynamic object list
        AthenaList_Append(mw_parser + 0x1c, local_58);    // add to MW object list
        
        (*local_58->vtable[22])();    // Init/Setup
        (*local_58->vtable[21])(timer); // SetTimer
        
        if (local_54 != 0) {
            AthenaList_Append(scene + 0x43B, local_54);  // add trigger zone
            AthenaList_Append(scene_physics + 0x18, local_54);
        }
    }
    
    // Advance to next ref point
}
```

**Key insight:** The factory returns a pointer in `local_58`. If it's non-NULL, the scene adds it to the dynamic object list and initializes it. The position comes from the MESHWORLD ref data (`puVar4[1], puVar4[2], puVar4[3]`).

---

## 2. How Ref Loading Works

### MESHWORLD Section 1 Ref Points

Each ref point in a MESHWORLD file contains:
- **Name** (string, e.g. `"CATAPULT"`, `"WATERWHEEL"`, `"N:BUMPER"`)
- **Position** (3 floats: x, y, z)
- **Rotation** (3 floats: x, y, z)
- **Material** (4×4 float matrix + Power + reflection + texture)
- **Extra params** (passed as `param_4` to the factory)

### Ref Name → Factory Matching

The factory function uses `__strnicmp` to match ref name prefixes:
- `"CATAPULT"` matches any ref starting with "CATAPULT" (case-insensitive)
- `"N:"` prefix refs are handled by a separate N:/E: handler at `0x40C5D0`
- `"E:"` prefix refs are event triggers (score, gravity, etc.)

### Two Passes

1. **vtable[33] factory** — matches bare ref names (CATAPULT, TIPPER, etc.)
2. **N:/E: handler** — called from within the factory for `N:` and `E:` prefixed refs

The handler is called from 25 sites within the vtable[33] factory functions.

---

## 3. Two Board Systems: Race vs Arena

Hamsterball has TWO independent Board systems, selected by `App+0x237`:

| Mode | App+0x237 | Board Constructors | vtable Range | Switch Function |
|------|-----------|-------------------|-------------|-----------------|
| Race | 1 (non-zero) | 0x422xxx | 0x4D1428–0x4D2298 | 0x426780 (jump table at 0x426AB0) |
| Arena | 0 | 0x41Cxxx | 0x4D04A8–0x4D21C0 | 0x427080 (switch at 0x427140) |

**Critical:** Race factories handle FAR FEWER ref types than Arena factories. Levels 1, 2, 3, 9, 10, 12, 14 use ONLY the base race factory (`0x4133E0`) which handles only `PLATFORM`, `STANDS`, `N:BUMPER`. Their level-specific objects (tippers, bridges, waterwheels) are NOT created in race mode — they're loaded via the Arena Board system or are part of static geometry.

**The universal ref loader should hook at 0x40C4BA** (the vtable[33] call site inside Scene_CreateDynamicObjects), which intercepts BOTH Race and Arena modes.

---

## 4. Factory Dispatch via vtable[33]

### The Dispatch Call Site

At address `0x0040C4BA` inside `Scene_CreateDynamicObjects`:
```asm
; *board is in a register, vtable[33] is at offset +0x84
mov eax, [board]          ; get vtable pointer
call [eax + 0x84]        ; call vtable[33] factory
; args: refName, &outObj, &outCol, refEntry
```

### Factory Function Signature

```c
void __thiscall FactoryFunc(
    Board* this,           // ECX = board pointer
    const char* refName,   // [ESP+4] = ref name string from MESHWORLD
    void** outObj,         // [ESP+8] = output: created object pointer
    void** outCol,         // [ESP+0xC] = output: secondary object (trigger zone)
    RefEntry* refEntry     // [ESP+0x10] = full ref entry (name, pos, rot, material)
);
```

### Complete vtable[33] Mapping

#### Race Board Factories

| # | Race Level | Constructor | Board Vtable | Factory Addr | Refs Handled |
|---|-----------|------------|-------------|-------------|-------------|
| 1 | Warm-up | 0x4224A0 | 0x4D1428 | 0x4133E0 | PLATFORM, STANDS (base only) |
| 2 | Beginner | 0x422550 | 0x4D14F0 | 0x4133E0 | PLATFORM, STANDS (base only) |
| 3 | Intermediate | 0x4226E0 | 0x4D15C0 | 0x4133E0 | PLATFORM, STANDS (base only) |
| 4 | Dizzy | 0x422790 | 0x4D1680 | 0x4143D0 | SPINNY, MACE, CATAPULT, TURRET, LIFTER, FAN + base |
| 5 | Tower | 0x4228C0 | 0x4D1740 | 0x414680 | MACE, CATAPULT, TURRET, LIFTER, FAN + base |
| 6 | Up | 0x422B10 | 0x4D17F8 | 0x414A20 | LIFTER, FAN, WOBBLY + base |
| 7 | Neon | 0x424860 | 0x4D1EC8 | 0x4173B0 | NEONPLATFORM, DFLOOR, TRODE + base |
| 8 | Expert | 0x423060 | 0x4D18C8 | 0x414BD0 | FAN, WOBBLY + base |
| 9 | Odd | 0x423220 | 0x4D1980 | 0x4133E0 | PLATFORM, STANDS (base only) |
| 10 | Toob | 0x4234E0 | 0x4D1A40 | 0x4133E0 | PLATFORM, STANDS (base only) |
| 11 | Wobbly | 0x423690 | 0x4D1B18 | 0x415460 | WOBBLY, PILLAR, POPCYLINDER + base |
| 12 | Glass | 0x424B60 | 0x4D2048 | 0x4133E0 | PLATFORM, STANDS (base only) |
| 13 | Sky | 0x423BF0 | 0x4D1BD8 | 0x415A30 | POPCYLINDER + base |
| 14 | Master | 0x424380 | 0x4D1C80 | 0x4133E0 | PLATFORM, STANDS (base only) |
| 15 | Impossible | 0x424EC0 | 0x4D2298 | 0x418760 | GEAR + base |

#### Arena Board Factories

| # | Arena Level | Constructor | Board Vtable | Factory Addr | Refs Handled |
|---|-----------|------------|-------------|-------------|-------------|
| 1 | Warm-up Arena | 0x41CA40 | 0x4D04A8 | 0x419750 | (none — NoOp) |
| 2 | Beginner Arena | 0x4200E0 | 0x4D1098 | 0x419750 | (none — NoOp) |
| 3 | Intermediate Arena | 0x41CB20 | 0x4D05A0 | 0x40A550 | BRIDGE, TIPPER, WATERWHEEL, SWIRL, GLUEBIE, SMASHER1, SMASHER2 |
| 4 | Dizzy Arena | 0x41D060 | 0x4D0890 | 0x40A5F0 | TIPPER, WATERWHEEL, SWIRL, GLUEBIE, SMASHER1, SMASHER2 |
| 5 | Tower Arena | 0x41E340 | 0x4D0A08 | 0x40D7C0 | CATAPULT, MACE, DRAWBRIDGE, WINDMILL, TRAPDOOR, CHOMPER, BONK, FAN, SAWBLADE, BRIDGE, JUDGE, BELL |
| 6 | Up Arena | 0x420390 | 0x4D11A0 | 0x4117B0 | LIFTER, SPEEDCYLINDER, TIMEBUTTON, TarBubble, BRIDGE, TIPPER, BONK, BBRIDGE1-2, POPCYLINDER, BLOCKDAWG1-2, CATAPULT |
| 7 | Neon Arena | 0x424440 | 0x4D1DF0 | 0x416910 | NEONPLATFORM, DFLOOR1-4, TRODE, N:NEONPLATFORM, E:ZOOP, E:LIGHTSOFF, E:LIGHTSON, FLICKNING, N:BUMP, N:GLASS, N:TENBONUS1 |
| 8 | Expert Arena | 0x41EA40 | 0x4D0B00 | 0x40E250 | BONK, FAN, SAWBLADE, BRIDGE, JUDGE, BELL, E:SCORE, E:BELL, LIFTER, E:GRAVITY |
| 9 | Odd Arena | 0x41ED80 | 0x4D0BC0 | 0x40EC40 | LIFTER, E:GRAVITY, WOBBLY1-7, WAVY1, N:SQUAREWOBBLY, N:WAVY, SPINNY |
| 10 | Toob Arena | 0x41F4B0 | 0x4D0E78 | 0x40FB30 | SPINNY, FALLOUT1, BLOCKDAWG1-3, E:ALERTSAW2, E:BRANCH, N:SPINNY, N:SAWTEETH, N:BUMPER, PILLAR, MAGNIFYER |
| 11 | Glass Arena | 0x41F110 | 0x4D0D38 | 0x40F420 | WOBBLY1-7, WAVY1, N:SQUAREWOBBLY, N:WAVY, SPINNY, FALLOUT1, BLOCKDAWG1-3, E:ALERTSAW2, E:BRANCH, N:SPINNY |
| 12 | Wobbly Arena | 0x424A90 | 0x4D1F90 | 0x40AD80 | SMASHER1, SMASHER2, SECRETUNLOCK, SECRET, BADBALL |
| 13 | Sky Arena | 0x41F930 | 0x4D0FC8 | 0x410AD0 | POPCYLINDER, TRAPDOOR, N:BUMPER, VAC-IN, LIFTER, SPEEDCYLINDER, TIMEBUTTON |
| 14 | Master Arena | 0x4206D0 | 0x4D12B0 | 0x4121D0 | BRIDGE, TIPPER, BONK, BBRIDGE1-2, POPCYLINDER, BLOCKDAWG1-2, CATAPULT, GLUEBIE, N:SPINNER, N:BUMPER, E:LAUNCH |
| 15 | Impossible Arena | 0x424C20 | 0x4D21C0 | 0x417FE0 | LOOPER, GEAR, BIGGEAR, ROTATOR, PENDULUM, N:BOUNCE, N:ONROTATOR, N:ONGEAR |

**Key insight:** Levels 1, 2, 3, 9, 10, 12, 14 Race factories handle ONLY `PLATFORM, STANDS`. Their interesting objects (tippers, bridges, waterwheels) are NOT created in race mode by vtable[33] — they're loaded via the Arena Board system or are static geometry.

---

## 5. Board Constructor Mesh Pre-Loading

Each Board constructor loads specific mesh files into `board+0x4xxx` slots BEFORE the factory runs. The factory then reads these slots to get the mesh data needed by each object's constructor.

### How It Works

```c
// Inside BoardLevel3_ctor (Dizzy Race, 0x41D060):
pvVar1 = operator_new(0x10D0);
pvVar1 = MeshWorld_ctor(pvVar1, App+0x174, "Levels\\Level3-WaterWheel");
*(void**)(board + 0x4BA8) = pvVar1;  // store mesh at board+0x4BA8

pvVar1 = operator_new(0x10D0);
pvVar1 = CollisionLevel_ctorWithLevel(pvVar1, board+0x4BA8);
*(void**)(board + 0x4BAC) = pvVar1;  // store collision at board+0x4BAC
```

### The Conflict Problem

**The same `board+0x4xxx` slot is reused for different meshes on different levels.** For example:

| Board Slot | Beginner | Intermediate | Tower | Expert | Impossible | Master | Up |
|---|---|---|---|---|---|---|---|
| +0x436C | *(empty)* | Level2-Bridge | Level4-Catapult | *(empty)* | Looper | BreakBridge1 | *(empty)* |
| +0x4370 | *(empty)* | (collision) | Level4-Drawbridge | *(empty)* | BigGear | BreakBridge2 | *(empty)* |
| +0x4374 | *(empty)* | *(empty)* | YellowLink(MeshNode) | *(empty)* | Rotator | PopCylinder2 | *(empty)* |
| +0x4378 | *(empty)* | *(empty)* | Level4-Mace | Level5-Bridge | Rotator | PopCylinder | *(empty)* |
| +0x437C | *(empty)* | *(empty)* | Level4-Windmill | Collision | Pendulum | *(empty)* | *(empty)* |

You cannot simply "load all meshes" into board slots — you'd overwrite one mesh with another.

### Solution: Just-In-Time Mesh Injection

The DLL maintains its own private array of all 46 object meshes loaded from disk. When the factory needs a specific mesh, the DLL:
1. Saves the current board slot value
2. Writes the correct mesh pointer into the slot
3. Calls the constructor (which reads the slot)
4. Restores the original board slot value

---

## 6. Object Categories: Allocating vs Static vs Configuring

All 46 verified ref types fall into three categories:

### Category 1 — Allocating Objects (MULTI-INSTANCE SAFE)

The factory allocates a new object each time, calls a constructor, and appends it to the active object list (`board+0x2578`). Multiple instances work fine — each gets independent state. They share the pre-loaded mesh at `board+0x4xxx` (read-only), which is fine for rendering.

| Object | Alloc Size | Constructor | Board Mesh Slot (READ) | AthenaList | Difficulty Gate |
|--------|-----------|-------------|----------------------|------------|-----------------|
| TIPPER | 0x1104 | Tipper_ctor | +0x4394, +0x4398 | +0x2578 | YES (App+0x23C≠0) |
| BONK | 0x1200 | Bonk_ctor | *(self-loads)* | +0x2578 | YES (App+0x23C≠0) |
| BREAKBRIDGE1 | 0x1100 | BreakBridge_ctor | +0x5410 | +0x2578 | No |
| BREAKBRIDGE2 | 0x1100 | BreakBridge_ctor | +0x5414 | +0x2578 | No |
| POPCYLINDER | 0x10E8 | PopCylinder_ctor | +0x5420 | +0x2578, +0x5428 | No |
| BLOCKDAWG1 | 0x1154 | Blockdawg_ctor | +0x5840 | +0x2578 | YES (App+0x23C≠0) |
| BLOCKDAWG2 | 0x1154 | Blockdawg_ctor | +0x5844 | +0x2578 | YES (App+0x23C≠0) |
| CATAPULT | 0x1108 | Catapult_ctor | +0x5848 | +0x2578, +0x584C | No |
| GLUEBIE | 0x110C | Gluebie_ctor | +0x607C | +0x6080, +0x2578 | YES (App+0x23C≠0) |
| LIFTER | 0x10F4 | Rotator_ctor_sound | +0x4784 (Up) / +0x47E0 (Odd) | +0x2578 | No |
| SPEEDCYLINDER | 0x150C | Pendulum_ctor | +0x4788 | +0x2578 | No |
| TIMEBUTTON | 0x10E8 | Rotator_ctor_nosound | +0x478C | +0x2578 | No |
| LOOPER | 0x1500 | Looper_ctor | +0x436C | +0x2578 | No |
| GEAR | 0x1514 | Gear_ctor | +0x4370 | +0x2578 | No |
| BIGGEAR | 0x1514 | Gear_ctor | +0x4374 | +0x2578 | No |
| ROTATOR | 0x1508 | Rotator_ctor | +0x4378 | +0x2578 | No |
| PENDULUM | 0x1504 | Pendulum_ctor | +0x437C | +0x2578 | No |
| FAN | 0x1188 | TowerLevel_Ctor | *(none — procedural)* | +0x2578 | YES (App+0x23C≠0) |
| SAWBLADE | 0x111C | Sawblade_Level_Ctor | *(none)* | +0x2578 | YES (App+0x23C≠0) |
| BRIDGE-Expert | 0x10FC | Spinner_Level_ctor | +0x4378 | +0x2578 (conditional) | No |
| JUDGE | 0x1100 | Gear_Level_ctor | *(none)* | +0x4BBC | No |
| BELL | 0x10E8 | Tipper_Level_Ctor | *(none)* | +0x2578 | No |
| DRAWBRIDGE | 0x113C | Glass_Level_ctor | +0x4370 | +0x2578 | No |
| MACE | 0x110C | (0x438750) | +0x4378 | +0x2578 | No |
| TRAPDOOR | 0x10F8 | (0x438290) | *(none)* | +0x2578 | No |
| TURRET | 0x10D0 | Stands_ctor | +0x43B4 | *(vtable dispatch)* | No |
| SPINNY | 0x1508 | Rotator_ctor | +0x47E0 | +0x2578 | No |
| SMASHER1 | — | — | — | +0x2578 | No |
| SMASHER2 | — | — | — | +0x2578 | No |
| WAVY | — | — | — | +0x2578 | No |
| WOBBLY | — | GameLevel_ctor | — | +0x2578 | No |

### Category 2 — Static Mesh Return Objects (MULTI-INSTANCE BROKEN)

The factory does **NO allocation, NO constructor, NO list append**. It returns the pre-loaded mesh pointer directly. Multiple refs all point to the SAME object in memory → only one renders (at the last position set).

| Object | Board Slot (returned) | Level | Description |
|--------|----------------------|-------|-------------|
| WATERWHEEL | +0x4BA8 | Dizzy | Spinning floor mesh |
| SWIRL | +0x4BC4 | Dizzy | Swirl mesh |
| CHOMPER | +0x4390 | Tower | MeshNode visual only (no collision/game logic) |
| WINDMILL | +0x437C | Tower | CollisionLevel only (no game object/render) |
| PILLAR | *(static)* | Sky | Static mesh + collision |

### Category 3 — Configuring Objects

The factory doesn't create anything — it moves/reconfigures the existing mesh.

| Object | Board Slot | Description |
|--------|-----------|-------------|
| BRIDGE (base) | +0x436C | Configures existing bridge mesh position/collision |

---

## 7. The Multi-Instance Problem

### Root Cause

When you add two WATERWHEEL refs to a Dizzy Race MESHWORLD file:

1. `Scene_CreateDynamicObjects` calls `vtable[33]` for the first WATERWHEEL ref
2. The factory returns `board+0x4BA8` (the pre-loaded mesh pointer) in `local_58`
3. The scene adds it to the dynamic list, sets position A, calls Init
4. `Scene_CreateDynamicObjects` calls `vtable[33]` for the second WATERWHEEL ref
5. The factory returns `board+0x4BA8` again (THE SAME pointer) in `local_58`
6. The scene adds it to the list again, sets position B, calls Init
7. **Result:** Both list entries point to the same MeshWorld object. The second Init overwrites the first position. Only one renders (at position B).

### The Fix: Clone-on-Return (Option A — CHOSEN)

When the universal ref-loader DLL detects a Category 2 object, it calls `Level_CloneTree` (0x466060) to create an independent copy before returning it.

```c
// In the DLL factory hook, when matching WATERWHEEL/SWIRL/CHOMPER/WINDMILL/PILLAR:
void* mesh = *(void**)(board + boardOffset);  // pre-loaded mesh
void* clone = Level_CloneTree(mesh, board);     // 0x466060, thiscall
*outObj = (int)clone;                          // return the clone, not the original
```

#### Level_CloneTree (0x466060) — What It Does

```c
undefined4* __thiscall Level_CloneTree(void* this, int param_1)
{
    // 1. Allocate new 0x10D0-byte object (same size as MeshWorld)
    undefined4* clone = operator_new(0x10D0);
    
    // 2. Call Level_ctor with parent's D3D device
    Level_ctor(clone, *(undefined4*)((int)this + 4));
    
    // 3. Set vtable to PTR_Level_DeletingDtor2 (0x4D9068)
    *clone = &PTR_Level_DeletingDtor2_004d9068;
    
    // 4. Copy parent's SceneObject (this+0x480)
    clone[0x120] = *(undefined4*)((int)this + 0x480);
    clone[0x121] = 1;  // flag: has scene object
    
    // 5. Copy parent's world matrix (this+8)
    clone[2] = *(undefined4*)((int)this + 8);
    
    // 6. Copy collision flag (this+0x430)
    *(char*)((int)clone + 0x10C) = *(char*)((int)this + 0x430);
    
    // 7. Set parent (param_1 = board)
    clone[0x11F] = param_1;
    
    // 8. Copy timer pointer (param_1 + 0x434)
    clone[0x10D] = *(undefined4*)(param_1 + 0x434);
    
    // 9. Recursively clone children (spatial tree)
    // For each child in this+0x424:
    //   Level_CloneTree(child, param_1) → append to clone's child list
    
    return clone;
}
```

### Alternative Options (kept for reference)

**Option B — Re-load from file each time:**
```c
void* mesh = operator_new(0x10D0);
MeshWorld_ctor(mesh, *(int*)(*(int*)(board + 0x878) + 0x174), "Levels\\Level3-WaterWheel");
*outObj = (int)mesh;
```
Slower (re-parses file) but fully independent. Works even if board slot is empty (useful for cross-level loading).

**Option C — Use Stands_ctor pattern:**
```c
void* obj = operator_new(0x10D0);
Stands_ctor(obj, *(void**)(board + 0x4BA8));  // clones spatial tree from parent
*outObj = (int)obj;
```
This is what the game itself uses for TURRET, MACE, DRAWBRIDGE — objects that need independent instances sharing a mesh.

---

## 8. The Mesh Dependency Problem

### Problem

Each Board constructor loads specific mesh files into `board+0x4xxx` slots. The same slot holds different meshes on different levels. The factory's constructors READ these slots to get mesh data.

If a level's constructor didn't load a mesh into the required slot, the slot is NULL. The factory does NOT null-check — it dereferences the pointer directly, causing an access violation (crash).

### Solution: Just-In-Time Mesh Injection

The universal ref-loader DLL maintains a private array of all object meshes loaded from disk. When dispatching to a factory:

1. **Before calling the constructor:** Save the current values of all board slots the constructor will read
2. **Write the correct mesh pointers** from the private array into those slots
3. **Call the constructor** (which reads the slots)
4. **Restore the original board slot values** (so the level's own objects aren't broken)

### Which Objects Need Which Board Slots

| Object | Required Board Slot (READ) | Mesh File Path |
|--------|---------------------------|----------------|
| BRIDGE (base) | +0x436C | Level2-Bridge (varies by level) |
| TIPPER | +0x4394 (mesh), +0x4398 (visual) | Level3-Tipper |
| WATERWHEEL | +0x4BA8 (static return) | Level3-WaterWheel |
| SWIRL | +0x4BC4 (static return) | Level3-Swirl |
| CHOMPER | +0x4390 (static return) | Meshes\\Chomper |
| WINDMILL | +0x437C (collision only) | Level4-Windmill |
| DRAWBRIDGE | +0x4370 | Level4-Drawbridge |
| MACE | +0x4378 | Level4-Mace |
| TURRET | +0x43B4 | Level4-Turret |
| CATAPULT | +0x5848 | Level4-Catapult |
| BREAKBRIDGE1 | +0x5410 | Level10-Bridge1 |
| BREAKBRIDGE2 | +0x5414 | Level10-Bridge2 |
| POPCYLINDER | +0x5420 | Level9-PopCylinder1 |
| BLOCKDAWG1 | +0x5840 | Level8-Blockdawg1 |
| BLOCKDAWG2 | +0x5844 | Level8-Blockdawg2 |
| GLUEBIE | +0x607C | Level3-Gluebie |
| LOOPER | +0x436C | LevelImpossible-Looper |
| GEAR | +0x4370 | Gear |
| BIGGEAR | +0x4374 | BigGear |
| ROTATOR | +0x4378 | Rotator |
| PENDULUM | +0x437C | Pendulum |
| SPINNY | +0x47E0 | Level8-Spinny |
| LIFTER (Up) | +0x4784 | LevelUp-Lifter |
| SPEEDCYLINDER | +0x4788 | LevelUp-SpeedCylinder |
| TIMEBUTTON | +0x478C | LevelUp-Button |
| BRIDGE-Expert | +0x4378 | Level5-Bridge |
| BONK | *(self-loads levels\\level5-bonk)* | Level5-Bonk |
| FAN | *(none — procedural)* | — |
| SAWBLADE | *(none)* | — |
| JUDGE | *(none)* | — |
| BELL | *(none)* | — |
| TRAPDOOR | *(none)* | — |

### Objects That DON'T Need Pre-Loaded Meshes

These objects work immediately without any mesh injection:
- **BONK** — self-loads `levels\level5-bonk` via `MeshWorld_ctor` inside `Bonk_ctor`
- **FAN** — procedural animation, no mesh at all
- **SAWBLADE** — self-contained (no mesh read)
- **JUDGE** — no mesh read (calls `Level_ctor` base)
- **BELL** — no mesh read
- **TRAPDOOR** — no mesh dependency

---

## 9. The Difficulty Gate Problem

### Problem

Some objects are gated behind `App+0x23C != 0`:
- **TIPPER** — only created when `App+0x23C != 0`
- **BONK** — only created when `App+0x23C != 0`
- **BLOCKDAWG1/2** — only created when `App+0x23C != 0`
- **GLUEBIE** — only created when `App+0x23C != 0`
- **FAN** — only created when `App+0x23C != 0`
- **SAWBLADE** — only created when `App+0x23C != 0`

### What App+0x23C Is

`App+0x23C` is a **difficulty enum** (not a simple boolean):
- **0** = Pipsqueak (easiest) — gated objects don't spawn
- **1** = Normal — gated objects spawn
- **2** = Frenzied/Hard — gated objects spawn

The absolute address is `0x4FD8B4` (App singleton at `0x4FD680` + `0x23C`).

### Solution

The DLL temporarily sets `App+0x23C = 1` during factory dispatch for gated objects, then restores the original value afterward:

```c
int originalDiff = *(int*)(app + 0x23C);
*(int*)(app + 0x23C) = 1;  // force Normal difficulty
// ... call factory ...
*(int*)(app + 0x23C) = originalDiff;  // restore
```

---

## 10. The Universal Ref Loader Design (Option B)

### Overview

A `bass.dll` proxy that hooks the vtable[33] dispatch at `0x0040C4BA` and replaces it with a universal factory that:

1. **Tries all 13 Arena factories** in sequence (each factory's `__strnicmp` will match its own refs)
2. **For each factory, injects the required meshes** into board slots before calling
3. **For Category 2 objects (WATERWHEEL, etc.), clones the mesh** instead of returning the same pointer
4. **For difficulty-gated objects, temporarily sets App+0x23C = 1**
5. **Restores all board slots and App+0x23C** after the factory returns

### Hook Point

```asm
; Original at 0x0040C4BA:
call [eax + 0x84]  ; board->vtable[33](refName, &outObj, &outCol, refEntry)

; DLL patches this to:
jmp universal_factory_dispatch
```

### Universal Factory Dispatch (Pseudocode)

```c
void universal_factory_dispatch(
    Board* board, const char* refName, 
    void** outObj, void** outCol, RefEntry* refEntry)
{
    // 1. Try the original level's factory first
    original_factory(board, refName, outObj, outCol, refEntry);
    if (*outObj != NULL) {
        // Original factory handled it — check if it's a static-mesh return
        if (is_static_mesh_object(refName)) {
            // Clone it for multi-instance support
            *outObj = Level_CloneTree(*outObj, board);
        }
        return;
    }
    
    // 2. Original factory didn't handle it — try all other factories
    for (int i = 0; i < NUM_FACTORIES; i++) {
        FactoryFunc* factory = factories[i];
        
        // Check if this factory handles the ref name
        if (!factory_handles_ref(factory, refName))
            continue;
        
        // Get the mesh slots this factory needs
        int* slots = get_required_slots(factory);
        
        // Save original slot values
        void** saved = save_board_slots(board, slots);
        
        // Inject correct meshes
        inject_meshes(board, slots);
        
        // Handle difficulty gate
        int savedDiff = *(int*)(app + 0x23C);
        if (is_difficulty_gated(refName))
            *(int*)(app + 0x23C) = 1;
        
        // Call the factory
        factory(board, refName, outObj, outCol, refEntry);
        
        // Restore slots and difficulty
        restore_board_slots(board, slots, saved);
        *(int*)(app + 0x23C) = savedDiff;
        
        if (*outObj != NULL) {
            // Check for static-mesh return
            if (is_static_mesh_object(refName)) {
                *outObj = Level_CloneTree(*outObj, board);
            }
            return;
        }
    }
    
    // No factory handled it — return NULL (object not created)
    *outObj = NULL;
}
```

### Private Mesh Array

At board constructor time (hooked via a separate hook), the DLL loads all 46 object meshes from disk into a private array:

```c
struct MeshEntry {
    const char* refName;    // "WATERWHEEL", "CATAPULT", etc.
    const char* meshPath;   // "Levels\\Level3-WaterWheel", etc.
    void* mesh;              // loaded MeshWorld*
    void* collision;         // loaded CollisionLevel*
    int boardSlot;           // which board+0x4xxx slot to inject into
};

MeshEntry meshDatabase[] = {
    {"CATAPULT",    "Levels\\Level4-Catapult",    NULL, NULL, 0x5848},
    {"TIPPER",      "Levels\\Level3-Tipper",       NULL, NULL, 0x4394},
    {"WATERWHEEL",  "Levels\\Level3-WaterWheel",   NULL, NULL, 0x4BA8},
    // ... all 46 entries
};
```

---

## 11. Clone-on-Return for Static Mesh Objects (Option A)

### Which Objects Need Cloning

| Object | Board Slot | Why It Needs Cloning |
|--------|-----------|---------------------|
| WATERWHEEL | +0x4BA8 | Factory returns mesh pointer directly, no alloc |
| SWIRL | +0x4BC4 | Factory returns mesh pointer directly, no alloc |
| CHOMPER | +0x4390 | Factory returns MeshNode pointer, no alloc |
| WINDMILL | +0x437C | Factory returns CollisionLevel, no alloc |
| PILLAR | *(static)* | Returns static mesh + collision |
| BRIDGE (base) | +0x436C | Factory configures existing mesh, no alloc |

### Clone Implementation

```c
// Called after the factory returns, if the ref is a Category 2 object
void* clone_static_mesh(void* originalMesh, Board* board) {
    // Level_CloneTree is at 0x466060, thiscall (ECX = originalMesh)
    // It allocates 0x10D0 bytes, copies spatial tree recursively
    typedef void* (__thiscall *CloneTreeFn)(void* this, int param_1);
    CloneTreeFn cloneTree = (CloneTreeFn)0x466060;
    return cloneTree(originalMesh, (int)board);
}
```

### Detection Logic

```c
bool is_static_mesh_object(const char* refName) {
    return _strnicmp(refName, "WATERWHEEL", 10) == 0
        || _strnicmp(refName, "SWIRL", 5) == 0
        || _strnicmp(refName, "CHOMPER", 7) == 0
        || _strnicmp(refName, "WINDMILL", 8) == 0
        || _strnicmp(refName, "PILLAR", 6) == 0
        || _strnicmp(refName, "BRIDGE", 6) == 0;  // base BRIDGE, not Expert
}
```

---

## 12. Complete Object Dependency Table

### All 46 Verified Ref Types (from MESHWORLD binary parsing)

| Object | Factory | Alloc Size | Constructor | Board Mesh Slot (READ) | AthenaList(s) | Difficulty Gate | Multi-Instance? | Needs Clone? |
|--------|---------|-----------|-------------|----------------------|--------------|-----------------|---------------|-------------|
| BRIDGE (base) | CreateLevelObjects | N/A | None (configures mesh) | +0x436C | — | No | NO | YES |
| TIPPER | CreateLevelObjects | 0x1104 | Tipper_ctor | +0x4394, +0x4398 | +0x2578 | YES | YES | No |
| BONK | CreateLevelObjects/CreateExpertLevelObjects | 0x1200 | Bonk_ctor | *(self-loads)* | +0x2578 | YES | YES | No |
| BREAKBRIDGE1 | CreateLevelObjects | 0x1100 | BreakBridge_ctor | +0x5410 | +0x2578 | No | YES | No |
| BREAKBRIDGE2 | CreateLevelObjects | 0x1100 | BreakBridge_ctor | +0x5414 | +0x2578 | No | YES | No |
| POPCYLINDER | CreateLevelObjects | 0x10E8 | PopCylinder_ctor | +0x5420 | +0x2578, +0x5428 | No | YES | No |
| BLOCKDAWG1 | CreateLevelObjects | 0x1154 | Blockdawg_ctor | +0x5840 | +0x2578 | YES | YES | No |
| BLOCKDAWG2 | CreateLevelObjects | 0x1154 | Blockdawg_ctor | +0x5844 | +0x2578 | YES | YES | No |
| CATAPULT | CreateLevelObjects | 0x1108 | Catapult_ctor | +0x5848 | +0x2578, +0x584C | No | YES | No |
| GLUEBIE | CreateLevelObjects | 0x110C | Gluebie_ctor | +0x607C | +0x6080, +0x2578 | YES | YES | No |
| LIFTER | CreateUpLevelObjects/CreateLifter | 0x10F4 | Rotator_ctor_sound | +0x4784 or +0x47E0 | +0x2578 | No | YES | No |
| SPEEDCYLINDER | CreateUpLevelObjects | 0x150C | Pendulum_ctor | +0x4788 | +0x2578 | No | YES | No |
| TIMEBUTTON | CreateUpLevelObjects | 0x10E8 | Rotator_ctor_nosound | +0x478C | +0x2578 | No | YES | No |
| LOOPER | CreateMechanicalObjects | 0x1500 | Looper_ctor | +0x436C | +0x2578 | No | YES | No |
| GEAR | CreateMechanicalObjects | 0x1514 | Gear_ctor | +0x4370 | +0x2578 | No | YES | No |
| BIGGEAR | CreateMechanicalObjects | 0x1514 | Gear_ctor | +0x4374 | +0x2578 | No | YES | No |
| ROTATOR | CreateMechanicalObjects | 0x1508 | Rotator_ctor | +0x4378 | +0x2578 | No | YES | No |
| PENDULUM | CreateMechanicalObjects | 0x1504 | Pendulum_ctor | +0x437C | +0x2578 | No | YES | No |
| FAN | CreateExpertLevelObjects | 0x1188 | TowerLevel_Ctor | *(none)* | +0x2578 | YES | YES | No |
| SAWBLADE | CreateExpertLevelObjects | 0x111C | Sawblade_Level_Ctor | *(none)* | +0x2578 | YES | YES | No |
| BRIDGE-Expert | CreateExpertLevelObjects | 0x10FC | Spinner_Level_ctor | +0x4378 | +0x2578 (conditional) | No | YES | No |
| JUDGE | CreateExpertLevelObjects | 0x1100 | Gear_Level_ctor | *(none)* | +0x4BBC | No | YES | No |
| BELL | CreateExpertLevelObjects | 0x10E8 | Tipper_Level_Ctor | *(none)* | +0x2578 | No | YES | No |
| DRAWBRIDGE | CreateTowerObjects | 0x113C | Glass_Level_ctor | +0x4370 | +0x2578 | No | YES | No |
| MACE | CreateTowerObjects | 0x110C | (0x438750) | +0x4378 | +0x2578 | No | YES | No |
| TRAPDOOR | CreateTowerObjects | 0x10F8 | (0x438290) | *(none)* | +0x2578 | No | YES | No |
| TURRET | CreateTowerObjects | 0x10D0 | Stands_ctor | +0x43B4 | *(vtable)* | No | PARTIAL | No |
| CHOMPER | CreateTowerObjects | NONE | NONE (static) | +0x4390 | — | No | NO | YES |
| WINDMILL | CreateTowerObjects | N/A | CollisionLevel only | +0x437C | — | No | NO | YES |
| WATERWHEEL | CreateDizzyObjects | NONE | NONE (static) | +0x4BA8 | — | No | NO | YES |
| SWIRL | CreateDizzyObjects | NONE | NONE (static) | +0x4BC4 | — | No | NO | YES |
| SPINNY | CreateSpinny/CreateMechanicalObjects | 0x1508 | Rotator_ctor | +0x47E0 | +0x2578 | No | YES | No |
| SMASHER1 | CreateWobblyObjects | — | — | — | +0x2578 | No | YES | No |
| SMASHER2 | CreateWobblyObjects | — | — | — | +0x2578 | No | YES | No |
| WAVY | CreateOddObjects | — | — | — | +0x2578 | No | YES | No |
| WOBBLY | CreateOddObjects/CreateUpObjects | — | GameLevel_ctor | — | +0x2578 | No | YES | No |
| PILLAR | CreateSkyObjects | N/A | NONE (static) | *(static)* | — | No | NO | YES |
| SIGN-TARPIT | (separate dispatch) | — | — | — | — | No | YES | No |
| TARBUBBLE | CreateUpLevelObjects | — | — | — | — | No | YES | No |
| LAUNCH | (separate dispatch) | — | — | — | — | No | YES | No |
| MOUSETRAP | (separate dispatch) | — | — | — | — | No | YES | No |
| NEONPLATFORM | CreateNeonObjects | — | — | +0x4374 | +0x2578 | No | YES | No |
| DFLOOR1-4 | CreateNeonObjects | — | — | +0x4378-0x4384 | +0x2578 | No | YES | No |
| TRODE | CreateNeonObjects | — | — | +0x4388 | +0x2578 | No | YES | No |
| FLICKNING | CreateNeonObjects | — | — | — | +0x2578 | No | YES | No |
| POPCYLINDER (Sky) | CreateSkyObjects | 0x10E8 | PopCylinder_ctor | +0x5420 | +0x2578, +0x5428 | No | YES | No |

---

## 13. Complete vtable[33] Mapping

See [Section 4](#4-factory-dispatch-via-vtable33) above for the full Race and Arena vtable[33] mapping tables.

---

## 14. Complete Board Slot → Mesh File Mapping

| Board Offset | Type | Description | Used By Levels |
|-------------|------|-------------|---------------|
| +0x2578 | AthenaList | Active game objects list (ALL objects) | All factories |
| +0x436C | MeshWorld* | Bridge/Tipper/Catapult/Spinny/Looper mesh (varies) | Dizzy, Tower, Expert, Master, Impossible |
| +0x4370 | CollisionLevel*/MeshWorld* | Collision for +0x436C / or secondary mesh | Dizzy, Tower, Expert, Master |
| +0x4374 | MeshWorld* | Gluebie (Dizzy) / 2PBridge (Master) / Fallout (Toob) | Dizzy, Master, Toob |
| +0x4378 | AthenaList*/MeshWorld* | Dizzy: AthenaList / Tower: Mace / Expert: Bridge / Toob: Blockdawg1 | Dizzy, Tower, Expert, Toob |
| +0x437C | CollisionLevel*/MeshWorld* | Dizzy: collision / Tower: Windmill / Expert: collision | Dizzy, Tower, Expert |
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
| +0x4784 | MeshWorld* | Up: Lifter mesh | Up |
| +0x4788 | MeshWorld* | Up: SpeedCylinder mesh | Up |
| +0x478C | MeshWorld* | Up: TimeButton mesh | Up |
| +0x4790 | AthenaList | Dizzy: secondary list | Dizzy |
| +0x4798 | AthenaList | Expert: Spinner bridge list 2 | Expert |
| +0x47D0 | AthenaList | Tower: list 2 | Tower |
| +0x47E0 | MeshWorld* | Spinny/Gear/Lifter mesh | Impossible, Spinny, Lifter |
| +0x47E4 | AthenaList | Tower Arena: list | Tower Arena |
| +0x4BA8 | MeshWorld* | Dizzy: WaterWheel mesh | Dizzy |
| +0x4BAC | CollisionLevel* | Dizzy: WaterWheel collision | Dizzy |
| +0x4BC4 | MeshWorld* | Dizzy: Swirl mesh | Dizzy |
| +0x4BC8 | CollisionLevel* | Dizzy: Swirl collision | Dizzy |
| +0x4BE8 | AthenaList | Tower: list 3 | Tower |
| +0x4FD4 | void* | Expert: BELL object pointer | Expert |
| +0x5000 | AthenaList | Tower: list 4 | Tower |
| +0x540C | void* | Bonk object pointer | Dizzy, Master |
| +0x5410 | MeshWorld* | BreakBridge1 mesh | Master |
| +0x5414 | MeshWorld* | BreakBridge2 mesh | Master |
| +0x5418 | void* | BreakBridge1 object pointer | Master |
| +0x541C | void* | BreakBridge2 object pointer | Master |
| +0x5420 | MeshWorld* | PopCylinder mesh | Master, Sky |
| +0x5424 | MeshWorld* | PopCylinder2 mesh (Master only) | Master |
| +0x5428 | AthenaList | PopCylinder list | Master |
| +0x5840 | MeshWorld* | BlockDawg1 mesh | Master, Toob |
| +0x5844 | MeshWorld* | BlockDawg2 mesh | Master, Toob |
| +0x5848 | MeshWorld* | Catapult mesh | Master |
| +0x584C | AthenaList | Catapult list | Master |
| +0x607C | MeshWorld* | Gluebie mesh | Master |
| +0x6080 | AthenaList | Gluebie list | Master |
| +0x868 | char* | Board display name string | All |
| +0x870 | int | Race ID from App+0x14+0x1DC | All |
| +0x878 | App* | App pointer | All |
| +0x8AC | Scene* | Scene pointer | All |
| +0x4344 | char* | Display/theme string | All |

---

## 15. Key Function Addresses

| Address | Function | Description |
|---------|----------|-------------|
| 0x40C430 | Scene_CreateDynamicObjects | Iterates MESHWORLD ref points, calls factory |
| 0x40C4BA | (call site) | vtable[33] dispatch call inside Scene_CreateDynamicObjects |
| 0x40C5D0 | N:/E: Handler | Handles N: and E: prefixed ref names |
| 0x4133E0 | BaseFactory | Base race factory (PLATFORM, STANDS) |
| 0x419750 | NoOpFactory | Warm-up/Beginner Arena factory (no-op) |
| 0x40A550 | CreateBeginnerObjects | Beginner Arena factory |
| 0x40A5F0 | CreateDizzyObjects | Dizzy Arena factory |
| 0x40D7C0 | CreateTowerObjects | Tower Arena factory |
| 0x4117B0 | CreateUpLevelObjects | Up Arena factory |
| 0x416910 | CreateNeonObjects | Neon Arena factory |
| 0x40E250 | CreateExpertLevelObjects | Expert Arena factory |
| 0x40EC40 | CreateOddObjects | Odd Arena factory |
| 0x40FB30 | CreateToobObjects | Toob Arena factory |
| 0x40F420 | CreateGlassObjects | Glass Arena factory |
| 0x40AD80 | CreateWobblyObjects | Wobbly Arena factory |
| 0x410AD0 | CreateSkyObjects | Sky Arena factory |
| 0x4121D0 | CreateLevelObjects | Master Arena factory (most inclusive) |
| 0x417FE0 | CreateMechanicalObjects | Impossible Arena factory |
| 0x4143D0 | CreateSpinny | Dizzy Race factory |
| 0x414680 | CreateTowerRaceObjects | Tower Race factory |
| 0x414A20 | CreateUpRaceObjects | Up Race factory |
| 0x414BD0 | CreateExpertRaceObjects | Expert Race factory |
| 0x4173B0 | CreateNeonRaceObjects | Neon Race factory |
| 0x415460 | CreateWobblyRaceObjects | Wobbly Race factory |
| 0x415A30 | CreateSkyRaceObjects | Sky Race factory |
| 0x418760 | Scene_CreateObject_Gear | Impossible Race factory |
| 0x466060 | Level_CloneTree | Clones a Level/MeshWorld object (thiscall) |
| 0x461510 | MeshWorld_ctor | Creates MeshWorld from file (alloc + parse) |
| 0x465080 | CollisionLevel_ctorWithLevel | Creates CollisionLevel from MeshWorld |
| 0x462850 | Stands_ctor | Base constructor — clones SpatialTree from parent |
| 0x453810 | AthenaList_Append | Append object to AthenaList |
| 0x453210 | AthenaList_Init | Initialize an AthenaList |
| 0x4BA57B | operator_new | Jump to malloc (0x10D0 = MeshWorld size) |
| 0x4C7677 | __strnicmp | Case-insensitive string compare |
| 0x4FD680 | App singleton | Global App object |
| 0x4FD8B4 | App+0x23C | Difficulty enum (0=Pipsqueak, 1=Normal, 2=Frenzied) |

---

## 16. Implementation Plan

### Phase 1: Build the Universal Factory Hook

1. Create a `bass.dll` proxy that patches `0x0040C4BA` with a JMP to the DLL's dispatch function
2. The dispatch function intercepts the factory call and tries all 13 Arena factories
3. For each factory attempt, save/restore board slots and inject meshes

### Phase 2: Just-In-Time Mesh Injection

1. Build a mesh database mapping all 46 ref names → mesh file paths + board slot offsets
2. At board constructor time, load all meshes from disk into a private array
3. During factory dispatch, inject the correct mesh into the board slot before calling the constructor

### Phase 3: Clone-on-Return for Static Mesh Objects

1. Detect Category 2 objects (WATERWHEEL, SWIRL, CHOMPER, WINDMILL, PILLAR, BRIDGE-base)
2. After the factory returns a static mesh pointer, call `Level_CloneTree` (0x466060) to clone it
3. Return the clone instead of the original

### Phase 4: Difficulty Gate Bypass

1. Detect difficulty-gated objects (TIPPER, BONK, BLOCKDAWG, GLUEBIE, FAN, SAWBLADE)
2. Before calling the factory, save `App+0x23C` and set it to 1
3. After the factory returns, restore `App+0x23C`

### Phase 5: Crash Test

1. Compile with MinGW: `i686-w64-mingw32-gcc -shared -o bass.dll mod.c -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc -Wl,--add-stdcall-alias`
2. Crash test via hbtestd or Wine/Xvfb: launch game, wait 35s, check if process alive
3. Copy to `mods/universal-ref-loader/` with README

---

## 17. Build & Test

### Build Command

```bash
i686-w64-mingw32-gcc -shared -o bass.dll universal_ref_loader.c \
    -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
    -Wl,--add-stdcall-alias
```

### Crash Test

```bash
# Via hbtestd MCP (preferred):
mcp_hbtestd_test_dll_mod(path="universal_ref_loader.zip", target_dll="bass.dll", timeout=10)

# Via Wine/Xvfb:
DISPLAY=:99 LIBGL_ALWAYS_SOFTWARE=1 timeout 35 wine Hamsterball.exe
# Check if process is alive after 35s (catches stack corruption, wrong hook addresses)
```

### Deliverables

- `mods/universal-ref-loader/bass.dll` — compiled DLL
- `mods/universal-ref-loader/universal_ref_loader.c` — source code
- `mods/universal-ref-loader/README.md` — documentation
- `mods/universal-ref-loader/hamsterball-universal-ref-loader.zip` — packaged mod
- Update `mods/README.md` catalog

---

## Appendix: Existing Related Files

| File | Description |
|------|-------------|
| `docs/REF_LOADING_SYSTEM.md` | Original ref-loading system documentation (needs update with clone findings) |
| `docs/VERIFIED_REFS_BY_LEVEL.md` | Verified 46 unique ref types from MESHWORLD binary parsing |
| `docs/objects/OBJECT_FACTORY_SYSTEM.md` | Factory system overview |
| `docs/objects/LEVEL_LOCKED_OBJECTS.md` | Level-locked objects and their board slot dependencies |
| `docs/objects/TOWER_OBJECT_ANALYSIS.md` | Dizzy/Tower factory analysis with spawnability verdicts |
| `docs/gameplay/ARENA_HAZARD_SYSTEM.md` | Arena hazard system (difficulty gating) |
| `analysis/factory_objects_comprehensive.md` | Comprehensive factory object analysis with all board offsets |
| `analysis/ghidra/decompilations/scene/Scene_CreateDynamicObjects_0040c430.c` | Full decompilation of the dispatch loop |
| `analysis/ghidra/decompilations/batch_auto/Level_CloneTree_0x00466060.c` | Full decompilation of the clone function |
| `analysis/ghidra/decompilations/batch_auto/Stands_ctor_0x00462850.c` | Stands_ctor (base constructor with SpatialTree clone) |
| `mods/universal-ref-loader/` | Existing universal ref loader mod (needs rebuild with new design) |
