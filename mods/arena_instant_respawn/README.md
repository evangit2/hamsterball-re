# Arena Instant Respawn v9

## What It Does

ALL balls (players 1-4 + badballs) in arenas respawn instantly at the nearest safespot — no respawn platform, no crashes/freezes when many balls fall simultaneously.

## Problem

When 5+ balls fall simultaneously in arenas, the game crashes/freezes because:
- `Scene_StartCountdown` only handles ONE ball at a time (`scene+0x10F8`)
- `Ball_Respawn` arena path calls expensive `Mesh_FindClosestCollision` for each ball
- Multiple balls compete for the shared respawn platform

## Fix — Four Patches

| # | Address | Type | Effect |
|---|---------|------|--------|
| 1 | 0x437130 | Code cave: `RET 4` | Skip Scene_StartCountdown for ALL balls (no platform) |
| 2 | 0x40580B | JNZ→NOP | Force race-path search (no Mesh_FindClosestCollision) |
| 3 | 0x405A3D | JNZ→NOP | Scan ALL safespots (not just first match) |
| 4 | 0x405190 | Code cave | Copy ball pos→LGP (search from actual position) |

## Why This Prevents Crashes

- **No respawn platform** — no shared resource conflicts between balls
- **Race-path search** — O(n) distance calculation, no expensive raycast per ball
- **Independent respawn** — each ball finds its own nearest safespot without blocking
- **5+ simultaneous respawns** — all complete in microseconds, no frame freeze
