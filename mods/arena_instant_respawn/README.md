# Arena Instant Respawn v3

## What It Does

Makes entity balls in arenas respawn the same way player 1 respawns in races — using the nearest safespot found via LGP (last grounded position).

## Root Cause (why v1 and v2 failed)

**Ball_Respawn (0x405190)** has TWO safespot search paths:

| Path | Address | Behavior |
|------|---------|----------|
| Race path | 0x405811 | Finds NEAREST safespot using LGP (ball+0x2DC/0x2E0/0x2E4) |
| Arena path | 0x405A80 | Picks RANDOM safespot + validates with Mesh_FindClosestCollision |

At **0x40580B**, Ball_Respawn checks `App+0x237` (arena flag). If arena mode, it takes the arena path (random + validation). The arena path:
1. Picks a random safespot index via `CPUID_CheckProcessorFeature` (used as RNG)
2. Validates it with `Mesh_FindClosestCollision` — expensive and can fail
3. If validation fails, loops back to pick another random safespot
4. If all fail, `local_84` stays NULL → falls through to `LAB_00405d1d` which just zeroes velocity **without moving the ball**

This is why entity balls "did nothing" — Ball_Respawn was called but the arena random-safespot path failed to find a valid spot, so the ball stayed where it was.

### v1 (failed)
Hooked `Ball_FallDeath` (0x409480) and `BadBall_StartFallCountdown` (0x402390) — wrong path. Entity balls are handled by `Scene_UpdateBallsAndState` before reaching vtable[8] handlers.

### v2 (failed)
Patched `Scene_UpdateBallsAndState` conditions at 0x41B5A9 and 0x41B64E — redundant. Entity balls in arenas already go through Ball_Respawn via the `App+0x237` check at 0x41B5B9. The patches were no-ops.

### v3 (this version)
Single patch inside Ball_Respawn itself: NOP out the `JNZ` at 0x40580B that branches to the arena random-safespot path. This forces Ball_Respawn to ALWAYS use the race path (nearest safespot via LGP), even in arena mode.

Arena levels DO have SAFESPOT entries (verified by scanning MESHWORLD files — all 17 arena levels contain "SAFESPOT" string entries).

## Patch

| Address | Original | Patched | Effect |
|---------|----------|---------|--------|
| 0x40580B | `0F 85 6F 02 00 00` (JNZ 0x405A80) | `90 90 90 90 90 90` (NOP×6) | Always use race-path safespot search |

## How Race Respawn Works (the path now used)

1. Ball_Respawn reads LGP from `ball+0x2DC` (X), `ball+0x2E0` (Y), `ball+0x2E4` (Z)
2. Iterates safespot list (`scene+0x1518` AthenaList, populated from MESHWORLD "SAFESPOT" entries)
3. For each safespot: calculates 3D distance from LGP to safespot
4. Picks the nearest safespot (minimum distance)
5. Teleports ball to safespot position: `pos = safespot_pos`, `Y = safespot_Y + radius`
6. Zeroes all velocity (X, Y, Z)
7. Sets respawn timer (150 frames = 2.5 seconds at 60fps)
8. Clears death flags, makes ball briefly invisible
