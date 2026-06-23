# Hamsterball Catapult System — Complete Reverse Engineering

## Overview

Catapults are level objects that launch the ball when it touches a specific collision surface. They appear in Level4 (Dizzy Race) and the Dizzy Arena. The catapult mesh is loaded from `Levels/Level4-Catapult.MESHWORLD`, which contains a single entity name: `E:CATAPULTBOTTOM`.

**Why your global-spawn catapults are "hollow":** Catapults are NOT standalone objects — they require a `CollisionLevel` child object with a spatial tree, the mesh must be registered with the scene's collision system, the catapult must be added to TWO AthenaLists on the scene (general + catapult-specific), and the launch is triggered by the **collision dispatcher** (Level_HandleCollision / CreateSpinner) matching the collision entry's entity name. If you just `operator_new` + `Catapult_ctor` without going through `CreateLevelObjects`, you get a visual shell with no collision registration and no event-name association.

---

## Class Hierarchy

```
SpriteAnim (base)
  └─ Level (Level_ctor @ 0x00461740, size ~0x10D0)
       └─ Stands (Stands_ctor @ 0x00462850)
            └─ Catapult (Catapult_ctor @ 0x00437E10, alloc size = 0x1108 bytes)
```

### Vtable

The Catapult vtable is at `0x004D5AD0` (referenced as `PTR_MeshNode_Level_DeleteDtor3` in Ghidra). Key slots:

| Vtable Offset | Address | Function |
|---|---|---|
| +0x00 | 0x0043E5E0 | Gear_Vec3List_DeletingDtor (dtor) |
| +0x58 | — | SceneObj_CallUpdate (inherited, updates render matrix) |
| +0x54 | — | SceneObj_CallRender (inherited, renders mesh) |
| +0x2C (slot 11) | 0x0043E600 | **Catapult_Update** |
| +0x30 (slot 12) | 0x0045DE30 | LoadMeshWorld (inherited) |
| +0x58 (slot 22) | 0x0043EA50 | Catapult_Vec3List_DeletingDtor |
| +0x5C (slot 23) | 0x0043EA70 | **Catapult_Render** |

> Note: The vtable at `0x004D4F98` (set in Catapult_ctor) is the **construction vtable** used during C++ construction. The real runtime vtable at `0x004D5AD0` is set later by the `Stands_CtorCollision`/`CollisionLevel_ctorWithLevel` chain. The data refs at `0x004D5AFC` (Catapult_Update) and `0x004D5B5C` (Catapult_Render) confirm the runtime vtable.

---

## Catapult Struct Layout (size 0x1108 bytes)

All offsets are byte offsets from the catapult object pointer.

| Offset | Type | Field | Description |
|---|---|---|---|
| +0x000 | void** | vtable | Points to Catapult vtable |
| +0x008 | void* | mesh_ptr | MeshWorld/SceneObject mesh data (set by Level_ctor → SceneObject_BaseInit) |
| +0x00D | byte | has_mesh_flag | Set to 0 by Level_ctor |
| +0x018 | AthenaList | spatial_list | SpatialTree clone list (from Stands_ctor) |
| +0x430 | byte | flag_430 | Collision flag (copied from parent) |
| +0x431 | byte | flag_431 | Set to 1 in Stands_ctor (marks as "stands" type) |
| +0x434 | void* | timer_obj | Timer object (allocated in Stands_ctor) |
| +0x47C | void* | parent_level | Pointer back to the parent level/scene (set in Stands_ctor) |
| +0x488 | AthenaList | list_488 | Initialized in Level_ctor |
| +0x8A0 | AthenaList | list_8A0 | Initialized in Level_ctor |
| +0xCB8 | AthenaList | list_CB8 | Initialized in Level_ctor |
| +0x10D0 | void* | scene_ptr | **Pointer to the Board/Scene** (passed as param_1 to Catapult_ctor) |
| +0x10D4 | void* | collision_level | **CollisionLevel child object** (allocated 0x10D0 bytes, CollisionLevel_ctorWithLevel) |
| +0x10D8 | float[3] | position | **XYZ position** (set from param_4 in CreateLevelObjects) |
| +0x10E4 | int | pad | Set to 0 |
| +0x10E8 | float | launch_dir_y | Initial value = **-1.0** (0xBF800000) — negative Y = downward initial direction |
| +0x10EC | void* | launch_ball_ptr | **Ball to launch** (set by Level_HandleCollision when E:CATAPULTBOTTOM fires) |
| +0x10F0 | float | launch_timer | **Launch countdown timer** (set to 0 initially, set to 50.0 by Catapult_Launch, decremented each frame) |
| +0x10F4 | float | launch_decrement | Set to **50.0** (0x42480000) by Catapult_Launch — wait, that's wrong. |
| +0x10F8 | AthenaList | ball_list | **List of balls currently on the catapult** (AthenaList, initialized in ctor) |
| +0x10FC | int | ball_list_count | Count of balls in ball_list |
| +0x1100 | byte | active_flag | Set to **1** in CreateLevelObjects (marks catapult as active) |
| +0x1104 | float | launch_force | Initial value = **17.0** (0x41880000) — upward launch velocity |
| +0x1108 | AthenaList | object_list | Object list (used by Catapult_AddObjectConditional) |
| +0x110C | int | object_list_count | Count for object_list |
| +0x1510 | byte | conditional_flag | Checked by Catapult_AddObjectConditional (must be non-zero) |

