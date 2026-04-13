# Ball_Update Decomposition (0x405190)

## Function Signature
```c
void __fastcall Ball_Update(int param_1);  // param_1 = Ball*
```

## Overview
Ball_Update is the main per-frame physics update for the ball. It handles:
1. Reset per-frame state
2. Find the closest collision surface (floor/wall) based on gravity plane
3. Snap ball to surface and zero velocity when surface found
4. If no surface found, ball falls freely

## New Ball Structure Offsets Discovered

| Offset | Type | Field | Description |
|--------|------|-------|-------------|
| +0x150 | uint32 | ??? | Reset to 0 each frame |
| +0x18 | int | player_index | 0 or 1 (player), -1 = AI/none |
| +0x1A4 | int | level_ptr | Pointer to Level object |
| +0x2DC | float | ??? | Used as X/Z position for collision |
| +0x2E0 | float | ??? | Used as Y/height for collision |
| +0x2E4 | float | ??? | Used as Z position for collision |
| +0x2E8 | char | flag1 | Reset to 0 each frame |
| +0x2E9 | char | flag2 | Reset to 0 each frame |
| +0x2EC | uint32 | ??? | Reset to 0 each frame |
| +0x2F0 | uint32 | force_count | Number of forces this frame |
| +0x2F8 | char | update_in_progress | Set to 1 during update |
| +0x2F9 | char | frozen | Ball is stuck/frozen on surface |
| +0x2FC | uint32 | freeze_timer | Countdown when frozen |
| +0x29C | float | scale | Reset to 1.0 (0x3F800000) |
| +0x300 | uint32 | ??? | Set to 150 (0x96) on surface snap |
| +0x310 | char | ??? | Set to 1 each frame |
| +0x314 | float | ang_vel_y | Set to 0 on snap |
| +0x318 | float | ang_vel_x | Set to 0 on snap |
| +0x31C | char | ??? | Set to 0 on snap |
| +0x31D | char | ??? | Set to 0 on snap |
| +0x324 | char | in_tube | If true, skip entire update |
| +0x76A | char | ??? | Set to 0 on snap |
| +0x768 | char | ??? | Set to 1 each frame |
| +0x810 | AthenaList | path_list | Ball path tracking |
| +0xC28 | void* | free_ptr | Freed and nulled each frame |
| +0xC2C | char[] | section_filter | Current collision section name |
| +0xC74 | uint32 | collision_count | Reset to 0 each frame |
| +0xC80 | char | ??? | Has special collision flag |
| +0xC88 | float[4] | special_col | Special collision data |

## Collision Surface Finding Algorithm

The function has THREE code paths based on **gravity_plane** (this+0x748):

### Gravity Plane 0 (XY - Top-down / standard)
- Iterates through collision surface list at `level+0x1518`
- For each collision entry (AthenaList iterator):
  - Check **section filter**: if `this+0xC2C` is set, only surfaces matching `(<filter>` prefix
  - Check **[Z] and [X] tags**: skip surfaces tagged `[Z]` or `[X]`
  - Compute distance from ball to surface using 3D distance (SQRT)
  - Ball position is compared against `(this+0x2E0 - radius - _DAT_004CF48C)` for Y threshold
  - Keep the CLOSEST surface (minimum distance)

### Gravity Plane 1 (Y-tilted)
- Similar iteration but checks for `[X]` tag (NOT `[Z]`)
- Distance computed with **opponent ball comparison** for 2-player mode
- Snaps ball differently: `pos.x = radius + surface.x, pos.y = surface.y, pos.z = surface.z`

### Gravity Plane 2 (XZ)
- Similar iteration but checks for `[Z]` tag (NOT `[X]`)
- Snaps ball: `pos.x = surface.x, pos.y = surface.y, pos.z = surface.z - radius`

## Surface Snap (when closest surface found)

When a valid surface is found (local_84 != NULL):

```c
// Gravity 0: snap Y to surface + radius
ball->y = surface[2] + ball->radius;

// Gravity 1: snap X to radius + surface
ball->x = ball->radius + surface[1];
ball->y = surface[2];
ball->z = surface[3];

// Gravity 2: snap Z to surface - radius
ball->z = surface[3] - ball->radius;

// ALL gravity planes: zero velocity + angular velocity
ball->vx = ball->vy = ball->vz = 0;
ball->ang_vel_x = ball->ang_vel_y = 0;
ball->frozen = 1;
ball->freeze_timer = 150;
```

## Ball-to-Ball Collision (2-Player Mode)

When `level+0x234` is set (2-player mode) and `ball->player_index != -1`:
- Gets the OTHER ball: `level + 0x5DC + (1 - player_index) * 0xA0`
- Compares ball position to other ball position
- If distance < ball->radius, skip this collision surface
- This prevents one ball from detecting surfaces near the other ball

## Mesh_FindClosestCollision

Called when gravity_plane == 0 and level has active collision mesh:
```c
Mesh_FindClosestCollision(level+0x8B0, &result);
// result is offset from surface position
// Checks if distance > _DAT_004CF484 (some threshold)
```

## Key Global Constants

| Address | Value | Usage |
|---------|-------|-------|
| 0x4CF48C | 2.0 | Y offset threshold (radius + epsilon) |
| 0x4CF484 | 40.0 | Collision mesh distance threshold | 
| 0x4CF368 | 0.0 | Epsilon for float comparison |
| 0x4CF380 | 0.25 | Force multiplier when force_count > 0 |
| 0x4CF378 | 0.0 | Force multiplier when in_tube (no force!) |
| 0x4CF374 | 0.2 | Force multiplier when on_ice |
| 0x4CF36C | 0.75 | Force multiplier when is_dizzy |
| 0x4CF3E8 | 6.0 | Ice friction factor |

## Level Structure Offsets (from Ball_Update)

| Offset | Type | Field | Description |
|--------|------|-------|-------------|
| +0x234 | char | is_2player | Two player mode |
| +0x237 | char | ??? | Active flag for collision |
| +0x878 | int | ??? | Sub-object reference |
| +0x8B0 | void* | collision_mesh | Mesh_FindClosestCollision target |
| +0x1518 | AthenaList | collision_list | List of collision surfaces |
| +0x151C | int | collision_count | Number of collision entries |
| +0x1520 | int* | collision_iter | Per-thread iterator indices |
| +0x1924 | void** | collision_array | Pointers to collision data |
| +0x29D4 | AthenaList | ball_list | List of other ball objects |
| +0x29D8 | int | ball_count | Number of ball entries |
| +0x2DE0 | void** | ball_array | Pointers to ball data |
| +0x5DC | Ball[2] | balls | Two player ball slots (0xA0 each) |

## Collision Entry Structure

Each collision entry in the AthenaList has:
```
[0]  char*  name         // Surface name (e.g. "PLATFORM(X)", "Walls[Y]")
[1]  float  x            // Surface position X (or Y for gravity 1)
[2]  float  y            // Surface position Y (or Z for gravity 1)
[3]  float  z            // Surface position Z (or height)
```

## Tags in Collision Names
- `[X]` — Surface only applies to gravity plane 1 (Y-tilted)
- `[Z]` — Surface only applies to gravity plane 0 (standard)
- `(section)` — Section filter prefix for zone-based collision

---

*Decompiled from Ghidra output, 477 lines of decompiled C*
*Date: April 12, 2026*