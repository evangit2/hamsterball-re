# Hamsterball Collision System

## Overview

The collision system uses a spatial tree (octree) for broad-phase culling and
AABB-triangle intersection tests for narrow-phase detection. It is the core of
ball physics — every frame, the ball's position is advanced and tested against
mesh triangles via `Ball_AdvancePositionOrCollision`.

## Architecture

```
Ball_Update
  └─ Ball_CollisionCheck (0x402DE0) — per-frame entry
       └─ Mesh_FindClosestCollision (0x465D90) — find closest hit point
            ├─ CollisionMesh_ctor — build collision mesh
            ├─ SpatialTree_ctor — build octree
            ├─ CollisionMesh_AddTriangle — populate tree with triangles
            └─ Ball_AdvancePositionOrCollision (0x4564C0) — advance + collide
                 ├─ Collision_TraverseSpatialTree (0x465EF0) — tree walk
                 │    └─ AABB_ContainsPoint (0x4580D0) — point-in-box test
                 └─ vtable[0x1C] callback — gravity-aware collision response
```

## CollisionMesh (0xCB4 bytes, vtable 0x4D8E10)

### Constructor (0x456D80)
```c
CollisionMesh_ctor(this, param_1) {
    this->vtable = &Mesh_DeletingDtor;    // +0x00
    this->ref_count = param_1;            // +0x10
    AthenaList_Init(&this->triangles, 0); // +0x18
    AthenaList_Init(&this->materials, 0); // +0x430
    AthenaList_Init(&this->vertices, 0); // +0x848
    this->accumulated_distance = 0;       // +0xC74
    this->position = {0,0,0};             // +0xCA4/CA8/CAC
    Ball_InitBattleMode(this);            // +0x00 (vtable call)
}
```

### Key Offsets
| Offset | Type | Description |
|--------|------|-------------|
| +0x00 | void** | Vtable (0x4D8E10) |
| +0x10 | int | Reference count / parent |
| +0x14 | byte | Has trail data flag |
| +0x18 | AthenaList | Triangle list |
| +0x1C | int | Triangle count |
| +0x430 | AthenaList | Material list |
| +0x424 | void** | Triangle data array |
| +0x848 | AthenaList | Vertex normal list |
| +0xC54 | void** | Vertex data array |
| +0xC64 | float | Scale factor (friction) |
| +0xC68 | float | Secondary scale |
| +0xC6C | float | Gravity callback scale |
| +0xC70 | float | Max distance threshold |
| +0xC74 | float | Accumulated distance (reset on collision) |
| +0xC7C | byte | Use gravity callback flag |
| +0xC8C | float | Gravity direction X |
| +0xC90 | float | Gravity direction Y |
| +0xC94 | float | Gravity direction Z |
| +0xCA4 | Vec3 | Position (X, Y, Z) — accumulated from velocity |

## SpatialTree (Octree, vtable 0x4D9038)

### Constructor (0x463330)
```c
SpatialTree_ctor(this, mesh_list) {
    CollisionNode_BaseInit(this, mesh_list);
    this->vtable = &SpatialTree_DeletingDtor; // +0x00
    this->scale = 0.1f;           // +0x0C (3DCCCCCDh)
    this->max_depth = 6;          // +0x10
    this->min_extent = 0.9f;      // +0x14 (3F666666h)
    this->flags = 0;              // +0x18/19
    this->enable_x = 1;           // +0x1A
    this->enable_y = 1;           // +0x1B
    this->enable_z = 1;           // +0x1C
    this->enable_sub1 = 1;        // +0x1D
    this->enable_sub2 = 1;        // +0x1E
}
```

### Key Parameters
| Offset | Value | Description |
|--------|-------|-------------|
| +0x0C | 0.1f | Octree node scale |
| +0x10 | 6 | Maximum tree depth |
| +0x14 | 0.9f | Minimum node extent |

## Spatial Tree Traversal (0x465EF0)

`Collision_TraverseSpatialTree(this, point, result_list)` does a 2-level traversal:

1. **Level 1**: Recurse through child nodes of the octree
   - Iterate `this->children[0..count-1]` at `this+0x424`
   - Uses `AthenaList_NextIndex(this+0x18)` for iteration tracking

2. **Level 2**: For each leaf, iterate mesh buffers
   - Access `this->mesh_data` at `this+0x08`
   - Iterate mesh buffer entries at `mesh_data+0x438`

