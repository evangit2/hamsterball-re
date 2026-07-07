# Arena Instant Respawn v6

## What It Does

Makes entity balls in arenas respawn at the nearest safespot (like player 1 in races) instead of on respawn platforms.

## Root Cause (found after v1-v5)

The SinkPlatform system catches entity balls BEFORE Ball_Respawn runs. `SinkPlatformArenaCollisionEvents` calls `Scene_StartCountdown`, which puts the ball on the respawn platform with a countdown timer. Ball_Respawn was being called later by `Scene_UpdateBallsAndState`, but by then the SinkPlatform countdown was already running — causing conflicts.

## Fix (v6)

Intercept entity balls INSIDE `Scene_StartCountdown` (0x437130). At the entity-ball path (0x43716B), instead of setting timer=50 and putting ball on platform, redirect to a code cave that:
1. Clears `scene+0x10F1` (undo countdown-active flag)
2. Copies ball position → LGP (so search uses actual death position)
3. Calls `Ball_Respawn(ball)` directly (teleport to nearest safespot)
4. Returns immediately (skips all platform setup)

## Patches

| # | Address | Original | Patched | Effect |
|---|---------|----------|---------|--------|
| 1 | 0x40580B | `0F 85 6F 02 00 00` (JNZ) | `90×6` (NOP) | Force race-path search in Ball_Respawn |
| 2 | 0x405A3D | `0F 85 89 02 00 00` (JNZ) | `90×6` (NOP) | Scan ALL safespots, not first match |
| 3 | 0x43716B | `C7 81 F4 10 00 00 32 00 00 00` | `JMP cave + NOP×5` | Intercept entity balls in Scene_StartCountdown |

## Code Cave (Patch 3)

At `Scene_StartCountdown` entity-ball path, the code cave:
- Clears the countdown-active flag that was just set
- Copies ball position (0x164/168/16C) → LGP (0x2DC/2E0/2E4)
- Calls Ball_Respawn(ball) which teleports to nearest safespot
- Returns from Scene_StartCountdown immediately (RET 4)

## Version History

| Version | Approach | Result |
|---------|----------|--------|
| v1 | Hook Ball_FallDeath | Wrong path |
| v2 | Patch Scene_UpdateBallsAndState | Redundant |
| v3 | NOP arena path in Ball_Respawn | Glitchy — all balls same platform |
| v4 | Also NOP arena early-break | Still glitchy — same issue |
| v5 | Code cave: copy pos→LGP at Ball_Respawn entry | Still on respawn platforms |
| v6 | Intercept in Scene_StartCountdown before platform | Correct — bypasses platform entirely |
