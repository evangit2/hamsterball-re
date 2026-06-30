# Windmill & Judge System (Tower + Expert Races)

Complete reverse-engineering analysis of the Windmill (Tower Race) and Hammy Judge (Expert Race) systems.

---

## WINDMILL (Tower Race)

### Overview

The Windmill is a **CollisionLevel + static visual mesh**. It has no game object vtable — instead, its collision mesh contains `N:SWIRL` events that rotate the ball on contact. The visual windmill mesh is rendered as part of the level's static geometry.

### Binary Addresses

| Symbol | Address | Purpose |
|--------|---------|---------|
| "Levels\\Level4-Windmill" | 0x4D095C | Mesh file path string |
| "N:SWIRL" | 0x4CF928 | Collision event on windmill triangles |
| CollisionLevel_ctorWithLevel | 0x465080 | Creates collision level from mesh |
| Level_LoadMeshes | 0x465200 | Loads mesh vertex/index data |
| SceneObject_SetupCallback | 0x45DD60 | Registers collision with scene manager (stdcall, ret 8) |
| SceneObject_ctor | 0x462850 | Creates visual mesh object (thiscall, ret 4) |
| Rotator_AddBall | 0x43B6F0 | N:SWIRL handler — applies rotation to ball |

### Creation Flow (Scene_LoadLevel4 @ 0x40D6D0)

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

// 6. Register with scene manager for collision detection
SceneObject_SetupCallback(0x4F7360, 0x168, 0);  // global scene + callback type

// 7. Store angle at board+0x438C
board->windmill_angle = (float)result;

// 8. Also creates a Trapdoor
trapdoor_mesh = Level_FindObjectByName("TRAPDOOR");
trapdoor = operator_new(0x10F8);
GlassStands_Ctor(trapdoor, mesh_ptr);
```

### How N:SWIRL Works

The windmill mesh's collision triangles have `N:SWIRL` event names embedded in them. When the ball intersects these triangles:

```c
// In collision dispatch (Tower Arena handler @ 0x414570):
if (stricmp(eventName, "N:SWIRL") == 0) {
    Rotator_AddBall(board, ball);  // 0x43B6F0
}
```

`Rotator_AddBall` registers the ball on a rotator AthenaList with a 10-frame tick counter. Each frame, a rotation matrix is applied to the ball's position and velocity, spinning it around the windmill center.

### Tower Constructor Mesh Pre-loading

```c
board+0x437C = MeshWorld_ctor("Levels\\Level4-Windmill");  // VA 0x4D095C
```

### Why It Needs Both Collision + Visual

- **CollisionLevel**: Provides N:SWIRL collision events that spin the ball
- **Visual mesh**: The windmill mesh is normally part of the level's static geometry (spatial octree). When spawned globally, a separate SceneObject_ctor object is needed to render it.

---

## JUDGE / HAMMY JUDGE (Expert Race)

### Overview

The Hammy Judge is a standalone game object with its own vtable, update, and render functions. It appears as a hamster judge character that scores balls passing through gates. Fully spawnable as a standalone object.

### Binary Addresses

| Function | Address | Calling Convention | Purpose |
|----------|---------|-------------------|---------|
| Judge_Ctor | 0x43A150 | thiscall(obj, board) ret 4 | Constructor — calls Level_ctor, sets vtable |
| Judge_Update | 0x434B60 | thiscall(obj) | vtable[11] — animation (trig oscillation) |
| Judge_Render | 0x43A270 | thiscall(obj) | vtable[18] — render judge mesh |
| Judge_scalar_dtor | 0x43A250 | thiscall(obj) | vtable[0] — destructor |
| Level_ctor | 0x461740 | thiscall(obj, D3D) | Base class constructor (loads mesh internally) |

### Object Structure (0x1100 bytes = 4352)

| Offset | Size | Type | Description |
|--------|------|------|-------------|
| +0x0000 | 4 | ptr | Vtable (0x4D52B8) |
| +0x10D0 | 4 | ptr | Board pointer |
| +0x10D4 | 4 | float | Position X |
| +0x10D8 | 4 | float | Position Y |
| +0x10DC | 4 | float | Position Z |
| +0x10E4 | 4 | float | Animation parameter |

### Vtable (0x4D52B8)

| Index | Offset | Address | Function |
|-------|--------|---------|----------|
| 0 | 0x00 | 0x43A250 | scalar_dtor |
| 11 | 0x2C | 0x434B60 | Judge_Update (trig oscillation animation) |
| 18 | 0x48 | 0x43A270 | Judge_Render |

### Creation Flow

**Factory** (inside `CreateSawblade` @ 0x40E590):
```c
// 1. Find "JUDGE" mesh ref
mesh_ref = Level_FindObjectByName("JUDGE", 5, board);
if (mesh_ref != NULL) skip;  // Already exists in Race mode