> **Important**: The `0x1104` value of 17.0 is set in `Catapult_ctor` at `*(float*)(this + 0x1104) = 0x41880000`. This is the launch velocity applied to the ball.

---

## Creation Process

### Step 1: Level Loading

When a level is loaded (e.g., `Scene_LoadLevel4` @ 0x0040D6D0):

1. `MeshWorld_ctor` loads the main level geometry from `"levels\\level4"` → stored at `Scene+0x22B` (= scene+0x8AC offset, param_1[0x22b])
2. `CollisionLevel_ctorWithLevel` creates a collision level from the MeshWorld → stored at `Scene+0x22C`
3. `Level_InitScene` sets up the scene
4. The level's `.MESHWORLD` file is loaded by `Level_LoadMeshes` (@ 0x00465860)

### Step 2: Level_LoadMeshes — Entity Registration

`Level_LoadMeshes` (@ 0x00465860) iterates all objects in the MeshWorld:

1. For each object with an entity name (stored at `obj+0x864`):
   - Creates a `MeshBuffer` (0x874 bytes) via `CreateMeshBuffer`
   - If name starts with `"N:"` → sets `meshbuffer+0x85D = 1` (name flag)
   - If name starts with `"E:"` → sets `meshbuffer+0x85D = 1` AND `meshbuffer+0x863 = 1` (event flag)
   - Copies the entity name string into `meshbuffer+0x219*4` (= +0x864 offset)
   - Creates triangle collision data and appends to the meshbuffer's list

2. This is how `E:CATAPULTBOTTOM` gets registered — the MeshWorld file `Level4-Catapult.MESHWORLD` contains the string `E:CATAPULTBOTTOM` at offset 13580, and Level_LoadMeshes creates a MeshBuffer with that entity name.

### Step 3: CreateLevelObjects — Catapult Factory

`CreateLevelObjects` (@ 0x00412711) is the factory dispatcher. When a mesh name matches `"CATAPULT"` (via `__strnicmp`):

```c
// From CreateLevelObjects @ 0x00412711, CATAPULT section:
if (__strnicmp(param_1, "CATAPULT", 8) == 0) {
    // 1. Allocate 0x1108 bytes for the Catapult object
    pvVar5 = operator_new(0x1108);
    
    // 2. Call Catapult_ctor(this, scene_ptr, mesh_source)
    pvVar5 = Catapult_ctor(pvVar5, this, *(int*)((int)this + 0x5848));
    
    // 3. Mark as active
    *(byte*)((int)pvVar5 + 0x1100) = 1;
    
    // 4. Set position from param_4 (the mesh transform data)
    *(float*)((int)pvVar5 + 0x10D8) = param_4[1];  // X
    *(float*)((int)pvVar5 + 0x10DC) = param_4[2];  // Y  
    *(float*)((int)pvVar5 + 0x10E0) = param_4[3];  // Z
    
    // 5. CRITICAL: Add to scene's general object list
    AthenaList_Append((void*)((int)this + 0x2578), (int)pvVar5);
    
    // 6. CRITICAL: Add to scene's catapult-specific list
    AthenaList_Append((void*)((int)this + 0x584C), (int)pvVar5);
}
```

