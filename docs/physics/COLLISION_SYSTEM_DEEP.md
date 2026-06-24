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

## Collision Event Dispatch (Per-Board Handlers → Shared Base)

**IMPORTANT:** There is no single "Level" or "Arena" handler. Almost every board type overrides vtable[0x1D] (+0x74) with its own unique collision handler. Each processes board-specific events, then falls through to `DispatchCollisionEvents` (0x40C5D0) as the shared base. Arena (Rumble) boards have their own separate vtables with separate handlers.

### Race Board Handlers

| Board | Handler Address | Handler Name |
|-------|----------------|-------------|
| WarmUp | 0x0040C5D0 | DispatchCollisionEvents (no override — uses base directly) |
| Intermediate | 0x0040D340 | IntermediateCollisionEvents |
| Dizzy | 0x0040D500 | DizzyCollisionEvents |
| Tower | 0x0040DCD0 | TowerCollisionEvents |
| Expert | 0x0040E6A0 | ExpertCollisionEvents |
| Odd | 0x0040ED30 | OddCollisionEvents |
| Wobbly | 0x0040F9A0 | WobblyCollisionEvents |
| Toob | 0x00410020 | ToobCollisionEvents |
| Sky/Neon | 0x00410D00 | NeonCollisionEvents |
| Beginner | 0x004111E0 | BeginnerCollisionEvents |
| Up | 0x004119B0 | UpCollisionEvents |
| Master | 0x00412850 | MasterCollisionEvents |
| Glass | 0x00417EB0 | GlassCollisionEvents |
| Impossible | 0x00418360 | ImpossibleCollisionEvents |

### Arena (Rumble) Board Handlers

| Arena | Handler Address | Handler Name | Events |
|-------|----------------|-------------|--------|
| Beginner Arena | 0x00413DF0 | BeginnerArenaCollisionEvents | N:BUMPER, DN:SINKPLATFORM |
| Intermediate Arena | 0x00413BD0 | SinkPlatformArenaCollisionEvents | DN:SINKPLATFORM only |
| Dizzy Arena | 0x00414350 | DizzyArenaCollisionEvents | N:SWIRL, DN:SINKPLATFORM |
| Tower Arena | 0x00414570 | TowerArenaCollisionEvents | E:CATAPULTBOTTOM, DN:SINKPLATFORM |
| Up Arena | 0x00413BD0 | SinkPlatformArenaCollisionEvents | DN:SINKPLATFORM only |
| Odd Arena | 0x00414DA0 | OddArenaCollisionEvents | E:GRAVITY, DN:SINKPLATFORM |
| Expert Arena | 0x00413BD0 | SinkPlatformArenaCollisionEvents | DN:SINKPLATFORM only |
| Toob Arena | 0x00415010 | ToobArenaCollisionEvents | N:BUMPER, DN:SINKPLATFORM |
| Wobbly Arena | 0x00415540 | WobblyArenaCollisionEvents | N:SQUAREWOBBLY, DN:SINKPLATFORM |
| Sky/Neon Arena | 0x00413BD0 | SinkPlatformArenaCollisionEvents | DN:SINKPLATFORM only |
| Warmup Arena | 0x00416140 | WarmupArenaCollisionEvents | E:LAUNCH, DN:SINKPLATFORM |
| Impossible Arena | 0x00418600 | ImpossibleArenaCollisionEvents | N:BOUNCE, DN:SINKPLATFORM |
| Master Arena | 0x00412850 | MasterCollisionEvents | (shared with race board) |

```
Board vtable[0x1D] (board-specific handler)
  ├─ Process board-specific events (if event name matches)
  │   └─ Return early (for some events) OR fall through
  └─ DispatchCollisionEvents (0x40C5D0) — shared base handler
      └─ Process universal events (N:GOAL, N:TARPIT, N:WATER, E:JUMP, etc.)
```

### Tower Events (TowerCollisionEvents)
| Event Name | Description |
|-----------|-------------|
| E:CATAPULTBOTTOM | Launch pad activation |
| N:TRAPDOOR | Trapdoor opening |
| E:OPENSESAM | Open trapdoor |
| E:BITE | Mace/Chomper bite damage |
| E:MACETRIGGER | Trigger mace swing |
| N:MACE | Mace ball bounce |

