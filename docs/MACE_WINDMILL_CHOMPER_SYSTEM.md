# Mace, Windmill & Chomper System (Tower Race)

Complete reverse-engineering analysis of the Mace (Pendulum), Windmill, and Chomper systems in the Tower Race.

---

## Overview

The Tower Race features three notable mechanical objects:

1. **MACE** (Pendulum) — Swinging mace on a chain that bounces balls
2. **WINDMILL** — Rotating collision level (pushes balls away)
3. **CHOMPER** — Visual-only mesh, NOT a separate game object. The actual object is the TURRET.

All three are created inline in `Scene_LoadLevel4` (0x40D6D0), not via a standalone factory function.

---

## MACE (Pendulum / Swinging Mace)

### Binary Addresses

| Function | Address | Calling Convention | Purpose |
|----------|---------|-------------------|---------|
| CascadeStands_Ctor | 0x438750 | thiscall(obj, board, mesh) ret 8 | Constructor — calls Stands_ctor + allocates CollisionLevel |
| Pendulum_Update | 0x43F3C0 | thiscall(obj) | vtable[11] — swing animation + collision |
| Pendulum_Render | 0x45E0E0 | thiscall(obj) | vtable[18] — shared base render |
| Pendulum_scalar_dtor | 0x438830 | thiscall(obj) | vtable[0] — destructor |
| Stands_ctor | 0x462850 | thiscall(obj, mesh) ret 4 | Base class constructor — sets vtable 0x4D8FB0 |
| SceneObject_SpawnWithSound | 0x4536A0 | — | Spawns scene object with sound |
| Level_FindObjectByName | 0x4C7677 | cdecl ret 12 | Finds mesh ref by name in loaded level |

### Object Structure (0x110C bytes = 4364)

| Offset | Size | Type | Description |
|--------|------|------|-------------|
| +0x0000 | 4 | ptr | Vtable (0x4D50C0) |
| +0x10D0 | 4 | ptr | Board pointer |
| +0x10D4 | 4 | float | Home position X (from mesh) |
| +0x10D8 | 4 | float | Current position X |
| +0x10DC | 4 | float | Current position Y |
| +0x10E0 | 4 | float | Current position Z |
| +0x10E8 | 4 | float | Swing amplitude (80.0) — oscillation radius |
| +0x10EC | 4 | — | Unused |
| +0x10F0 | 4 | int | Activation flag (0=idle, 1=triggered by E:MACETRIGGER) |
| +0x10F4 | 4 | int | Active flag (1 = active) |
| +0x10F8 | 4 | int | Timer (init 50) |
| +0x10FC | 4 | ptr | CollisionLevel pointer (allocated by CascadeStands_Ctor) |
| +0x1100+ | — | — | CollisionLevel data (0x10D0 bytes) |

### Vtable (0x4D50C0)

| Index | Offset | Address | Function |
|-------|--------|---------|----------|
| 0 | 0x00 | 0x438830 | scalar_dtor |
| 11 | 0x2C | 0x43F3C0 | Pendulum_Update (swing + collision) |
| 18 | 0x48 | 0x45E0E0 | Render (shared base) |

### Creation Flow

**Inline in Scene_LoadLevel4 (0x40D6D0):**

```c
// 1. Find "MACE" mesh ref in loaded level
mesh_ref = Level_FindObjectByName("MACE");
if (mesh_ref == NULL) skip;

// 2. Allocate
obj = operator_new(0x110C);

// 3. Construct
// CascadeStands_Ctor(this=obj, board, *(board+0x4378))
//   → Stands_ctor(obj, mesh) → sets vtable = 0x4D50C0
//   → operator_new(0x10D0) → CollisionLevel_ctorWithLevel(coll, obj)
mesh_ptr = *(board + 0x4378);   // pre-loaded Level4-Mace mesh
CascadeStands_Ctor(obj, board, mesh_ptr);  // thiscall, ret 8

// 4. Copy position from mesh ref to obj+0x10D8/+0x10DC/+0x10E0
obj->pos = mesh_ref->pos;

// 5. Register to lists
AthenaList_Append(board+0x2578, obj);   // general objects
AthenaList_Append(board+0x5000, obj);   // mace list (for E:MACETRIGGER)

// 6. Set fields
obj->swing_amplitude = 80.0;  // +0x10E8
obj->active = 1;               // +0x10F4
obj->timer = 50;              // +0x10F8
```

