> **⚠️ Status: Untested** — This is an HB+ API port that has not yet been crash-tested or verified in-game.

# Jump Mod (HB+ v2.1)

Press SPACE to jump (Player 1 only, raycast ground detection).

Port of bass.dll proxy jump_mod v22 to HB+ API.

## Features

- **Raycast ground detection:** Casts a ray straight down using `LevelRaycastHit`. If the hit is within `radius * 1.45` of the ball, the ball is grounded and can jump.
- **Countdown gating:** No jumping during Ready/Set/Go countdown (`Scene+0x3A4C == 0`).
- **Race-end gating:** No jumping after touching the goal (`ball+0x14C == 1` or `App+0x5D6 == 1`).
- **Rising-edge detection:** One jump per keypress (no auto-repeat).
- **Adjustable jump force** via Options menu slider (5.0 - 50.0, default 20.0).
- Toggle on/off with the "Jump (Space)" button.

## Differences from bass.dll proxy version

| Component | bass.dll (v22) | HB+ (this) |
|---|---|---|
| Input polling | Background thread + DirectInput | `WasKeyPressed(DIK_SPACE)` in `onBallUpdate` |
| Raycast | `Mesh_FindClosestCollision` (raw call) | `LevelRaycastHit` (HB+ API wrapper) |
| Impulse application | Phase 15 code cave (asm FLD/FADD/FSTP) | Direct write to `ball+0x174` |
| BASS proxy | ~150 lines | Removed |
| Config | Hardcoded | HB+ slider (Jump Force) |
| Code size | 589 lines | ~130 lines |

Author: RodentRacer / Hamsterbot
