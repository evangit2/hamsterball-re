# Entity Limit Fixer v2

A Cheat Engine AutoAssembler script that prevents Hamsterball from freezing when too many entities (balls, 8-balls, clones) are active simultaneously in arenas.

## Root Cause

The freeze is caused by three O(N²) loops running every frame:

1. **Ball-Ball Collision**: Each ball's `Ball_Update` (0x405E00) creates a `CollisionNode` at 0x40691B that references the full ball list (Scene+0x29D4). When `CollisionMesh::AdvancePosition` (0x4564C0) traverses the spatial tree, it hits `CollisionNode::CheckAllBalls` (0x467030) which iterates ALL balls → O(N) per ball, O(N²) total per frame.

2. **AI Target Search**: Each 8-ball's `Ball_AI_ChaseNearest` (0x408390) calls `Ball_Update`, then iterates ALL balls AGAIN at 0x4083D9 to find the nearest target → another O(N²).

3. **Respawn Storm**: `Ball_FindClosestRespawnPoint` (0x405190) iterates all SAFESPOT ref points per falling ball per frame.

With 50+ balls: ~10K checks/frame. With 100+ balls: ~20K checks/frame → hard freeze.

## Patches

| Patch | Address | Original Bytes | What It Does |
|-------|---------|---------------|-------------|
| A | 0x40690F | `8B 8E 14 00 00 00` | Skip CollisionNode creation when ball count > MAX_BALLS |
| B | 0x4083D9 | `8B 4E 14 81 C1 D4 29 00 00` | Skip AI nearest-target search when ball count > MAX_BALLS |
| C | 0x005190 | `81 EC 84 00 00 00` | Throttle respawn search to every Nth frame |

## v2 Fixes (from v1)

- **CRITICAL FIX**: Patch C now uses plain `RET` instead of `RET 0x04`. The function `Ball_FindClosestRespawnPoint` is `__thiscall` with **0 stack parameters** (confirmed by disassembly: `RET` at 0x405D83, no stack cleanup). The previous `ret 0x04` corrupted the caller's stack → crash when combined with spawn/clone mods.
- **MAX_BALLS default lowered** from 30 to 20 (game freezes at ~20 balls, not 30).
- **Compatibility verified**: No address conflicts with Player Clone System v13 (hooks 0x41B540). All three patch sites (0x40690F, 0x4083D9, 0x005190) are inside Ball_Update / Ball_AI / Ball_FindClosestRespawnPoint — completely separate from the clone mod's hook on Scene_UpdateBallsAndState.

## Configuration

- **MAX_BALLS** (default: 20): Ball count threshold. When exceeded, collision + AI throttling activates. Lower = more aggressive (10=very safe, 30=risky).
- **respawn_throttle** (default: 3): Run respawn search every 3rd frame.

## Usage

1. Open Cheat Engine, attach to `Hamsterball.exe`
2. Load `EntityLimitFixer.CEA`
3. Enable the script
4. Spawn as many entities as you want — no freeze

## How It Works

- **Patch A**: When ball count exceeds MAX_BALLS, skips `CollisionNode_ctor` (0x466CF0) by jumping to the null-node path (XOR EAX,EAX at 0x406922). This prevents the ball list from being registered with the CollisionMesh, so the spatial tree traversal never iterates balls → eliminates O(N²) ball-ball collision. Ball-geometry collision (floor/walls) still works because the `SpatialTree` at 0x4068BC is separate.
- **Patch B**: Skips the AI target search iteration when ball count is high. `Ball_Update` has already run, so physics still works. The AI ball just won't chase targets when too many balls are active.
- **Patch C**: Throttles respawn point searches to every Nth frame, preventing respawn storms when many balls fall simultaneously. Returns 0 (null) on skip frames using plain `RET`.

## Compatibility with Other Mods

- **Player Clone System v13**: ✅ No conflicts. Clone mod hooks `Scene_UpdateBallsAndState` at 0x41B540. Entity limiter patches are inside `Ball_Update` (0x40690F), `Ball_AI_ChaseNearest` (0x4083D9), and `Ball_FindClosestRespawnPoint` (0x005190).
- **8-ball AI Fix**: ✅ No conflicts. That mod patches 0x4083AE and 0x4085CD (inside the AI function but at different offsets).
- **Jump Mod**: ✅ No conflicts. That mod hooks 0x407BB4 (Ball_ApplyForce call site).

## Technical Details

### Function Call Chain

```
Scene_UpdateBallsAndState (0x41B540)  ← clone mod hooks HERE
  └─ for each ball in AthenaList (Scene+0x29D4):
       └─ Ball_Update (0x405E00)      ← ESI = ball pointer (MOV ESI,ECX at 0x405E20)
            ├─ SpatialTree_ctor (0x463330)          ← geometry collision tree (unaffected)
            ├─ CollisionNode_ctor (0x466CF0)       ← ball list reference ← PATCH A HERE
            ├─ CollisionMesh::AdvancePosition (0x4564C0)
            │    └─ traverse spatial tree
            │         └─ CollisionNode::CheckAllBalls (0x467030)  ← O(N) per ball
            └─ collision result iteration (0x406B3B)

Ball_AI_ChaseNearest (0x408390)      ← 8-ball AI tick (vtable[4])
  ├─ Ball_Update (0x405E00)          ← runs full physics
  └─ iterate ALL balls (0x4083D9)   ← PATCH B HERE

Ball_FindClosestRespawnPoint (0x405190) ← PATCH C HERE
  └─ RET at 0x405D83 (plain RET, 0 stack params)
```

### AthenaList Layout (from AthenaList_Append decompilation at 0x453810)

```
Scene + 0x29D4 = Ball AthenaList
  +0x00: unknown
  +0x04: count (int) — number of balls
  +0x08: iteration state (0x400 bytes = 256 ints)
  +0x40C: data pointer (array of ball pointers)
  +0x414: sorted mode flag
```

### Ball Struct Layout (from Ball_ctor2 at 0x4039E0)

```
Ball + 0x00: vtable pointer
Ball + 0x10: App pointer
Ball + 0x14: Scene pointer       ← used by Patch A and B to get ball count
Ball + 0x18: player_index
Ball + 0x1A4: CollisionMesh pointer
Ball + 0x164: position (x, y, z)
Ball + 0x170: velocity (x, y, z)
Ball + 0x284: radius
Ball + 0x768: alive flag
Ball + 0x80C: state (controls CollisionNode creation path)
```

All addresses verified via Ghidra decompilation + disassembly + memory reads.
