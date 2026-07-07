# Arena Instant Respawn v5

## What It Does

Makes entity balls in arenas respawn at the nearest safespot to where they actually died — exactly like player 1 respawns in races.

## Root Cause (found after v1-v4)

Entity balls have LGP (last grounded position, ball+0x2DC/0x2E0/0x2E4) = **(0,0,0)** because it's never initialized by CreateBadBall or Ball_ctor. LGP is only updated in Ball_Update when the ball touches ground (type 2 collision + ball+0x768 flag). If a ball falls before touching ground, LGP stays (0,0,0).

Ball_Respawn searches for the nearest safespot using LGP as the search origin. With LGP=(0,0,0), ALL entity balls compute distance from (0,0,0) → ALL pick the SAME nearest safespot → all balls respawn on one platform.

- **v1** (failed): Hooked Ball_FallDeath — wrong code path
- **v2** (failed): Patched Scene_UpdateBallsAndState — redundant no-ops
- **v3** (glitchy): Forced race path, but LGP was still (0,0,0)
- **v4** (glitchy): Scanned all safespots, but LGP was still (0,0,0)
- **v5** (this): Copies ball's current position to LGP at Ball_Respawn entry

## Three Patches

| # | Address | Type | Effect |
|---|---------|------|--------|
| 1 | 0x40580B | JNZ→NOP (6 bytes) | Use race-path search instead of arena random path |
| 2 | 0x405A3D | JNZ→NOP (6 bytes) | Scan ALL safespots, not just first match |
| 3 | 0x405190 | Code cave | Copy ball pos (0x164/168/16C) → LGP (0x2DC/2E0/2E4) before search |

### Code Cave Detail

At Ball_Respawn entry (0x405190), before the function body runs:
1. Execute original `SUB ESP, 0x84`
2. Copy `ball+0x164` (pos X) → `ball+0x2DC` (LGP X)
3. Copy `ball+0x168` (pos Y) → `ball+0x2E0` (LGP Y)
4. Copy `ball+0x16C` (pos Z) → `ball+0x2E4` (LGP Z)
5. Jump back to 0x405196 (original second instruction)

This ensures the safespot search uses the ball's actual death position as origin, so different balls at different positions pick different safespots.

## How It Works (all three patches together)

1. Entity ball falls off arena → Ball_FallDeath sets death_pending
2. Scene_UpdateBallsAndState calls Ball_Respawn
3. **Code cave** copies ball position to LGP
4. **Patch 1** forces race-path search (nearest safespot, not random)
5. **Patch 2** scans all safespots (not just first match)
6. Ball teleports to nearest safespot, becomes invisible
7. Respawn timer (150 frames), then ball fades back in
