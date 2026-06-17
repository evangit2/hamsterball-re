# Hamsterball Level Collision System

This document describes the **level-side collision classes** used by the game’s static mesh raycasts. It is distinct from the ball’s `CollisionMesh` physics body (see [COLLISIONMESH_OBJECT_MODDING.md](COLLISIONMESH_OBJECT_MODDING.md)).

All offsets and addresses are from the original Windows `Hamsterball.exe` (image base `0x00400000`).

---

## Overview

When a level is loaded, the game creates two objects from the `.MW` file:

| Scene Offset | Field | Type | Purpose |
|---|---|---|---|
| `+0x8AC` | `source_meshworld_ptr` | `MeshWorld*` | Render/source level geometry, loaded by `MeshWorld_ctor` |
| `+0x8B0` | `collision_level_ptr` | `CollisionLevel*` (or `Level*`) | Collision-only copy of the level, used by `Mesh_FindClosestCollision` |

Both pointers are filled by `Scene_LoadLevel2` / `Scene_LoadLevel3` etc. and finalized by `Level_InitScene`. The collision pointer (`+0x8B0`) is the `this` pointer you hook in `Mesh_FindClosestCollision`.

---

## Class Hierarchy

```
SceneObject (base)
  └── Level / MeshWorld
        ├── +0x8   MeshWorld*   inner_mesh_world  (copy of source)
        ├── +0x480 SpatialTree* root_spatial_tree (acceleration structure)
        └── inherited AthenaLists for buffers / objects

CollisionLevel = Level variant produced by CollisionLevel_ctorWithLevel
                 its vtable is at 0x004D9068.
```

### Important distinction

- `MeshWorld` contains the render-side vertex buffers, materials, and object lists.
- `CollisionLevel` is a **separate level instance** that mirrors the geometry but is optimized for collision queries (spatial tree, NOCOLLIDE handling, `(NOCOLLIDE)` tag stripping, etc.).
- You almost never want to touch the source `MeshWorld` for physics; raycasts use `Scene + 0x8B0`.

---

## CollisionLevel / Level Struct Layout

The class is loaded through `MeshWorld_ctor` and `CollisionLevel_ctorWithLevel`. alloc size is `0x10d0` (4304) bytes.

| Offset | Type | Field | Notes |
|---|---|---|---|
| `+0x000` | `void**` | `vtable` | `0x004D9068` for CollisionLevel; `0x004D8FB0` for plain Level |
| `+0x004` | `int` | `ref_count` | Standard `SceneObject` refcount |
| `+0x008` | `MeshWorld*` | `inner_meshworld` | Points to the internal copy of the render mesh. `Collision_TraverseSpatialTree` reads `this + 0x8` → `+0x2c` (mesh buffer list). |
| `+0x00C` | `int` | `owner_id` | Usually holds the owning `App*` passed to `Level_ctor`. |
| `+0x018` | `AthenaList` | `child_levels` | List of subdivided child levels (used by `Scene_SubdivideRandom`). |
| `+0x430` | `byte` | `has_subdivided` | Set to `1` if the level was spatially subdivided. |
| `+0x431` | `byte` | `use_subdivision` | Subdivision active flag. |
| `+0x434` | `uint32_t` | `level_index` | Level number / index. |
| `+0x438` | `Timer` | `update_timer` | Standard `Timer` object. |
| `+0x47C` | `void*` | `owner_app` | Back-pointer to the `App` instance. |
| `+0x480` | `SpatialTree*` | `root_spatial_tree` | Root of the static spatial acceleration tree (size `0x10d0`). |
| `+0x484` | `byte` | `build_spatial_tree` | If `1`, a spatial tree was built for collision. |
| `+0x488` | `AthenaList` | `render_contexts` | `RenderContext` list. |
| `+0x8A0` | `AthenaList` | `mesh_buffer_list2` | Secondary buffer list. |
| `+0xCB8` | `AthenaList` | `object_list` | Level objects such as `START1-1`, `E:JUMP`, etc. |

> **Note:** Many internal lists are `AthenaList` (12-byte embedded structure: head index + size + data vector). Do not write raw pointers into them.

---

## The SpatialTree

`SpatialTree` is the actual acceleration structure used for ray-triangle tests. It is allocated at `0x10d0` bytes and stored in the `Level` at `+0x480`.

### SpatialTree vtable: `0x004D9038`