3. **Per-triangle test**: For each triangle vertex group (3 vertices × 8 floats)
   - Call `AABB_ContainsPoint(point, vx, vy, vz)` per vertex
   - If contained, `AthenaList_Append(result_list, vertex_ptr)`
   - Groups processed 4 at a time (iVar5 = 3, counting down)

This means: walk octree → for each leaf → for each mesh buffer → for each
vertex group → test 3 vertices against AABB → collect matching triangles.

## AABB_ContainsPoint (0x4580D0)

Tests if point `(x, y, z)` is inside axis-aligned bounding box. AABB layout:

```
+0x00: float min_x
+0x04: float min_y
+0x08: float min_z
+0x0C: float max_x
+0x10: float max_y
+0x14: float max_z
```

Returns:
- Low byte = 1 if point is inside all 6 planes
- Bit 8 = set if a test failed (point < min for any axis)
- Bit 10 = set if NaN detected
- Bit 14 = set if point == boundary (exact edge hit)

## Collision_InitDefaultAABB (0x458000)

Creates a massive bounding box for initial broad-phase test:
```c
// 0x4B18967F ≈ 10,000,000.0f
// 0xCB18967F ≈ -10,000,000.0f
AABB = { min_x=1e7, min_y=1e7, min_z=1e7,
         max_x=-1e7, max_y=-1e7, max_z=-1e7 }
```

## AABB_TriangleIntersect2 (0x4583F0)

Double test — calls `AABB_TriangleTest6Edges` twice. The second call may test
with modified parameters (separating axis theorem — test both AABB edges and
triangle edges as potential separating axes).

## Ball_AdvancePositionOrCollision (0x4564C0) — Core Physics

**Signature**: `Ball_AdvancePositionOrCollision(this, out_pos, start_pos, direction, gravity_accel, dt)`

### Algorithm
1. **Clear trail data**: If `this+0x14 != 0`, free all entries in `this+0x18` list
2. **Clear vertex normals**: Free all entries in `this+0x848` list
3. **Apply direction to position**: If direction != (0,0,0):
   - Compute current and new position magnitude
   - If new distance > max_distance (`this+0xC70`): normalize and clamp
4. **Apply friction**: `position *= (1.0 - dt) + (1.0 - this+0xC68) * dt`
5. **Collision callback or direct offset**:
   - If `this+0xC7C == 0`: `out = position + start_pos` (simple translation)
   - If `this+0xC7C != 0`: call `vtable[0x1C](out, start, dir, accel, dt, &hit)` — gravity-aware
6. **Apply gravity**: accumulate gravity vector scaled by dt × gravity_scale
7. **Update distance counter**:
   - On no hit: `this+0xC74 += dt` and optionally log trail entry
   - On hit: `this+0xC74 = 0` (collision resets accumulated distance)
8. **Write output position**

### Key Physics Constants
| Address | Value | Description |
|---------|-------|-------------|
| 0x4CF310 | 1.0 | Identity friction base |
| 0x4CF368 | 0.0 | Zero comparison threshold |
| 0x4CF3F0 | (varies) | Gravity scaling factor |

## Mesh_FindClosestCollision (0x465D90)

High-level collision query. Creates a temporary collision mesh, builds a
spatial tree, then traces a ray from start position in a direction to find
the closest triangle intersection.

```
1. AthenaList_Init(mesh_list, 0)
2. AthenaList_Append(mesh_list, this)      // add self
3. CollisionMesh_ctor(collision_mesh, 0)
4. SpatialTree_ctor(spatial_tree, mesh_list)
5. CollisionMesh_AddTriangle(collision_mesh, spatial_tree)
6. Vec3_NormalizeAndScale(direction, 99999.0)  // extend ray very far
7. Ball_AdvancePositionOrCollision(collision_mesh, out, start, dir, gravity, 0.01)
8. SpatialTree_Free(spatial_tree)
9. Mesh_Clear(collision_mesh)
10. Vec3List_Free(mesh_list)
11. return out_position
```

The time step is hardcoded to `0.01` and the ray extends to `99999.0` units,
making this effectively a raycast from start→direction finding the nearest
triangle surface.

## Collision_GradientEval_Stub (0x458190)

Empty stub — `return;`. Called from `Ball_Update` and `Gear_AdvanceAlongPath`
as a no-op placeholder for gradient evaluation (unused feature).

## Integration with Ball Physics

The collision system is called from `Ball_Update` (0x405190) every frame:

1. Ball computes desired new position from velocity + forces
2. `Ball_CollisionCheck` tests the mesh via `Mesh_FindClosestCollision`
3. `Ball_AdvancePositionOrCollision` updates position, detects collision
5. On collision: velocity zeroed, accumulated distance reset, material tracked
6. On no collision: advance freely, accumulate trail data

