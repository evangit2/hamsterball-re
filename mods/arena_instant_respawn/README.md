# Arena Instant Respawn v7

## What It Does

Makes entity balls in arenas respawn at the nearest safespot to where they died — exactly like player 1 respawns in races. No respawn platforms.

## Root Cause (found after v1-v6)

**TWO callers** invoke Ball_Respawn in arenas:
1. **ArenaBoard_Update** (0x420DA0) — checks `ball_Y < ball+0x2D0 - radius * mult` (PRIMARY — fires when ball falls off arena)
2. **Scene_UpdateBallsAndState** (0x41B540) — checks `ball+0x2E8` (death_pending) (SECONDARY)

**PLUS** the SinkPlatform system calls `Scene_StartCountdown` which puts the ball on a respawn platform, conflicting with Ball_Respawn.

**AND** entity balls have LGP=(0,0,0) (never initialized), so Ball_Respawn's safespot search computes distance from origin → all balls pick the same nearest safespot.

**AND** Ball_Respawn's arena path uses random safespots, and the arena early-break accepts the first match instead of scanning all.

## Four Patches — All Needed

| # | Address | Type | Effect |
|---|---------|------|--------|
| 1 | 0x40580B | JNZ→NOP | Force race-path search (nearest, not random) |
| 2 | 0x405A3D | JNZ→NOP | Scan ALL safespots (not just first match) |
| 3 | 0x405190 | Code cave | Copy ball pos→LGP at Ball_Respawn entry (for both callers) |
| 4 | 0x43716B | Code cave | Intercept entity balls in Scene_StartCountdown (prevent platform) |

### Patch 3 — Ball_Respawn Entry Cave
At Ball_Respawn entry (0x405190), before the function body:
- Execute original `SUB ESP, 0x84`
- Copy `ball+0x164/168/16C` (current X/Y/Z) → `ball+0x2DC/2E0/2E4` (LGP X/Y/Z)
- Jump back to 0x405196 (PUSH EBX)

This ensures both callers (ArenaBoard_Update and Scene_UpdateBallsAndState) benefit from correct LGP.

### Patch 4 — Scene_StartCountdown Intercept
At the entity-ball path in Scene_StartCountdown (0x43716B):
- Clear `scene+0x10F1` (undo countdown-active flag)
- Copy ball position → LGP
- Call `Ball_Respawn(ball)` directly
- Return immediately (`RET 4`)

This prevents the SinkPlatform from putting entity balls on the respawn platform.

## Version History

| Version | Approach | Result |
|---------|----------|--------|
| v1 | Hook Ball_FallDeath | Wrong path |
| v2 | Patch Scene_UpdateBallsAndState | Redundant |
| v3 | Force race path | Glitchy — LGP=(0,0,0) |
| v4 | Scan all safespots | Still glitchy — LGP=(0,0,0) |
| v5 | Copy pos→LGP at Ball_Respawn entry | Still on platforms |
| v6 | Intercept in Scene_StartCountdown | Missing pos→LGP for ArenaBoard_Update path |
| v7 | ALL FOUR patches together | Correct — both callers + both systems |
