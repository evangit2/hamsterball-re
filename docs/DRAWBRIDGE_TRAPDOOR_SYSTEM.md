# Drawbridge & Trapdoor: Complete Reverse Engineering

## Overview

Both objects are spawned by `CreateTowerObjects` (factory @ `0x0040D7C0`) in Tower Race.
They can be spawned globally using a CEA hotkey, but require different setup.

---

## 1. DRAWBRIDGE (Glass_Level)

### Factory Spawn Sequence
```
; At 0x40D956-0x40D9DF in CreateTowerObjects
push 0x113C                ; alloc size = 4412 bytes
call operator_new (0x4BA57B)
add esp, 4
; if alloc succeeded:
mov edx, [esi+0x4370]      ; mesh = Board+0x4370 (Drawbridge MeshWorld)
push edx                   ; param_2 = mesh_ptr
push esi                   ; param_1 = Board
mov ecx, eax               ; this = alloc
call Glass_Level_ctor (0x4384A0)  ; ret 8
; Copy position from param_block+4/+8/+0xC to obj+0x10D8 (3 floats)
push edi                   ; obj
lea ecx, [esi+0x2578]
call AthenaList_Append (0x453810)  ; Board+0x2578 (general list)
push edi                   ; obj
lea ecx, [esi+0x4BE8]
call AthenaList_Append (0x453810)  ; Board+0x4BE8 (drawbridge list)
```

### Constructor: Glass_Level_ctor @ 0x4384A0
**Calling convention:** `__thiscall` — `ecx=alloc`, push Board, push mesh_ptr → `ret 8`

**Steps:**
1. Calls `SceneObject_ctor(this, mesh_ptr)` @ 0x462850 — base class init
   - Sets vtable to 0x4D8FB0 (Stands base)
   - Inits AthenaLists at +0x18, +0x488, +0x8A0, +0xCB8
   - Copies mesh data (vertices, spatial tree) from mesh_ptr
   - Creates Timer (0x44 bytes) at +0x434
2. Sets vtable to `0x4D5060` (Glass_Level/Impossible_Level vtable)
3. Stores Board at +0x10D0
4. Zeroes position at +0x10D8/+0x10DC/+0x10E0
5. Sets +0x10E4 = 0 (rotation/state)
6. Sets +0x10E8 = -1.0 (0xBF800000, break direction)
7. Allocates CollisionLevel (0x10D0 bytes) → calls `CollisionLevel_ctorWithLevel(alloc, this)` @ 0x465080
   - Stores result at +0x10D4
   - Links Timer from this+0x434 into CollisionLevel+0x434
8. Creates Timer array (2 × 0x44 bytes) via `_eh_vector_constructor_iterator`
9. Allocates TimerArray (0x90 bytes) via `0x475980` → stores at +0x10EC
10. Looks up chain bridge collision strings:
    - "Chain1Bridge" @ 0x4D5BB8 → stores at obj+0x10F0
    - "Chain2Bridge" @ 0x4D5BA8 → stores at obj+0x10FC
    - "Chain1Wall" @ 0x4D5B9C → stores at obj+0x1120
    - "Chain2Wall" @ 0x4D5B90 → stores at obj+0x112C

### Struct Layout (0x113C bytes)
| Offset | Size | Description |
|--------|------|-------------|
| +0x000 | 4 | vtable ptr (0x4D5060) |
| +0x004 | 4 | mesh data ptr (from SceneObject_ctor) |
| +0x018 | 0x420 | AthenaList (inline objects) |
| +0x438 | 0x50 | AthenaList |
| +0x434 | 4 | Timer ptr (0x44 bytes, owned) |
| +0x480 | 4 | mesh field (from SceneObject_ctor) |
| +0x488 | 0x418 | AthenaList |
| +0x8A0 | 0x418 | AthenaList |
| +0xCB8 | 0x418 | AthenaList |
| +0x10D0 | 4 | Board ptr (parent scene) |
| +0x10D4 | 4 | CollisionLevel ptr (owned, 0x10D0 bytes) |
| +0x10D8 | 4 | position X |
| +0x10DC | 4 | position Y |
| +0x10E0 | 4 | position Z |
| +0x10E4 | 4 | rotation/state (0) |
| +0x10E8 | 4 | break direction (-1.0 = 0xBF800000) |
| +0x10EC | 4 | TimerArray ptr (owned, 0x90 bytes) |
| +0x10F0 | 12 | Chain1Bridge position (Vec3) |
| +0x10FC | 12 | Chain2Bridge position (Vec3) |
| +0x1108 | 1 | flag (set to 1 in ctor) |
| +0x1120 | 12 | Chain1Wall position (Vec3) |
| +0x112C | 12 | Chain2Wall position (Vec3) |
| +0x1138 | 4 | state flag |