### Update Function (vtable[11] @ 0x43F3C0)

```c
void Pendulum_Update(this) {
    // Base update (collision tree check)
    call 0x4605E0;  // Level_Update base
    call 0x457AD0;  // Timer_Init
    call 0x457C60;  // Timer_Cleanup
    
    call 0x40A0B0;  // scene operation
    
    // Load position for collision
    FLD [this+0x10D8]  // pos X
    FLD [this+0x10DC]  // pos Y
    FLD [this+0x10E0]  // pos Z
    
    call 0x457A40;  // timer operation
    
    // Swing animation
    FLD [this+0x10E8]          // load amplitude (80.0)
    MOV [this+0x10E8], 0.05    // temporarily set to 0.05 (angle increment)
    FLD [this+0x10E8]          // load increment
    
    // Trig calculation (sin/cos for oscillation)
    FLD [this+0x10E0]          // pos Z
    FLD [this+0x10DC]          // pos Y
    call 0x459860;             // trig function
    
    MOV [this+0x10E8], 80.0    // restore amplitude
    
    call 0x43F559;             // collision check sub-function
    RET;
}
```

The pendulum oscillates around its position. The 80.0 amplitude defines the swing radius, and 0.05 is the angle increment per frame (about 2.86°/frame ≈ 71.5°/sec at 25fps).

### Collision Events

Handled in `Level_HandleCollision` (0x40DCD0):

```c
// E:MACETRIGGER — activates all maces in board+0x5000
if (stricmp(eventName, "E:MACETRIGGER") == 0) {
    for each mace in board+0x5000:
        mace->activation_flag = 1;  // +0x10F0
}

// N:MACE — ball touches mace, bounce away
if (stricmp(eventName, "N:MACE") == 0) {
    for each mace in board+0x5000:
        if (mace->collides_with_ball && boundary_check)
            ball->vtable[8]();  // bounce/split
}
```

### Mesh Dependency

Pre-loaded by `BoardLevel5_Tower_ctor` (0x41E340):
```c
board+0x4378 = MeshWorld_ctor("Levels\\Level4-Mace");  // VA 0x4D0974
```

For global spawn: load mesh via `MeshWorld_ctor` with JIT mesh injection at `board+0x4378`.

---

## WINDMILL

### Binary Addresses

| Function | Address | Purpose |
|----------|---------|---------|
| CollisionLevel_ctorWithLevel | 0x465080 | Creates collision level from mesh |
| Level_LoadMeshes | 0x465200 | Loads mesh vertex/index data |
| SceneObject_SetupCallback | 0x45DD60 | Registers collision with scene manager (stdcall, ret 8) |

### Object Structure

The Windmill is NOT a standalone game object. It creates:

1. **CollisionLevel** (0x10D0 bytes) — collision-only, no vtable/update/render
2. **Trapdoor** (0x10F8 bytes) — separate object via GlassStands_Ctor

### Creation Flow

```c
// 1. Find "WINDMILL" mesh ref
mesh_ref = Level_FindObjectByName("WINDMILL");
if (mesh_ref == NULL) skip;

// 2. Get mesh from board+0x437C (pre-loaded by Tower ctor)
mesh_ptr = *(board + 0x437C);  // Level4-Windmill

// 3. Create CollisionLevel
coll = operator_new(0x10D0);
CollisionLevel_ctorWithLevel(coll, mesh_ptr);  // thiscall, ret 4

// 4. Load mesh data
Level_LoadMeshes(coll);

// 5. Copy position from mesh ref
coll->pos = mesh_ref->pos;

// 6. Register with scene manager
SceneObject_SetupCallback(0x4F7360, 0x168, 0);  // global scene + callback type

// 7. Store angle at board+0x438C
board->windmill_angle = (float)result;

// 8. Create Trapdoor
trapdoor_mesh = Level_FindObjectByName("TRAPDOOR");
trapdoor = operator_new(0x10F8);
GlassStands_Ctor(trapdoor, mesh_ptr);
```

### Why Windmill Can't Be Spawned Globally

The Windmill is a **CollisionLevel** — a collision shape without:
- A game object vtable
- An update function (vtable[11])
- A render function (vtable[18])