### Expert Events (ExpertCollisionEvents)
| Event Name | Description |
|-----------|-------------|
| E:CALLHAMMER | Hammer chase activation |
| E:HAMMERCHASE | Hammer movement |
| E:ALERTSAW1/2 | Saw warning |
| E:ACTIVATESAW1/2 | Saw blade activation |
| E:ALERTJUDGES | Reset all judges |
| E:SCORE | Score display |
| E:BELL | Bell + bonus time |

### Base Events (DispatchCollisionEvents)
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
| 0x40E6A0 | ExpertCollisionEvents | Expert board collision handler |
| 0x418360 | ImpossibleCollisionEvents | Impossible board collision handler (N:BOUNCE, N:ONROTATOR, N:ONGEAR) |
| 0x40DCD0 | TowerCollisionEvents | Tower board collision handler |
| 0x40C5D0 | DispatchCollisionEvents | Base collision handler |
| 0x46B070 | WaterRipple_Render | Water ripple collision (physics-based) |
| 0x415480 | CreateWobbly1 | Wobbly bridge (baked vertex animation) |
| 0x43B6F0 | Rotator_AddBall | Registers ball on rotator tracking list (formerly misnamed Rotator_AddBall) |
| 0x43E600 | Catapult_Update | Per-frame rotation of tracked balls (applies rotation matrix to pos+vel) |
| 0x43E9C0 | Catapult_AddObjectConditional | Registers ball on catapult/gear tracking list (guarded by +0x1510) |
| 0x434290 | Catapult_Launch | Launch pad activation (sets catapult+0x10F0=1, +0x10F4=50 timer) |

## Rotator System (Gears, Swirls, Spinny Objects)

Spinning objects in Hamsterball physically carry the ball using a two-function system:

1. **`Rotator_AddBall`** (0x43B6F0) — called on collision with `N:ONROTATOR`, `N:SPINNY`, or `N:SWIRL`
2. **`Catapult_Update`** (0x43E600) — called every frame, applies rotation matrix to tracked balls

### How it works

When the ball touches a spinning object's collision surface, the collision handler calls `Rotator_AddBall(scene, ball)`. This function:

1. Searches the rotator's AthenaList (at `scene+0x10F0`) for the ball pointer
2. **If found**: resets the entry's tick counter to 10 (ball already tracked — keeps it on the rotator)
3. **If not found**: allocates an 8-byte struct `[ball_ptr, tick_counter=10]` and appends it to the list

Every frame, `Catapult_Update` iterates the ball-tracking list:
- Decrements each entry's tick counter
- If counter reaches 0: frees the entry (ball released from rotator)
- Otherwise: applies the object's rotation matrix to the ball's position (`ball+0x164/+0x168/+0x16C`) and velocity (`ball+0xCA4/+0xCA8/+0xCAC`)

### 10-frame grace period (not a carry limit)

The counter **resets to 10 every frame** the ball remains in contact with the rotator surface, because `Ball_FallUpdate` fires the collision event every frame. The countdown only starts ticking down after the ball leaves the rotator. This means:
- While on the rotator: ball stays tracked indefinitely (counter keeps resetting to 10)
- After leaving: 10 more frames of rotation before release (smooth transition)

### Rotator/Catapult struct offsets

| Offset | Type | Field |
|--------|------|-------|
| +0x436 | float | centerX (pivot X) |
| +0x437 | float | centerY (pivot Y) |
| +0x438 | float | centerZ (pivot Z) |
| +0x439 | float | rotSpeedZ |
| +0x43A | float | rotSpeedX |
| +0x43B | float | rotSpeedAngle |
| +0x43C | float | rotAngle (accumulated, decremented by rotSpeed each frame) |
| +0x43E | AthenaList | ballList (tracked balls with tick counters) |
| +0x10F0 | AthenaList | rotatorList (Scene-level, used by Rotator_AddBall) |
| +0x10F8 | AthenaList | catapultBallList (used by Catapult_AddObjectConditional) |
| +0x1510 | byte | active flag (Catapult_AddObjectConditional guard) |

### Event → collision handler mapping

| Event | Collision Handler | Level |
|-------|-------------------|-------|
| N:ONROTATOR | ImpossibleCollisionEvents (0x418360) | Impossible race (gears) |
| N:SPINNY | ToobCollisionEvents (0x410020) | Toob race |
| N:SWIRL | DizzyArenaCollisionEvents (0x414350) | Dizzy arena |
| N:ONGEAR | ImpossibleCollisionEvents (0x418360) | Impossible race (calls Catapult_AddObjectConditional) |