### Vtable: 0x4D5060
| Index | Address | Function |
|-------|---------|----------|
| [0] | 0x438730 | scalar_deleting_dtor |
| [9] | 0x45DFD0 | SceneObject_CallUpdate wrapper |
| [18] | 0x43F2F0 | Update (calls base + chain animation) |
| [21] | 0x45DF90 | SceneObject_CallRender (tail-call) |
| [22] | 0x45DF80 | SceneObject_CallTimerUpdate (tail-call) |
| [24] | 0x438830 | scalar_deleting_dtor (alt) |

### Collision
- No dedicated handler in `TowerCollisionEvents` (0x40DCD0)
- Collision is via the CollisionLevel sub-object at obj+0x10D4
- CollisionLevel has its own spatial tree built from the Drawbridge mesh
- Chain bridge/wall names are looked up in the mesh's entity table for collision triggers

### Mesh Dependency
- **Board+0x4370** = MeshWorld("Levels\Level4-Drawbridge")
- Loaded by `LevelBoard_Tower_ctor` (0x41E340) at line 72
- On non-Tower levels: must load manually via `MeshWorld_ctor(0x10D0, App+0x174, "Levels\\Level4-Drawbridge")`

### AthenaList Dependencies
- **Board+0x2578** (general objects) — initialized by Board_ctor on ALL levels ✓
- **Board+0x4BE8** (drawbridge list) — initialized by LevelBoard_Tower_ctor ONLY

---

## 2. TRAPDOOR (GlassStands)

### Factory Spawn Sequence
```
; At 0x40DA82-0x40DB5C in CreateTowerObjects
push 0x10F8                ; alloc size = 4344 bytes
call operator_new (0x4BA57B)
add esp, 4
; if alloc succeeded:
push esi                   ; param_1 = Board
mov ecx, eax               ; this = alloc
call GlassStands_Ctor (0x438290)  ; ret 4
; Copy position from param_block+4/+8/+0xC to obj+0x10E0 (3 floats)
push edi                   ; obj
lea ecx, [esi+0x2578]
call AthenaList_Append (0x453810)  ; Board+0x2578 (general list)
push edi                   ; obj
lea ecx, [esi+0x47D0]
call AthenaList_Append (0x453810)  ; Board+0x47D0 (trapdoor list)
; Append sub-objects to collision lists:
mov ecx, [edi+0x10D8]     ; sub-object 1 (Stands)
push ecx
lea ecx, [esi+0xCD4]
call AthenaList_Append     ; Board+0xCD4
mov edx, [edi+0x10DC]     ; sub-object 2 (TipperVisual)
push edx
lea ecx, [esi+0x10EC]
call AthenaList_Append     ; Board+0x10EC
; Append to collision levels:
mov ecx, [esi+0x8AC]      ; Board+0x8AC (primary CollisionLevel)
mov eax, [edi+0x10D8]     ; sub-object 1
mov ecx, [ecx+0x480]      ; CollisionLevel+0x480
push eax
add ecx, 0x1C             ; AthenaList at CollisionLevel+0x480+0x1C
call AthenaList_Append
mov eax, [edi+0x10DC]     ; sub-object 2
mov ecx, [esi+0x8B0]      ; Board+0x8B0 (secondary CollisionLevel)
push eax
add ecx, 0x18             ; AthenaList at CollisionLevel+0x18
call AthenaList_Append
```

### Constructor: GlassStands_Ctor @ 0x438290
**Calling convention:** `__thiscall` — `ecx=alloc`, push Board → `ret 4`

**Steps:**
1. Reads App from Board+0x878
2. Calls `SceneObject_ctor(this, App+0x594)` @ 0x462850 — base class init with Trapdoor1 mesh
   - App+0x594 = MeshWorld("Levels\Level4-Trapdoor1") — globally pre-loaded!
3. Sets vtable to `0x4D4FF8` (GlassStands vtable)
4. Stores Board at +0x10D0
5. Allocates TipperVisual (0x10D0 bytes) using App+0x59C (CollisionLevel of Trapdoor1)
   - Calls `0x4661A0` (TipperVisual_ctor) with App+0x59C as mesh
   - Stores at +0x10D4
   - Calls `Level_LoadMeshes(+0x10D4, this)` @ 0x465200
6. Allocates Stands (0x10D0 bytes) using App+0x598 (MeshWorld of Trapdoor2)
   - Calls `SceneObject_ctor(alloc, App+0x598)` @ 0x462850
   - Stores at +0x10D8