The visual windmill mesh is part of the level's static geometry (rendered by the scene's spatial tree). The CollisionLevel only provides collision detection that pushes balls away when they touch it.

To spawn a windmill globally, you would need to:
1. Create the CollisionLevel (pushes balls)
2. Also spawn a visual mesh at the same position
3. Register both with the scene manager

This is significantly more complex than spawning a Mace.

---

## CHOMPER (and TURRET)

### The Truth About Chompers

**CHOMPER is NOT a separate game object.** It is just a visual mesh (MeshNode) loaded by the Tower constructor:

```c
board+0x4390 = MeshNode_ctor("Meshes\\Chomper");  // VA 0x4D094C
```

The MeshNode is a static visual element with no collision, update, or interaction logic.

### The Actual Object: TURRET

The game object that appears near chompers is the **TURRET**, which uses the `Level4-Turret` mesh (not the Chomper mesh).

| Function | Address | Purpose |
|----------|---------|---------|
| Stands_ctor | 0x462850 | Constructor — thiscall(obj, mesh) ret 4 |
| Turret vtable | 0x4D8FB0 | Set by Stands_ctor |

**Object Structure** (0x10D0 bytes):

| Offset | Description |
|--------|-------------|
| +0x10D0 | Board pointer |
| +0x10D4/+0x10D8/+0x10DC | Position XYZ |

### Creation Flow

```c
// Find "TURRET" mesh ref
mesh_ref = Level_FindObjectByName("TURRET");
if (mesh_ref == NULL) skip;

// Allocate
obj = operator_new(0x10D0);

// Construct with Level4-Turret mesh
mesh_ptr = *(board + 0x43B4);  // pre-loaded Level4-Turret
Stands_ctor(obj, mesh_ptr);    // thiscall, ret 4 → sets vtable = 0x4D8FB0

// Copy position
obj->pos = mesh_ref->pos;

// Register
call 0x457AD0;  // Timer_Init or similar setup
```

### Mesh Pre-loading (Tower Constructor)

```c
board+0x436C = MeshWorld_ctor("Levels\\Level4-Catapult");
board+0x4370 = MeshWorld_ctor("Levels\\Level4-Drawbridge");
board+0x4374 = MeshNode_ctor("Meshes\\YellowLink");
board+0x4378 = MeshWorld_ctor("Levels\\Level4-Mace");      // Mace mesh
board+0x437C = MeshWorld_ctor("Levels\\Level4-Windmill");  // Windmill mesh
board+0x4390 = MeshNode_ctor("Meshes\\Chomper");            // Chomper visual
board+0x43B4 = MeshWorld_ctor("Levels\\Level4-Turret");     // Turret mesh
```

---

## Global Spawn: Mace

The Mace (Pendulum) is the most suitable for global spawning because it:
- Has its own vtable with update (swing) and render functions
- Has a standalone constructor that can be called with any mesh
- Has ball collision via N:MACE event
- Has activation via E:MACETRIGGER

### JIT Mesh Injection Pattern

1. Load `Levels\Level4-Mace` mesh via `MeshWorld_ctor` → cache at `board+0x4370`
2. On spawn:
   - Save old `board+0x4378` value
   - Write Mace mesh to `board+0x4378`
   - `operator_new(0x110C)`
   - `CascadeStands_Ctor(obj, board, mesh)` — reads from `board+0x4378`
   - Restore `board+0x4378`
   - Set position at `obj+0x10D8/+0x10DC/+0x10E0`
   - Set `obj+0x10E8 = 80.0` (swing amplitude)
   - Set `obj+0x10F4 = 1` (active)
   - Set `obj+0x10F8 = 50` (timer)
   - `AthenaList_Append(board+0x2578, obj)` (general list)
   - `AthenaList_Append(board+0x5000, obj)` (mace list)
3. Per-frame: call `vtable[11]` (Pendulum_Update) for swing animation

### Key Differences from Other Mods

- **Alloc size:** 0x110C (larger than Bonk's 0x1200 or Sawblade's 0x111C)
- **Position offset:** +0x10D8 (not +0x10D4 like most objects)
- **Constructor:** Takes 2 params (board + mesh), not just board
- **Two lists:** General (board+0x2578) AND mace list (board+0x5000)
- **JIT mesh injection:** Required because CascadeStands_Ctor reads from board+0x4378