// 2. Allocate
obj = operator_new(0x1100);  // 4352 bytes

// 3. Copy position from mesh ref to stack
push_pos(mesh_ref->pos);

// 4. Construct
// Judge_Ctor(this=obj, board) — thiscall, ret 4
// Inside: Level_ctor(obj, D3D_device) → loads "meshes\hammyjudge" mesh
//         Sets vtable = 0x4D52B8
push board;
mov ecx, obj;
call Judge_Ctor;

// 5. Register
AthenaList_Append(board+0x4BBC, obj);  // judge list
```

**Constructor** (`Judge_Ctor` @ 0x43A150):
```c
Judge_Ctor(this, board) {
    App = board+0x878;
    D3D = App+0x174;
    Level_ctor(this, D3D);          // loads "meshes\hammyjudge" mesh
    vtable = 0x4D52B8;
    // copy position from param
}
```

### Expert Constructor Mesh Loading

`LevelBoard_Expert_ctor` loads 3 copies of the HammyJudge mesh:
```c
board+0x4BB0 = MeshWorld_ctor("meshes\\hammyjudge");  // judge 1
board+0x4BB4 = MeshWorld_ctor("meshes\\hammyjudge");  // judge 2
board+0x4BB8 = MeshWorld_ctor("meshes\\hammyjudge");  // judge 3
```

### Update Function (vtable[11] @ 0x434B60)

```c
void Judge_Update(this) {
    FLD [this+0x10D8]  // pos X
    FLD [this+0x10DC]  // pos Y
    CALL 0x459860      // trig function (same as Pendulum — sin/cos oscillation)
    CALL 0x4BA754      // some function
    FLD [this+0x10E4]  // animation parameter
    // ... animation logic
}
```

The Judge animates using the same trig oscillation pattern as the Pendulum/Mace — it bobs/rotates around its position using sin/cos functions.

### Collision Events

| Event | Handler | Action |
|-------|---------|--------|
| `E:ALERTJUDGES` | ExpertCollisionEvents (0x40E6A0) | Iterates board+0x4BBC, activates all judges |
| `E:BELL` | ExpertCollisionEvents | Rings bell sound |
| `E:SCORE` | ExpertCollisionEvents | Awards points |

### Judge List

Judges are registered to `board+0x4BBC` (dedicated judge list). The `E:ALERTJUDGES` event iterates this list to activate/deactivate judges.

### Texture Files

| File | Purpose |
|------|---------|
| `textures\hammy1.png` | Judge texture 1 |
| `textures\hammy2.png` | Judge texture 2 |
| `textures\hammy3.png` | Judge texture 3 |

---

## Global Spawn Approach

### Judge

Fully spawnable — standalone game object with vtable:
1. `operator_new(0x1100)`
2. `Judge_Ctor(obj, board)` — loads "meshes\hammyjudge" internally via Level_ctor
3. Set position at `obj+0x10D4/+0x10D8/+0x10DC`
4. `AthenaList_Append(board+0x4BBC, obj)` — judge list
5. `AthenaList_Append(board+0x2578, obj)` — general list
6. Per-frame: call `vtable[11]` (Judge_Update) for animation

No JIT mesh injection needed — Level_ctor loads the mesh internally.

### Windmill

Requires spawning BOTH collision + visual mesh:
1. Load `Levels\Level4-Windmill` mesh via `MeshWorld_ctor`
2. Create visual: `operator_new(0x10D0)` → `SceneObject_ctor(obj, mesh)` → visible mesh
3. Set position, register to `board+0x2578`
4. Create collision: `operator_new(0x10D0)` → `CollisionLevel_ctorWithLevel(coll, mesh)`
5. `Level_LoadMeshes(coll)` — load collision data
6. Set position on collision
7. `SceneObject_SetupCallback(0x4F7360, 0x168, 0)` — register collision
8. N:SWIRL events fire automatically when ball touches collision mesh
