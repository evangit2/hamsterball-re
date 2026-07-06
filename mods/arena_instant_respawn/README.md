# Arena Instant Respawn v2

## What It Does

Makes ALL entity balls in arenas (BadBalls, AI balls) respawn the same way player 1 respawns in races — via `Ball_Respawn` (0x405190), which uses the ball's LGP (last grounded position) to find the nearest safespot and teleports the ball there on the ground.

**Original behavior:** Entity balls (ball+0x18 == -1) that fall off the arena are destroyed, then a new ball is spawned via CreateBadBall at the original spawn position. This causes freezes when 5+ balls trigger the expensive Mesh_FindClosestCollision simultaneously.

**Patched behavior:** ALL balls go through `Ball_Respawn`, which:
1. Searches the safespot list (board+0x1518) for the nearest spawn point to the ball's LGP
2. Teleports the ball to the safespot position on the ground (Y = safespot_y + radius)
3. Zeroes all velocity
4. Sets respawn timer (150 frames)
5. Clears death flags, makes ball briefly invisible

## How It Works

`Scene_UpdateBallsAndState` (0x41B540) iterates two ball lists per frame:

**List 1 (scene+0x29D4)** — primary balls (players + arena entity balls):
- At 0x41B5A9: `JNZ 0x41B5CD` — only player balls (ball+0x18 != -1) go to Ball_Respawn
- Entity balls fall through to destroy path (unless App+0x237 arena flag is set)

**List 2 (scene+0x3204)** — secondary balls:
- At 0x41B64E: `JZ 0x41B659` — entity balls always jump to destroy path

### Patches

| Address | Original | Patched | Effect |
|---------|----------|---------|--------|
| 0x41B5A9 | `75 22` (JNZ) | `EB 22` (JMP) | List 1: ALL non-split balls go to Ball_Respawn |
| 0x41B64E | `74 09` (JZ) | `90 90` (NOP) | List 2: entity balls fall through to Ball_Respawn |

## Why v1 Failed

v1 hooked `Ball_FallDeath` (0x409480) and `BadBall_StartFallCountdown` (0x402390) — these are vtable[8] handlers called when balls fall. But the actual arena respawn flow doesn't go through these for the freeze issue. The real flow is:

1. Ball falls off arena → death_pending flag (ball+0x2E8) set
2. `Scene_UpdateBallsAndState` checks ball+0x2E8 next frame
3. If entity ball: calls `AthenaList_Remove` + `ball_dtor` → ball destroyed
4. `CreateBadBall` spawns new ball → expensive `Mesh_FindClosestCollision`

v1 hooks never fired because the balls were destroyed before reaching the vtable[8] handlers. v2 patches the condition check directly in `Scene_UpdateBallsAndState` so entity balls take the Ball_Respawn path instead.

## Files

- `ArenaInstantRespawn.CEA` — CE Auto Assembler script (pure CEA, no Lua)