**This is the key step you're missing.** Two AthenaList_Append calls:
- `Scene+0x2578` — the general level objects list (for rendering/updating)
- `Scene+0x584C` — the **catapult-specific list** (used by the collision dispatcher to find catapults)

### Step 4: Catapult_ctor Internals

`Catapult_ctor` (@ 0x00437E10, `__thiscall`):

```c
void* Catapult_ctor(void* this, void* scene_ptr, int mesh_source) {
    // 1. Call parent: Stands_ctor(this, mesh_source)
    //    - Calls SpriteAnim_Ctor (base)
    //    - Calls Level_ctor → sets up AthenaLists, SceneObject
    //    - Clones SpatialTree nodes from mesh_source
    Stands_ctor(this, mesh_source);
    
    // 2. Set Catapult vtable
    *(void***)this = &PTR_MeshNode_Level_DeleteDtor3_004D4F98;
    
    // 3. Store scene pointer
    *(void**)((int)this + 0x10D0) = scene_ptr;
    
    // 4. Initialize position fields (will be overwritten by CreateLevelObjects)
    *(float*)((int)this + 0x10D8) = 0;  // X
    *(float*)((int)this + 0x10DC) = 0;  // Y
    *(float*)((int)this + 0x10E0) = 0;  // Z
    
    // 5. Clear pad and launch direction
    *(int*)((int)this + 0x10E4) = 0;
    *(float*)((int)this + 0x10E8) = -1.0f;  // Initial launch direction Y = -1
    
    // 6. CRITICAL: Create CollisionLevel child object
    void* col = operator_new(0x10D0);
    col = CollisionLevel_ctorWithLevel(col, (int)this);
    *(void**)((int)this + 0x10D4) = col;
    
    // 7. Copy timer from parent and clear collision flag
    *(void**)((int)col + 0x434) = *(void**)((int)this + 0x434);
    *(byte*)(*(int*)((int)this + 0x10D4) + 0x431) = 0;
    
    // 8. Clear launch flags
    *(byte*)((int)this + 0x10F0) = 0;  // launch_timer = 0 (not active)
    *(byte*)((int)this + 0x10F8) = 0;  // ball_list empty
    *(byte*)((int)this + 0x1100) = 0;  // active_flag = 0 (set to 1 later by CreateLevelObjects)
    
    // 9. Set launch force
    *(float*)((int)this + 0x1104) = 17.0f;  // 0x41880000
}
```

### Step 5: CollisionLevel_ctorWithLevel

`CollisionLevel_ctorWithLevel` (@ 0x00465080, `__thiscall`):

```c
void* CollisionLevel_ctorWithLevel(void* this, int parent_level) {
    // 1. Call Level_ctor with parent's mesh data
    Level_ctor(this, *(void**)(parent_level + 4));  // mesh source
    
    // 2. Set CollisionLevel vtable
    *(void***)this = &PTR_Level_DeletingDtor2_004D9068;
    
    // 3. CRITICAL: Load meshes (creates MeshBuffers with entity names)
    Level_LoadMeshes(this, parent_level);
    
    // This is where E:CATAPULTBOTTOM gets its collision triangles created
}
```

---

## Collision Detection & Launch Trigger

### How the Ball Hits the Catapult

The game's collision system (SpatialTree) detects when the ball intersects a collision triangle. When a collision entry is created, its entity name (at `collision_obj+0x864`) is checked by the **collision dispatcher**.

### Race Mode: Level_HandleCollision

`Level_HandleCollision` (@ 0x0040DCD0, `__thiscall`):

