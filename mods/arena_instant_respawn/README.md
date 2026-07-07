# Arena Instant Respawn v4

## What It Does

Makes entity balls in arenas respawn the same way player 1 respawns in races — finding the NEAREST safespot to the ball's LGP (last grounded position), not just the first one in the list.

## Root Cause Analysis

**Ball_Respawn (0x405190)** has TWO arena-specific behaviors that break proper respawn:

### Bug 1: Arena random-safespot path (found in v3)
At `0x40580B`, `JNZ 0x405A80` branches to a random-safespot search when `App+0x237` (arena flag) is set. The arena random path picks a random safespot and validates it with `Mesh_FindClosestCollision` — which is expensive and often fails, causing the ball to not respawn at all.

**v3 fix:** NOP the `JNZ` at `0x40580B` → forces race-path search (nearest safespot via LGP).

### Bug 2: Arena early-break in race path (found in v4, the v3 glitch)
Even after v3 forces the race path, there's a SECOND arena check at `0x405A3D` INSIDE the search loop. When `App+0x237` (arena flag) is set, the loop accepts the FIRST acceptable safespot and breaks immediately — it never scans the rest of the list to find the actually nearest one.

This is why v3 caused "all balls respawn on just one platform" — every ball picked the same first safespot in the list.

**v4 fix:** NOP the `JNZ` at `0x405A3D` → loop scans ALL safespots and picks the truly nearest one.

## Patches

| # | Address | Original | Patched | Effect |
|---|---------|----------|---------|--------|
| 1 | 0x40580B | `0F 85 6F 02 00 00` (JNZ) | `90 90 90 90 90 90` (NOP) | Use race-path search instead of arena random path |
| 2 | 0x405A3D | `0F 85 89 02 00 00` (JNZ) | `90 90 90 90 90 90` (NOP) | Scan all safespots instead of breaking after first match |

## How Race Respawn Works (the path now used fully)

1. Ball_Respawn reads LGP from `ball+0x2DC` (X), `ball+0x2E0` (Y), `ball+0x2E4` (Z)
2. Iterates ALL safespots in `scene+0x1518` AthenaList (populated from MESHWORLD "SAFESPOT" entries)
3. For each safespot: calculates 3D distance from LGP to safespot position
4. Tracks the minimum-distance safespot across the ENTIRE list (no early break)
5. Teleports ball to nearest safespot: `pos = safespot_pos`, `Y = safespot_Y + radius`
6. Zeroes all velocity (X, Y, Z)
7. Sets respawn timer (150 frames = 2.5 seconds at 60fps)
8. Clears death flags, makes ball briefly invisible

## Version History

| Version | Approach | Result |
|---------|----------|--------|
| v1 | Hook Ball_FallDeath + BadBall_StartFallCountdown | Did nothing — wrong code path |
| v2 | Patch Scene_UpdateBallsAndState conditions | Did nothing — redundant no-ops |
| v3 | NOP JNZ at 0x40580B (arena random path) | Glitchy — all balls respawned on same platform |
| v4 | Also NOP JNZ at 0x405A3D (arena early break) | Correct — scans all safespots for nearest |
