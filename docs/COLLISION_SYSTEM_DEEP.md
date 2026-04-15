# Collision System Deep Dive

## Overview
Hamsterball uses a spatial tree (octree) for broad-phase collision detection,
with AABB (Axis-Aligned Bounding Box) tests and per-triangle collision mesh
processing for narrow-phase. Ball_AdvancePositionOrCollision (0x4564C0) is the
main physics entry point that delegates to collision handlers via vtable dispatch.

## Spatial Tree (Octree)

### Collision_TraverseSpatialTree (0x465EF0)
Recursive traversal of the octree structure. For each node:

1. **Descend children** (AthenaList at node+0x18):
   - Get first child from node+0x424
   - For each child: recurse into Collision_TraverseSpatialTree(child, aabb, result_list)
   - Increment iterator at node+0x20

2. **Process leaf data** (AthenaList at node+0x2C):
   - Get collision mesh buffer from node+0x438
   - For each collision mesh buffer:
     - For each vertex group (3 vertices per face = 1 triangle):
       - AABB_ContainsPoint(param_1, vx, vy, vz)
       - If contained: AthenaList_Append(param_2, vertex_ptr)

### AABB_ContainsPoint
Tests whether a point (x, y, z) is inside an axis-aligned bounding box.
Returns 0 (outside) or 1 (inside).

## CollisionMesh Structure (0xCB0 bytes)

| Offset | Type | Description |
|--------|------|-------------|
| +0x00 | vtable* | Mesh_DeletingDtor (0x4D8E10) |
| +0x10 | void* | Scene pointer (param_1) |
| +0x18 | AthenaList | Child mesh list (octree children) |
| +0x430 | AthenaList | Face/vertex data (collision geometry) |
| +0x848 | AthenaList | Additional mesh data |
| +0xC74 | int | Flags/counter |
| +0xCA4 | Vec3 | AABB min (or position) |
| +0xCA8 | Vec3 | AABB max (or size) |
| +0xCAC | int | Collision state flag |

### CollisionMesh_ctor (0x456D80)
```
CollisionMesh_ctor(this, scene):
  this->vtable = 0x4D8E10  // Mesh_DeletingDtor
  this->scene = scene       // +0x10
  AthenaList_Init(this + 0x18)   // children
  AthenaList_Init(this + 0x430)  // faces
  AthenaList_Init(this + 0x848)  // mesh data
  this->flags = 0          // +0xC74
  this->aabb_min = {0,0,0} // +0xCA4
  this->aabb_max = {0,0,0} // +0xCA8
  this->state = 0          // +0xCAC
  Ball_InitBattleMode(this)
```

## Collision Event Dispatch (3-Tier Chain)

```
Arena_HandleCollision (0x40E6A0)
  └─→ Level_HandleCollision (0x40DCD0)
       └─→ GameObject_HandleCollision (0x40C5D0)
```

### Arena Events
| Event Name | Objects Affected |
|-----------|-----------------|
| CALLHAMMER | Hammer chase activation |
| HAMMERCHASE | Hammer movement |
| ALERTSAW1 | Saw warning #1 |
| ALERTSAW2 | Saw warning #2 |
| ACTIVATESAW | Saw blade activation |

### Level Events
| Event Name | Description |
|-----------|-------------|
| CATAPULT | Launch pad activation |
| MACE | Swinging mace collision |
| TRAPDOOR | Trapdoor opening |
| ROTATOR | Rotating platform collision |

### Base Events (GameObject_HandleCollision)
| Event Name | Description |
|-----------|-------------|
| SECRET | Secret area discovered |
| UNLOCKSECRET | Secret unlock reward |
| NODIZZY | No-dizzy powerup |

Event strings can include `<TIME>` XML tags for timed events.

## Ball Physics Pipeline

### Ball_AdvancePositionOrCollision (0x4564C0)
6-phase physics pipeline:

1. **Free Lists**: Release trail point and collision marker lists
2. **Input Velocity**: Add input force to velocity, clamp to max_speed
3. **Damping**: Apply damping formula: `(1-dt) + (1-damping)*dt`
4. **Collision**: If collision flag set → vtable[0x1C] dispatch
5. **Gravity**: Apply gravity with 0.95 damping constant
6. **Trail Recording**: Record trail points on non-collision frames

### Ball Collision Constants
| Constant | Value | Description |
|----------|-------|-------------|
| _DAT_004CF310 | 1.0 | Forward speed |
| _DAT_004CF368 | 0.0 | Zero |
| _DAT_004CF3F0 | 0.95 | Damping constant |
| _DAT_004D03A0 | Wave scale | Oscillation scale for path camera |
| _DAT_004D03A4 | Wave frequency | Oscillation frequency |
| _DAT_004CFF78 | 700.0 | Path camera max distance |
| _DAT_004CF3EC | min_dist | Path camera minimum distance |

### Ball Struct Physics Offsets
| Offset | Type | Description |
|--------|------|-------------|
| +0x14 | char | Trail recording flag |
| +0x18 | AthenaList | Trail points (Material structs, 0x68 each) |
| +0x164 | Vec3 | Position (X) |
| +0x170 | Vec3 | Velocity |
| +0x188 | float | Max speed (5000.0 = 5.0f) |
| +0x18C | float | Speed scale |
| +0x1A4 | void* | Physics body |
| +0x1A8 | float | Gravity scale |
| +0x27C | float | Radius factor |
| +0x284 | float | Radius (26.0 default) |
| +0x744 | int | Shake flag (camera shake) |
| +0x758 | Vec3 | Camera target |
| +0x76C | Vec3 | Camera actual |
| +0x848 | AthenaList | Collision markers |
| +0xC604 | vtable* | Collision handler (vtable[0x1C]) |
| +0xCA4 | Vec3 | Velocity X/Y/Z |
| +0xC64 | float | Speed scale factor |
| +0xC68 | float | Damping factor |
| +0xC70 | float | Max velocity clamp |
| +0xC74 | float | Accumulated trail distance |
| +0xC7C | char | Collision occurred flag |
| +0xC88 | Vec3 | Gravity components |
| +0xC94 | float | Gravity scale (0.5 default) |

## Face/Vertex Data Format

### CollisionFace Structure (0x18 bytes)
| Offset | Type | Description |
|--------|------|-------------|
| +0x00 | vtable* | Face vtable |
| +0x04 | uint16 | Vertex index A |
| +0x08 | uint16 | Vertex index B |
| +0x0C | uint16 | Vertex index C |
| +0x10 | uint16 | Face type (0=solid, 1=trigger, etc.) |
| +0x14 | float | Distance/direction |

### MeshBuffer Structure (0x14 bytes)
| Offset | Type | Description |
|--------|------|-------------|
| +0x00 | uint32 | Vertex count |
| +0x04 | uint32 | Vertex offset |
| +0x08 | uint32 | Index count |
| +0x0C | uint32 | Index offset |
| +0x10 | uint32 | Face count |

### .COL Binary Format
```
Header: "COL\0" (4 bytes)
uint32 vertex_count
uint32 face_count
uint16 vertex_coords[vertex_count * 3]  // 16-bit fixed-point
float positions[vertex_count * 3]        // 32-bit float positions
```

## Key Collision Functions

| Address | Function | Description |
|---------|----------|-------------|
| 0x4564C0 | Ball_AdvancePositionOrCollision | Main physics pipeline (6 phases) |
| 0x465EF0 | Collision_TraverseSpatialTree | Octree traversal + AABB test |
| 0x456D80 | CollisionMesh_ctor | Collision mesh constructor |
| 0x40E6A0 | Arena_HandleCollision | Arena collision dispatcher |
| 0x40DCD0 | Level_HandleCollision | Level collision dispatcher |
| 0x40C5D0 | GameObject_HandleCollision | Base collision handler |
| 0x46B070 | WaterRipple_Render | Water ripple collision (physics-based) |
| 0x415480 | CreateWobbly1 | Wobbly bridge (baked vertex animation) |