# Arena Instant Respawn v10

## What It Does

ALL balls (Players 1-4 + badballs) in arenas respawn the same way player 1 respawns in races — nearest safespot via LGP, no respawn platform.

## Race Respawn Flow (what we copy)

1. Ball falls → Ball_Update sets falling flag → calls Ball_FallDeath
2. Ball_FallDeath sets ball+0x2E8=1 (falling), creates visual fragments
3. Next frame: Scene_UpdateBallsAndState sees 0x2E8=1 → calls Ball_Respawn
4. Ball_Respawn searches safespot list using LGP (ball+0x2DC/2E0/2E4)
5. Finds NEAREST safespot, teleports ball there
6. Ball becomes invisible, respawns after 150 frames

## Arena Differences (what we fix)

- **SinkPlatform**: catches balls, puts them on respawn platform → **skip entirely**
- **Arena search**: picks RANDOM safespot + validates with expensive Mesh_FindClosestCollision → **force race path** (nearest, no raycast)
- **Arena early-break**: accepts first match → **scan ALL safespots**
- **Entity ball LGP**: uninitialized (0,0,0) → **copy ball position to LGP at Ball_Respawn entry**

## Patches

| # | Address | Effect |
|---|---------|--------|
| 1 | 0x40580B | NOP JNZ — force race-path search (not arena random) |
| 2 | 0x405A3D | NOP JNZ — scan ALL safespots (not just first match) |
| 3 | 0x405190 | Code cave — copy ball pos→LGP before search |
| 4 | 0x437130 | RET 4 — skip Scene_StartCountdown for ALL balls (no platform) |