```c
void Level_HandleCollision(void* this, int* ball, int* coll_obj) {
    // Check entity name on the collision object
    if (__stricmp(*(char**)(coll_obj[1] + 0x864), "E:CATAPULTBOTTOM") == 0) {
        // Cooldown check: ball+0x202 prevents re-triggering
        if (ball[0x202] < 1) {
            ball[0x202] = 1000;  // Set cooldown (frames)
            
            // Iterate catapult list (Scene+0x43B8 in race mode)
            int idx = AthenaList_NextIndex((int)this + 0x43B8);
            for (each catapult in Scene+0x47C4 list) {
                // Match collision mesh to catapult
                if (catapult->collision_level (catapult+0x10D4) == *coll_obj) {
                    catapult->launch_ball_ptr (catapult+0x10EC) = ball;
                    Catapult_Launch(catapult);  // 0x00434290
                    Sound_PlayChannel(*(scene+0x878)->sounds[0x464]);  // catapult sound
                }
            }
        }
    }
    // ... other collision handlers (OPENSESAME, TRAPDOOR, BITE, MACETRIGGER, etc.)
    CreateNoDizzy(this, ball, coll_obj);
}
```

### Arena Mode: CreateSpinner

`CreateSpinner` (@ 0x00412D57) handles the same `E:CATAPULTBOTTOM` collision but uses **different scene offsets** for the catapult list:

```c
// In CreateSpinner, E:CATAPULTBOTTOM handler:
if (__stricmp(*(char**)(coll_obj[1] + 0x864), "E:CATAPULTBOTTOM") == 0) {
    if (ball[0x202] < 1) {
        ball[0x202] = 1000;
        
        // ARENA catapult list: Scene+0x584C (instead of Scene+0x43B8)
        // ARENA catapult items: Scene+0x5C58 (instead of Scene+0x47C4)
        int idx = AthenaList_NextIndex((int)this + 0x584C);
        for (each catapult in Scene+0x5C58 list) {
            if (catapult+0x10D4 == *coll_obj) {
                catapult+0x10EC = ball;
                Catapult_Launch(catapult);
                Sound_PlayChannel(*(scene+0x878)->sounds[0x464]);
            }
        }
    }
}
```

### Catapult_Launch

`Catapult_Launch` (@ 0x00434290, `__fastcall`):

```c
void Catapult_Launch(int catapult) {
    *(byte*)(catapult + 0x10F0) = 1;      // Set launch active flag
    *(float*)(catapult + 0x10F4) = 50.0f;  // Set launch timer (0x32 = 50 as int, 
                                            // but stored as float 0x42480000 = 50.0)
}
```

This is extremely simple — it just sets two fields:
- `+0x10F0 = 1` — marks the catapult as "launching"
- `+0x10F4 = 50.0` — countdown timer (decremented each frame by Catapult_Update)

---

## Catapult_Update — The Launch Physics

`Catapult_Update` (@ 0x0043E600, `__fastcall`) is called every frame via the vtable. It:

### 1. Timer Countdown
```c
// FLD [EBX+0x10F0]  (load launch_timer)
// FSUB [EBX+0x10F4]  (subtract decrement)
// FSTP [EBX+0x10F0]  (store result)
catapult->launch_timer -= catapult->launch_decrement;
```

This counts down the launch timer. When `launch_timer` reaches 0 or below, the launch is complete.

### 2. Ball Position Transformation

Catapult_Update iterates through the ball list (`catapult+0x10F8` AthenaList) and applies a **matrix transformation** to each ball:

```c
// For each ball in the catapult's ball_list:
//   1. Calculate ball position relative to catapult (ball_pos - catapult_pos)
//   2. Apply rotation matrix (built from launch_timer value)
//   3. Apply scaling (Gfx_ScaleY, Gfx_ScaleX, Gfx_ScaleZ)
//   4. Transform ball position using the matrix
//   5. Update ball position (ball+0x164/X, ball+0x168/Y, ball+0x16C/Z)
//   6. Transform ball velocity (ball+0x1A4 → physics_obj+0xCA4/CAC)
```

The key physics operations:
- **Ball position** is at `ball+0x164` (X), `ball+0x168` (Y), `ball+0x16C` (Z)
- **Ball velocity** is at `ball->physics_obj (ball+0x1A4) + 0xCA4` (velX), `+0xCA8` (velY), `+0xCAC` (velZ)
- The matrix is built using `Timer_Init` + `Gfx_Scale*` calls, which construct a transformation matrix
- The `launch_force` at `+0x1104` (17.0) determines the upward velocity

### 3. Object Aging (Time-Out)

Catapult_Update also manages objects in the ball_list with a countdown:
```c
// For each entry in the list:
//   entry[1] -= 1;  (decrement timer)
//   if (timer < 1) { remove entry; free memory; }
```

