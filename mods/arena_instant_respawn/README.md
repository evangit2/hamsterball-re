# Arena Instant Respawn v1

## What it does
Replaces the spawn platform respawn system in arenas with instant teleport respawn for entity balls (BadBalls). When entity balls fall or break, they instantly teleport back to their original spawn position instead of going through the expensive SAFESPOT search via `Mesh_FindClosestCollision`.

## Problem
In arenas, when 5+ entity balls fall/respawn simultaneously, the game freezes. This is because `Ball_Respawn` (0x405190) calls `Mesh_FindClosestCollision` for each ball, and doing 5+ collision searches in one frame causes a massive frame spike.

## Solution
Two hooks intercept the fall/death path for entity balls:

1. **Ball_FallDeath (0x409480)** — vtable[8] for player-type balls (used by BADBALL entities). Instead of shattering the ball and setting death_pending, instantly teleports to stored spawn position at ball+0xC60/0xC64/0xC68.

2. **BadBall_StartFallCountdown (0x402390)** — vtable[8] for BadBall vtable balls. Same instant teleport, but only if ball+0xC60 contains a valid position (not the 1.0f countdown timer).

## What gets skipped
- Ball shattering (2 Ball_Split fragments + ArenaScoreParticle ring)
- Ball_Respawn's expensive SAFESPOT search (Mesh_FindClosestCollision)
- The freeze when 5+ balls respawn simultaneously

## What still works
- Player balls (ball+0x18 != -1) use original Ball_FallDeath — unaffected
- Split balls (ball+0x324 != 0) use original — unaffected
- BadBall vtable balls with timer at +0xC60 fall through to original countdown

## Ball offsets used
| Offset | Type | Description |
|--------|------|-------------|
| +0x18 | int | Player slot (-1 = entity ball) |
| +0x164/168/16C | float | Position X/Y/Z |
| +0x170/174/178 | float | Velocity X/Y/Z |
| +0x17C/180/184 | float | Force X/Y/Z |
| +0x1A4 | ptr | Physics object |
| +0x284 | float | Radius |
| +0x2E8 | byte | Death pending |
| +0x2E9 | byte | Falling flag |
| +0x2EC | dword | Bounce count |
| +0x2F9 | byte | Respawn invuln |
| +0x2FC | float | Alpha (0=opaque) |
| +0x300 | dword | Invuln timer |
| +0x310 | byte | Position update flag |
| +0x324 | byte | Split ball flag |
| +0xC60/0xC64/0xC68 | float | Stored spawn X/Y/Z (from CreateBadBall) |

## Game constants
- 0x4CF55C = float 5.0 (spawn position offset, used by CreateBadBall)

## Files
- `ArenaInstantRespawn.CEA` — Pure CEA script, ENABLE/DISABLE, no debugs