7. Allocates TipperVisual (0x10D0 bytes) using App+0x5A0 (CollisionLevel of Trapdoor2)
   - Calls `0x4661A0` (TipperVisual_ctor) with App+0x5A0 as mesh
   - Stores at +0x10DC
   - Calls `Level_LoadMeshes(+0x10DC, +0x10D8)` @ 0x465200
8. Sets +0x10EC = 0 (timer/state)
9. Sets +0x10F0 = -1.0 (0xBF800000, break direction)
10. Sets +0x10F4 = 0 (triggered flag)

### Struct Layout (0x10F8 bytes)
| Offset | Size | Description |
|--------|------|-------------|
| +0x000 | 4 | vtable ptr (0x4D4FF8) |
| +0x004 | 4 | mesh data ptr (from SceneObject_ctor) |
| +0x018 | 0x420 | AthenaList (inline objects) |
| +0x434 | 4 | Timer ptr (0x44 bytes, owned) |
| +0x480 | 4 | mesh field |
| +0x488 | 0x418 | AthenaList |
| +0x8A0 | 0x418 | AthenaList |
| +0xCB8 | 0x418 | AthenaList |
| +0x10D0 | 4 | Board ptr (parent scene) |
| +0x10D4 | 4 | TipperVisual ptr (Trapdoor1 collision, owned) |
| +0x10D8 | 4 | Stands ptr (Trapdoor2 mesh, owned) |
| +0x10DC | 4 | TipperVisual ptr (Trapdoor2 collision, owned) |
| +0x10E0 | 4 | position X |
| +0x10E4 | 4 | position Y |
| +0x10E8 | 4 | position Z |
| +0x10EC | 4 | timer/state (0) |
| +0x10F0 | 4 | break direction (-1.0) |
| +0x10F4 | 4 | triggered flag (0 = closed, 10 = opening) |

### Vtable: 0x4D4FF8
| Index | Address | Function |
|-------|---------|----------|
| [0] | 0x4383F0 | scalar_deleting_dtor |
| [9] | 0x45DFD0 | SceneObject_CallUpdate wrapper |
| [11] | 0x4342C0 | GlassStands_Dtor (cleanup) |
| [18] | 0x45E0E0 | Update (Stands_Update base) |
| [21] | 0x45DF90 | SceneObject_CallRender (tail-call) |
| [22] | 0x45DF80 | SceneObject_CallTimerUpdate (tail-call) |
| [24] | 0x423E0000 | (float data, NOT a function — vtable ends at [23]) |

Note: vtable at 0x4D4FF8 has only 24 entries (0x60 bytes). The values at [24]=0x423E0000
and [25]=0x42A00000 are float constants (40.0 and 80.0), not function pointers.
The next vtable (Glass_Level at 0x4D5060) starts at [26].

### Collision
Handled in `TowerCollisionEvents` (0x40DCD0):

1. **"N:TRAPDOOR"** → `Trapdoor_Activate(obj)` @ 0x438410
   - Iterates Board+0x47D0 (trapdoor list) via Board+0x4BDC (heap array)
   - Matches by obj+0x10D4 or obj+0x10DC == colliding object
   - Sets obj+0x10F4 = 10 (activation timer)
   - Plays 3D sound at obj+0x10E0/+0x10E4/+0x10E8

2. **"E:OPENSESAME"** → `Trapdoor_Open(obj)` @ 0x4344D0
   - Iterates Board+0x4BE8 (drawbridge list) via Board+0x4FF4 (heap array)
   - Opens the first trapdoor in the list

### Trapdoor_Activate @ 0x438410
```
if (obj+0x10F4 == 0):           // only if not already activated
    if (obj+0x10EC < threshold): // timer check
        play 3D sound at position
    obj+0x10F4 = 10             // set activation timer
```

### Trapdoor_Open @ 0x4344D0
```
if (obj+0x10E4 == threshold):
    obj+0x10E4 = 1.0  (0x3F800000)
```

### Mesh Dependency
**NONE!** Trapdoor uses App-level meshes that are globally pre-loaded:
- App+0x594 = MeshWorld("Levels\Level4-Trapdoor1") — loaded by resource loader (0x4298C0)
- App+0x598 = MeshWorld("Levels\Level4-Trapdoor2")
- App+0x59C = CollisionLevel(App+0x594)
- App+0x5A0 = CollisionLevel(App+0x598)

### AthenaList Dependencies
- **Board+0x2578** (general) — initialized by Board_ctor ✓
- **Board+0x47D0** (trapdoor) — initialized by LevelBoard_Tower_ctor ONLY
- **Board+0xCD4** — initialized by Board_ctor ✓ (or BoardLevel5)
- **Board+0x10EC** — initialized by Board_ctor ✓ (or BoardLevel5)
- **Board+0x8AC→+0x480→+0x1C** — collision list, exists on all levels
- **Board+0x8B0→+0x18** — collision list, exists on all levels