## Collision Event Dispatch System

The collision system dispatches game events when the ball hits specific object types.
There are two dispatchers: Level_HandleCollision (for level objects) and Arena_HandleCollision (for RumbleBoard arena objects).

### Level_HandleCollision (0x40DCD0) — Level Objects

String-based dispatch on collider type name (collider+0x864):

| Type Name | Behavior |
|-----------|----------|
| "E:CATAPULTBOTTOM" | Catapult_Launch if ball cooldown < 1, play catapult sound |
| "E:OPENSESAME" | Trapdoor_Open on first door in list |
| "N:TRAPDOOR" | Trapdoor_Activate on doors matching collider ID |
| "E:BITE" | Set damage=25.0 (damage_amount+0x43A0, damage_timer+0x43A8) |
| "E:MACETRIGGER" | Set active=1 for all maces in mace list |
| "N:MACE" | Ball bounce callback (vtable[0x20]) if mace is moving |

### Arena_HandleCollision (0x40E6A0) — RumbleBoard Arena Objects

| Type Name | Behavior |
|-----------|----------|
| "E:CALLHAMMER" | CreateBonkPopup (tournament only) |
| "E:HAMMERCHASE" | Hammer_ChaseStart (tournament only) |
| "E:ALERTSAW1"/"ALERTSAW2" | Saw_AlertActivate (warning phase) |
| "E:ACTIVATESAW1"/"ACTIVATESAW2" | Saw_Activate (full activation) |
| "E:ALERTJUDGES" | Judge_Reset for all judges |
| "E:SCORE\<time\>" | ScoreDisplay_SetTime parsed from suffix |
| "E:JUMP" | Jump pad: cooldown=10, vert_vel=0.008, vert_vel_on=1, play sound |
| "E:BELL\<suffix\>" | Bell_Activate, add 500 bonus time if not playing |

### Level_LoadCollision (0x65260) — Collision File Format

Binary .COL file format loaded at level startup:

```
Header: 24 bytes into MeshWorld+0x45C (transform/flags)
int32: sublevel_count

If sublevel_count < 1 (single-mesh mode):
  int32: object_count
  For each object:
    CreateMeshBuffer(0x874 bytes)
    int32: name_length
    char[name_length]: name string ("N:"=interactive, "E:"=event)
    int32: face_count
    For each face (0x60 bytes each):
      9 floats: v0, v1, v2 positions (3 floats each)
      Compute normal via cross product, normalize via SIMD
      Same normal stored for all 3 verts (flat shading)

If sublevel_count >= 1 (multi-level mode):
  scene->is_sprite_mode = 1
  For each sublevel: Level_ctor + vtable[0x60](file) recursive load
```

### CollisionFace Structure (0x60 bytes)

| Offset | Field | Description |
|--------|-------|-------------|
| +0x00 | v0.x | Vertex 0 X |
| +0x04 | v0.y | Vertex 0 Y |
| +0x08 | v0.z | Vertex 0 Z |
| +0x0C | n0.x | Face normal X (flat, same for all verts) |
| +0x10 | n0.y | Face normal Y |
| +0x14 | n0.z | Face normal Z |
| +0x18 | v1.x | Vertex 1 X |
| +0x1C | v1.y | Vertex 1 Y |
| +0x20 | v1.z | Vertex 1 Z |
| +0x24 | n1.x | Face normal X (same as n0) |
| +0x28 | n1.y | Face normal Y |
| +0x2C | n1.z | Face normal Z |
| +0x30 | v2.x | Vertex 2 X |
| +0x34 | v2.y | Vertex 2 Y |
| +0x38 | v2.z | Vertex 2 Z |
| +0x3C | n2.x | Face normal X (same as n0) |
| +0x40 | n2.y | Face normal Y |
| +0x44 | n2.z | Face normal Z |

### MeshBuffer Structure (0x874 bytes)

| Offset | Field | Description |
|--------|-------|-------------|
| +0x000 | vtable | CollisionMesh vtable |
| +0x00C | AthenaList | Face list |
| +0x217 | byte | render_flag (=0) |
| +0x85D | byte | interactive_flag (1 for N:/E: prefixes) |
| +0x863 | byte | no_render_flag (1 for E: prefix only) |
| +0x864 | char* | Type name string ("N:WALL", "E:CATAPULTBOTTOM", etc.) |
6. Vtable[0x1C] callback handles gravity-plane-aware collision (tilted/flat modes)