# Global Bonk (Hammer) Mod

Spawns Bonk the Hammer on any level. Hammers chase ALL ball entities, physically move toward their targets, and smash without head-turning during the smash animation.

## Usage

1. Load `GlobalBonk.CEA` in Cheat Engine
2. Enable the script
3. Set `SpawnBonk` to 1 in CE (or use a hotkey)
4. Bonk spawns at player 1's position and activates when any ball comes within ~30 units

## v2 Fixes (June 2026)

### Issue 1: No head turn during smash
**Root cause:** The old script wrote player 1's position into the target fields (`+0x1120-0x1128`) every frame, even during smash state. This fought with the game's own target-finding logic and forced the hammer to always face player 1.

**Fix:** Removed ALL manual target writes. The game's `vtable[11]` (Bonk_Update at `0x43F930`) sets targets itself. During state 2 (smash), positions are frozen — no rotation drift.

### Issue 2: Hammer doesn't move (only rotates head)
**Root cause:** Two compounding bugs:
- (a) `vtable[11]` was never called by the script — the old version only manually wrote positions, never invoking the game's update function.
- (b) The original Bonk is a **stationary turret** — `current` moves toward `start`, but `start` was set to the spawn position, so `current` never moved toward the ball.

**Fix:**
- Call `vtable[11]` every frame (during player 1's Ball_Update for once-per-frame timing)
- After the call, if NOT smashing (state ≠ 2), copy target → start (`0x1120` → `0x1108`)
- This redirects the game's own movement code: `current` now moves toward the ball at 20%/frame

### Issue 3: Only targets player 1
**Root cause:** The old script filtered everything to `[esi+0x18]==0` (player 1 only) and overwrote the game's target with player 1's position. But `vtable[11]` already iterates the full ball list (`board+0x29D4`) and finds the nearest ball automatically.

**Fix:**
- Removed player-1 filter from proximity check — any ball can activate
- Do NOT override target positions — let `vtable[11]` find the nearest ball from ALL balls
- Activation state set to 0 (find target) so the game finds nearest ball on the very next frame

## Object Details

- **Name string:** "BONK" (0x4CFA4C)
- **Alloc size:** 0x1200 (4608 bytes)
- **Constructor:** 0x438850 (ret 0x10 — 4 stack params)
- **Vtable:** 0x4D5120
- **Mesh:** "levels\\level5-bonk" — loaded INTERNALLY by the constructor
- **Update function:** vtable[11] at offset 0x2C → 0x43F930

## Key Object Fields

| Offset | Size | Description |
|--------|------|-------------|
| +0x10D0 | 4 | Board pointer |
| +0x10D4 | 12 | Current position XYZ (float) |
| +0x10E0 | 12 | Home position XYZ (float) |
| +0x10F8 | 4 | CollisionLevel pointer |
| +0x10FC | 1 | Idle flag (1=idle, 0=active) |
| +0x10FD | 1 | Chasing flag (1=chasing) |
| +0x1100 | 4 | Timer/decay (init 1000, decays to 0) |
| +0x1104 | 4 | State machine: 0=find target, 1=approach, 2=smash |
| +0x1108 | 12 | Start position (where current moves toward) |
| +0x1120 | 12 | Target position (ball position, set by vtable[11]) |
| +0x1138 | 4 | Speed (starts 0.5, ×1.15/frame until ≥90 → smash) |
| +0x113C | 4 | Smash countdown (25 frames) |

## State Machine (vtable[11] @ 0x43F930)

```
State 0 (find target): speed -= 10.0
  When speed < 1.0:
    speed = 0.5, state → 1
    Iterate board+0x29D4 ball list, find NEAREST ball
    Set target (0x1120-0x1128) = ball position + radius offset
    Set start (0x1108-0x1110) = current position

State 1 (approach): speed *= 1.15
  Move home → target at dist*0.333/frame
  Move current → start at dist*0.2/frame
  When speed ≥ 90.0: SMASH
    Play BONKBASH sound, state → 2, countdown = 25
    Knockback all balls within 80.0 units

State 2 (smash): countdown -= 1, when 0 → state 0

Rendering (every frame):
  Gfx_RotateY(home.x - current.x, 0, home.z - current.z)
  Gfx_SetPosition(current.x, current.y - timer, current.z)
```

## How v2 Makes the Hammer Chase

The original game's Bonk is stationary — `current` moves toward `start`, but `start` = spawn position, so the hammer never physically relocates. v2 fixes this by redirecting `start = target` after each `vtable[11]` call (when not smashing). This makes the game's own movement code move `current` toward the target ball at 20% of remaining distance per frame (~0.25s to reach 95%).

## No External Dependencies

- Mesh loaded internally by constructor (no Board+0x43xx needed)
- CollisionLevel created internally
- Only appends to Board+0x2578 (always init'd)
- Board+0x436C is saved/restored (slot JIT pattern)

## Hook Point

- Address: 0x405E22 (inside Ball_Update, ESI = current ball)
- Original: `mov eax, [esi+0x0C5C]` (6 bytes: 8B 86 5C 0C 00 00)
- Player 1 filter (`[esi+0x18]==0`) used only for: vtable[11] call (once per frame), spawn trigger, position save
- Proximity check: no player filter — any ball can activate idle bonks
