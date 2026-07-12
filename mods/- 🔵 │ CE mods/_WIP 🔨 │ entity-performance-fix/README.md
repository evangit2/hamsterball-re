# Entity Performance Fix

## Problem
The game freezes/crashes when too many entities (balls, clones, 8-balls) are active in arenas, especially when they fall/respawn simultaneously.

## Root Cause Analysis (Ghidra decompilation trace)

### Bottleneck 1: O(N²) Ball-Ball Collision
**Function:** `Scene_UpdateBallsAndState` (0x41B540) → `Ball_Update` (0x405E00)

Each frame, the game iterates ALL balls in the AthenaList at `Scene+0x29D4` and calls `vtable[4]` (Ball_Update) for each. Ball_Update internally:
- Allocates a `SpatialTree` (0x20 bytes) via `operator_new` at 0x40689A
- Allocates a `CollisionNode` (0x14 bytes) via `operator_new` at 0x4068F7
- Recursively traverses the spatial tree via `Collision_TraverseSpatialTree` (0x465EF0)
- Iterates all collision entries (O(N) per ball)
- For ball-ball collision (type==1): full physics response per pair

Total cost: **O(N²) per frame** for N balls, plus 0x34 bytes of heap allocation per ball per frame.

### Bottleneck 2: O(N²) Mass Despawn
**Function:** `AthenaList_Remove` (0x453690)

When balls fall off the arena, they're removed from the AthenaList. The remove function does:
1. `malloc(count * 4)` — allocate a new array
2. Copy all entries except the removed one
3. `free(old_array)`
4. Decrement 0x20 (32) iterator indices

This is O(N) per removal. When M balls fall simultaneously: **O(M × N) total**.

### Bottleneck 3: Heavy Respawn Scan
**Function:** `Ball_FindClosestRespawnPoint` (0x405190)

Scans ALL spawn points in `Scene+0x1518` AthenaList. For each spawn point:
- `stricmp` / `strstr` string comparisons
- Distance calculation (3D magnitude)
- When a candidate is found: calls `Mesh_FindClosestCollision` (0x465D90) which builds a **full spatial tree from scratch** (AthenaList_Init → CollisionMesh_ctor → SpatialTree_ctor → CollisionMesh_AddTriangle → Ball_AdvancePositionOrCollision → full cleanup)

Cost: O(spawn_points) per respawn, with a heavy collision mesh build per candidate.

### Bottleneck 4: Per-Add Heap Realloc
**Function:** `AthenaList_Append` (0x453780)

Does `realloc(ptr, (count+1)*4)` PER ADD. O(N) heap churn per spawn, causes fragmentation.

### Bottleneck 5: Iterator Limit
**Function:** `AthenaList_NextIndex` (0x4532B0)

Wraps at 0xFF (255). Hard limit on concurrent nested iterations — if exceeded, iterators corrupt each other's indices.

## Solution: CE AutoAssembler Script

Four hooks that cap work per frame:

| Hook | Address | Function | Effect |
|------|---------|----------|--------|
| 1 | 0x41B540 | Scene_UpdateBallsAndState entry | Reset per-frame counters, store ball count |
| 2 | 0x41B58A | First loop vtable[4] call | Skip non-player balls when count > cap |
| 3 | 0x41B62F | Second loop vtable[4] call | Same skip for secondary entity list |
| 4 | 0x405190 | Ball_FindClosestRespawnPoint entry | Throttle respawns to N per frame |

### Skip Logic
- Player balls (ball+0x18 != -1) **always** get full physics
- Non-player balls use hash-based rotation: `(ball_ptr >> 4) XOR frame_counter`
- Each frame, `MAX_BALLS_PER_FRAME / total_count` fraction of non-player balls get processed
- All balls get processed within `ceil(count / MAX)` frames
- At 60 FPS with 100 balls and MAX=18: each ball updates every ~6 frames (10 Hz)

### Performance Impact
| Ball Count | Without Fix | With Fix (MAX=18) | Improvement |
|-----------|-------------|-------------------|------------|
| 50 | ~50 balls × 50 collisions = 2500 ops | ~18 × 50 = 900 ops | 2.8x |
| 100 | ~100 × 100 = 10000 ops | ~18 × 100 = 1800 ops | 5.5x |
| 200 | ~200 × 200 = 40000 ops | ~18 × 200 = 3600 ops | 11x |

### Adjustable Parameters (CE Address List)
- `MAX_BALLS_PER_FRAME` (default 18): Max balls with full physics per frame
- `MAX_RESPAWNS_PER_FRAME` (default 3): Max respawns per frame
- `BALL_COUNT`: Current ball count (read-only, for monitoring)

## Usage
1. Open Cheat Engine, attach to Hamsterball.exe
2. File → Load → select `EntityPerformanceFix.CEA`
3. Enable the script (check the box)
4. Add `MAX_BALLS_PER_FRAME` and `MAX_RESPAWNS_PER_FRAME` to your address list to adjust at runtime

## Tuning Guide
- **Still lagging?** Decrease `MAX_BALLS_PER_FRAME` to 10-12
- **Balls too jerky?** Increase `MAX_BALLS_PER_FRAME` to 24-30
- **Respawns too slow?** Increase `MAX_RESPAWNS_PER_FRAME` to 5-8
- **Respawn storms still lag?** Decrease `MAX_RESPAWNS_PER_FRAME` to 1-2