This means balls stay on the catapult list for a limited number of frames (initial value = 10, set by Catapult_AddObjectConditional).

---

## Catapult_AddObjectConditional

`Catapult_AddObjectConditional` (@ 0x0043E9C0, `__thiscall`):

```c
void Catapult_AddObjectConditional(void* this, int ball_ptr) {
    // Only proceed if catapult is active (flag at +0x1510)
    if (*(char*)((int)this + 0x1510) != '\0') {
        // Check if ball is already in the list
        for (each entry in ball_list (this+0x10F8)) {
            if (entry->ball == ball_ptr) {
                entry->timer = 10;  // Reset timer
                return;
            }
        }
        // Add new entry
        int* entry = operator_new(8);
        entry[0] = ball_ptr;   // ball pointer
        entry[1] = 10;         // frames to stay on list
        AthenaList_Append(this+0x10F8, entry);
    }
}
```

This is called from an unnamed function at ~0x004184D9 (likely a per-frame collision check that adds balls that are near/on the catapult to the ball list).

---

## Catapult_Render

`Catapult_Render` (@ 0x0043EA70, `__fastcall`):

```c
// 1. Calculate oscillation using Wave_Sin
float wave = Wave_Sin(PTR_PTR_004F7188, catapult->render_time (+0x53F));
Gfx_ScaleZ(wave * _DAT_004D5D20);

// 2. Set position
Gfx_SetPosition(catapult_pos_x, catapult_pos_y, catapult_pos_z);

// 3. Update render time
catapult->render_time (+0x53F) += catapult->render_speed (+0x540);

// 4. Call inherited render (vtable+0x58 = CallUpdate, vtable+0x54 = CallRender)
(**(code**)(*catapult + 0x58))();  // SceneObj_CallUpdate
(**(code**)(*catapult + 0x54))();  // SceneObj_CallRender

// 5. Age and free expired objects from the render list
```

The `Wave_Sin` call creates the visual oscillation/animation of the catapult platform.

---

## MESHWORLD File: Level4-Catapult.MESHWORLD

- **File**: `Levels/Level4-Catapult.MESHWORLD` (14,370 bytes)
- **Entity name**: `E:CATAPULTBOTTOM` (at file offset 13,580)
- **This is the ONLY entity name in the file** — no `N:` names, no other `E:` events
- The mesh name "CATAPULT" is what `CreateLevelObjects` matches with `__strnicmp`

The catapult's collision geometry (the "bottom" surface the ball touches) is tagged with `E:CATAPULTBOTTOM`. When the SpatialTree detects a collision between the ball and any triangle belonging to this mesh, the collision entry's `+0x864` field points to the string `"E:CATAPULTBOTTOM"`, which the collision dispatcher matches.

---

## Scene List Offsets Summary

| Context | General Objects List | Catapult List | Catapult Items Array |
|---|---|---|---|
| **Race (Level_HandleCollision)** | Scene+0x2578 | Scene+0x43B8 | Scene+0x47C4 |
| **Arena (CreateSpinner)** | Scene+0x2578 | Scene+0x584C | Scene+0x5C58 |

> Note: In race mode, `Scene+0x43B8` is the catapult AthenaList and `Scene+0x47C4` is its internal items array. In arena mode, `Scene+0x584C` and `Scene+0x5C58` serve the same purpose. The CreateLevelObjects factory adds to `Scene+0x584C` for both modes — the arena offsets appear to be the primary ones, and Level_HandleCollision uses different offsets that may be set up during race initialization.

---

## Why Your Global Spawns Are Hollow — And How to Fix Them

### What you're probably doing:
```c
// WRONG: Just creating the object
void* cat = operator_new(0x1108);
Catapult_ctor(cat, scene, mesh_source);
// ... set position ...
// Catapult renders but has NO collision and NO launch behavior
```