| Slot | Address | Notes |
|---|---|---|
| 0 | `0x004633B0` | `SpatialTree_DeletingDtor` |
| 1 | `0x004633B0`? | destructor path |
| 2+ | traversal / build slots | `Collision_TraverseSpatialTree` uses child lists |

### SpatialTree notable fields

| Offset | Type | Field | Default | Notes |
|---|---|---|---|---|
| `+0x00` | `void**` | `vtable` | `0x004D9038` |  |
| `+0x04` | `void*` | `owner` | list or Level* | Used by `CollisionNode_BaseInit` |
| `+0x08` | `void*` | `meshworld` | — | Used to reach mesh buffers (`+0x2c`) |
| `+0x0C` | `float` | `leaf_size` | `0.1f` | Cell/target size for tree subdivision |
| `+0x10` | `int` | `max_depth` | `6` | Max spatial-tree recursion |
| `+0x14` | `float` | `scale` | `0.9f` | Bounding-box scale factor |
| `+0x18`–`+0x1E` | `byte[7]` | `axis_flags` | all `1` | Which axes are active for subdivision |

`Collision_TraverseSpatialTree` recursively walks the child list at `+0x18`, tests triangles from each `MeshBuffer` against an AABB, and appends candidate triangles to a temporary collision mesh.

---

## How to Access an Instance

### From the global App

The easiest stable pointer is the global `App` object at `0x004FD680`:

```cpp
App* app   = (App*)0x004FD680;
void* scene = *(void**)((char*)app + 0x878);   // App->active_scene
void* collision_level = *(void**)((char*)scene + 0x8B0);
```

### From the Ball

```cpp
void* ball  = ...;
void* scene = *(void**)((char*)ball + 0x14);  // Ball->scene
void* collision_level = *(void**)((char*)scene + 0x8B0);
```

### From a `Mesh_FindClosestCollision` hook

The `this` pointer in `ECX` is exactly `Scene + 0x8B0`:

```cpp
using Mesh_FindClosestCollision_t =
    Vec3* (__thiscall*)(void* collision_level, Vec3* out,
                         Vec3 origin, Vec3 direction, float max_dist);

Mesh_FindClosestCollision_t Original =
    (Mesh_FindClosestCollision_t)0x00465D90;
```

---

## Key Functions

| Address | Name | Role |
|---|---|---|
| `0x00461510` | `MeshWorld_ctor` | Loads/references a `.MW` level and creates the source `MeshWorld`. |
| `0x00465080` | `CollisionLevel_ctorWithLevel` | Creates a collision-only `Level` from a source `MeshWorld`. Sets vtable `0x004D9068`; calls `Level_LoadMeshes`. |
| `0x00465860` | `Level_LoadMeshes` | Builds the internal `MeshWorld` copy and collision geometry for a `Level`. |
| `0x00465D90` | `Mesh_FindClosestCollision` | **Main raycast API**; builds a temporary spatial tree over the level and casts a tiny sphere. |
| `0x00403980` | `Ball_FindMeshCollision` | Thin wrapper around `Mesh_FindClosestCollision`; rarely called by the engine directly. |
| `0x00465EF0` | `Collision_TraverseSpatialTree` | Recursive triangle-collection function inside `Mesh_FindClosestCollision`. |
| `0x00463330` | `SpatialTree_ctor` | Initializes an axis-aligned spatial subdivision node. |
| `0x0040D280` | `Scene_LoadLevel2` | Hard-coded loader for Level 2; allocates `MeshWorld` at `+0x8AC` and `CollisionLevel` at `+0x8B0`. |
| `0x0040B090` | `Level_InitScene` | Finishes scene setup after both level instances are created; reads `scene + 0x8AC` and `+0x8B0`. |
| `0x00456D80` | `CollisionMesh_ctor` | Creates the temporary `CollisionMesh` used inside a single raycast (not persisted). |

---

## Vtables

### CollisionLevel / Level vtable: `0x004D9068`