---

## Global Spawn Requirements

### Drawbridge
1. Load mesh: `MeshWorld_ctor(0x10D0, App+0x174, "Levels\\Level4-Drawbridge")` → Board+0x4370
2. Init AthenaList: `AthenaList_Init(Board+0x4BE8, 0)`
3. Allocate: `operator_new(0x113C)`
4. Construct: `Glass_Level_ctor(alloc, Board, Board+0x4370)` (thiscall, ret 8)
5. Set position: `obj+0x10D8/+0x10DC/+0x10E0 = X/Y/Z`
6. Append: `AthenaList_Append(Board+0x2578, obj)`
7. Append: `AthenaList_Append(Board+0x4BE8, obj)`

### Trapdoor
1. **No mesh loading needed** (App+0x594/+0x598/+0x59C/+0x5A0 pre-loaded)
2. Init AthenaList: `AthenaList_Init(Board+0x47D0, 0)`
3. Allocate: `operator_new(0x10F8)`
4. Construct: `GlassStands_Ctor(alloc, Board)` (thiscall, ret 4)
5. Set position: `obj+0x10E0/+0x10E4/+0x10E8 = X/Y/Z`
6. Append: `AthenaList_Append(Board+0x2578, obj)`
7. Append: `AthenaList_Append(Board+0x47D0, obj)`
8. Append: `AthenaList_Append(Board+0xCD4, obj+0x10D8)` (sub-object 1)
9. Append: `AthenaList_Append(Board+0x10EC, obj+0x10DC)` (sub-object 2)
10. Append: `AthenaList_Append(Board+0x8AC→+0x480→+0x1C, obj+0x10D8)` (collision)
11. Append: `AthenaList_Append(Board+0x8B0→+0x18, obj+0x10DC)` (collision)

---

## Key Function Addresses

| Address | Function | Calling Convention |
|---------|----------|-------------------|
| 0x4BA57B | operator_new | push size, call, add esp 4 |
| 0x4BA74D | operator_delete (free) | push ptr, call, add esp 4 |
| 0x453810 | AthenaList_Append | ecx=list, push obj, call (ret 4) |
| 0x453210 | AthenaList_Init | ecx=list, push 0, call (ret 4) |
| 0x461510 | MeshWorld_ctor | ecx=alloc, push App+0x174, push path, call (ret 8) |
| 0x462850 | SceneObject_ctor | ecx=alloc, push mesh_ptr, call (ret 4) |
| 0x465080 | CollisionLevel_ctorWithLevel | ecx=alloc, push source_level, call (ret 4) |
| 0x465200 | Level_LoadMeshes | ecx=dest, push source, call (ret 4) |
| 0x4661A0 | TipperVisual_ctor | ecx=alloc, push mesh_ptr, call (ret 4) |
| 0x4384A0 | Glass_Level_ctor (Drawbridge) | ecx=alloc, push Board, push mesh, call (ret 8) |
| 0x438290 | GlassStands_Ctor (Trapdoor) | ecx=alloc, push Board, call (ret 4) |
| 0x438410 | Trapdoor_Activate | ecx=trapdoor_obj, call |
| 0x4344D0 | Trapdoor_Open | ecx=trapdoor_obj, call |
| 0x40DCD0 | TowerCollisionEvents | thiscall(Board, ball, collObj) |
| 0x40D7C0 | CreateTowerObjects | thiscall, processes MESHWORLD objects |
| 0x41E340 | LevelBoard_Tower_ctor | thiscall, loads Tower meshes + inits lists |

## Mesh Path Strings (in .rdata)

| Address | String | Board Offset | Used By |
|---------|--------|-------------|---------|
| 0x4D099C | Levels\Level4-Drawbridge | +0x4370 | Drawbridge |
| (global) | Levels\Level4-Trapdoor1 | App+0x594 | Trapdoor |
| (global) | Levels\Level4-Trapdoor2 | App+0x598 | Trapdoor |

## App-Level Resource Loading (0x4298C0)

All trapdoor meshes are loaded GLOBALLY for every level:
```
App+0x594 = MeshWorld("Levels\\Level4-Trapdoor1")     // mesh
App+0x598 = MeshWorld("Levels\\Level4-Trapdoor2")     // mesh
App+0x59C = CollisionLevel(App+0x594)                   // collision from trapdoor1
App+0x5A0 = CollisionLevel(App+0x598)                   // collision from trapdoor2
```

Drawbridge mesh is Tower-specific, loaded only by LevelBoard_Tower_ctor.