### What you need to do:
```c
// CORRECT: Full creation pipeline

// 1. Allocate and construct
void* cat = operator_new(0x1108);
Catapult_ctor(cat, scene, mesh_source);

// 2. Mark as active
*(byte*)((int)cat + 0x1100) = 1;

// 3. Set position
*(float*)((int)cat + 0x10D8) = x;
*(float*)((int)cat + 0x10DC) = y;
*(float*)((int)cat + 0x10E0) = z;

// 4. CRITICAL: Add to scene's general object list (for rendering/updating)
AthenaList_Append((void*)((int)scene + 0x2578), (int)cat);

// 5. CRITICAL: Add to scene's catapult-specific list (for collision dispatch)
//    Use Arena offsets (0x584C / 0x5C58) — these are what CreateLevelObjects uses
AthenaList_Append((void*)((int)scene + 0x584C), (int)cat);

// 6. The CollisionLevel child (cat+0x10D4) was created by Catapult_ctor
//    and Level_LoadMeshes registered the E:CATAPULTBOTTOM entity name.
//    BUT: the collision mesh must be registered with the scene's SpatialTree
//    for the ball to actually collide with it.
```

### The Missing Piece: Collision Registration

The `CollisionLevel` at `cat+0x10D4` has its own SpatialTree (cloned from the parent in `Stands_ctor`). However, for the ball's collision system to detect intersections with the catapult's mesh, the catapult's collision triangles must be registered in the scene's main SpatialTree.

In the normal level loading flow, this happens because:
1. `Level_LoadMeshes` creates MeshBuffers with entity names and collision triangles
2. The SpatialTree in the CollisionLevel stores these triangles
3. When `Level_HandleCollision` / `CreateSpinner` checks `catapult+0x10D4 == *coll_obj`, it's matching the collision object that was created from the CollisionLevel's mesh data

**For a global spawn**, you need to ensure:
1. The CollisionLevel's mesh is loaded (Catapult_ctor does this via `CollisionLevel_ctorWithLevel`)
2. The collision triangles are registered with the scene's SpatialTree
3. The catapult is added to the correct scene list so the collision dispatcher can find it

The simplest approach: call `CreateLevelObjects` with a mesh named "CATAPULT" — this does everything correctly. If you want to spawn at runtime, replicate all the steps that `CreateLevelObjects` does (allocation, construction, position set, **both** AthenaList_Append calls).

---

## Related Functions Reference

| Address | Name | Description |
|---|---|---|
| 0x00437E10 | Catapult_ctor | Constructor (thiscall: this, scene, mesh_source) |
| 0x0043E600 | Catapult_Update | Per-frame update: timer countdown + ball physics |
| 0x0043EA70 | Catapult_Render | Per-frame render: oscillation + mesh render |
| 0x00434290 | Catapult_Launch | Sets launch flag + timer (called on E:CATAPULTBOTTOM) |
| 0x0043E9C0 | Catapult_AddObjectConditional | Adds ball to catapult's ball_list |
| 0x0043EA50 | Catapult_Vec3List_DeletingDtor | Destructor |
| 0x00437820 | Catapult_Vec3List_Level_Dtor | Level destructor |
| 0x00412711 | CreateLevelObjects | Factory: matches "CATAPULT" mesh name |
| 0x0040DCD0 | Level_HandleCollision | Race collision dispatcher (E:CATAPULTBOTTOM) |
| 0x00412D57 | CreateSpinner | Arena collision dispatcher (E:CATAPULTBOTTOM) |
| 0x00462850 | Stands_ctor | Parent constructor |
| 0x00465080 | CollisionLevel_ctorWithLevel | Creates CollisionLevel + loads meshes |
| 0x00461740 | Level_ctor | Grandparent constructor |
| 0x00465860 | Level_LoadMeshes | Loads meshes + registers entity names |
| 0x00461460 | SceneObject_BaseInit | Initializes SceneObject mesh data |
| 0x0045DF80 | SceneObject_CallUpdate | Inherited vtable: update render matrix |
| 0x0045DF90 | SceneObject_CallRender | Inherited vtable: render mesh |

---

## String References

| Address | String | Usage |
|---|---|---|
| 0x004CF99C | `"CATAPULT"` | Mesh name matched by CreateLevelObjects |
| 0x004CF9A4 | `"E:CATAPULTBOTTOM"` | Collision event name (stricmp in dispatchers) |
| 0x004D09C6 | `"Levels\\Level4-Catapult"` | MESHWORLD file path |
| 0x004D2DEF | `"sounds\\catapult"` | Sound file path |