| Slot | Address | Logical method |
|---|---|---|
| 0 | `0x00465240` | `Level_DeletingDtor2` |
| 1 | `0x004606D0` | (render / strip builder) |
| 2 | `0x00472770` | `SceneObject_BuildStrips` |
| 3 | `0x00471750` | `LoadMesh` |
| 4 | `0x00470440` | (render utility) |
| 5 | `0x0046F3B0` | (render utility) |
| 6 | `0x00470150` | `SceneObject_RenderFull` |
| 7 | `0x0046F390` | (render utility) |
| 8 | `0x00471830` | utility / mesh preprocessor |
| 9 | `0x0045DFD0` | `SceneObject_CallRender` variant |
| 10 | `0x00461370` | `MeshWorld_CollectTrianglesInAABB` related |
| 11 | `0x0044ACB0` | `SceneObject_CallUpdate` variant |
| 12 | `0x00465100` | `Level_LoadCollision` / clone helper |
| 13 | `0x00461890` | `Scene_LoadMeshWorld` |
| 14 | `0x004651D0` | `Level_dtor` helper |
| 15 | `0x00460DA0` | `Scene_RenderFrame` related |
| 16 | `0x00461F00` | `Scene_Subdivide` |
| 17 | `0x00462100` | `Scene_SubdivideRandom` |
| 18 | `0x00465650` | `Level_CloneTree` related |

### CollisionMesh (temporary) vtable: `0x004D8E10`

This is the temporary collision buffer used *inside* `Mesh_FindClosestCollision`, not the level class itself. Listed here because the same name appears in the code path.

| Slot | Address | Method |
|---|---|---|
| 0 | `0x00456870` | `Mesh_DeletingDtor` |
| 1 | `0x004564C0` | `Ball_AdvancePositionOrCollision` |
| 5 | `0x00456120` | `CollisionMesh_AddTriangle` |
| 8 | `0x00457A20` | helper |
| 19 | `0x00458200` | helper |
| 22 | `0x0045DF20` | traversal helper |
| 23 | `0x00458A40` | `Scene_ScalarDtorBase` path |

---

## Hooking Example: Ground Raycast Using the Collision Level

```cpp
struct Vec3 { float x, y, z; };

using Mesh_FindClosestCollision_t =
    Vec3* (__thiscall*)(void* collision_level,
                        Vec3* out,
                        Vec3 origin,
                        Vec3 direction,
                        float max_dist);

static auto Mesh_FindClosestCollision =
    (Mesh_FindClosestCollision_t)0x00465D90;

bool IsGroundedAt(float x, float y, float z, void* scene, float radius) {
    void* collision_level = *(void**)((char*)scene + 0x8B0);
    if (!collision_level) return false;

    Vec3 origin = { x, y, z };
    Vec3 down   = { 0.0f, -1.0f, 0.0f };
    Vec3 out    = { x, y - 1000.0f, z };
    float max_dist = radius + 0.5f;

    Mesh_FindClosestCollision(collision_level, &out, origin, down, max_dist);

    float dy = origin.y - out.y;
    return (dy > 0.001f && dy < max_dist);
}
```

---

## Modding Notes

- Do **not** free or replace `Scene + 0x8B0`. It owns triangle data and child spatial-tree memory used by the physics tick.
- The spatial tree is **rebuilt per raycast** in `Mesh_FindClosestCollision`. The level pointer stored at `+0x8B0` is stable; the heavy work is done inside `Collision_TraverseSpatialTree`.
- `Scene + 0x8AC` is the source `MeshWorld` used for rendering and object lookup; `Scene + 0x8B0` is the collision-only copy. Modifying `+0x8AC` geometry will not affect physics unless you regenerate `+0x8B0`.
- `CollisionLevel_PlayBreakSound` (`0x00435B00`) and `CollisionLevel_Spatial_Ctor` (`0x00436250`) are arena/break-specific branches; normal race levels use the standard collision path.

---

## Sources

All data verified from live GhidraMCP headless decompilation of `Hamsterball.exe`:

- `CollisionLevel_ctorWithLevel` @ `0x00465080`
- `Level_LoadMeshes` @ `0x00465860`
- `Level_ctor` @ `0x00461740`
- `MeshWorld_ctor` @ `0x00461510`
- `Mesh_FindClosestCollision` @ `0x00465D90`
- `Collision_TraverseSpatialTree` @ `0x00465EF0`
- `SpatialTree_ctor` / `SpatialTree_CloneToLevel` @ `0x00463330` / `0x00462380`
- `Scene_LoadLevel2` @ `0x0040D280`
- `Level_InitScene` @ `0x0040B090`
- Static disassembly of `Hamsterball.exe` for field references to `+0x8AC` and `+0x8B0`

---

*Document generated: 2026-06-17*
