# Hamsterball Dizzy/Stun System — Complete Analysis

## Overview

When a ball falls off the level, gets swallowed by a pipe, or is otherwise knocked off the
playfield, the game triggers a **stun state** ("dizzy"). The ball respawns at the nearest
checkpoint, becomes semi-transparent (fades in), is surrounded by circling stars, and the
player loses all control for approximately 4 seconds (at default 25 FPS).

## Three Separate Control-Loss Systems

The game has **three independent systems** that can disable player input:

### 1. Respawn Stun ("Dizzy") — The main system the user asked about

| Field | Offset | Type | Description |
|-------|--------|------|-------------|
| `show_stars` | +0x2F8 | byte | 1 = render 8-star circling effect |
| `is_stunned` | +0x2F9 | byte | 1 = blocks ALL force application (input disabled) |
| `alpha` | +0x2FC | float | 0.0 (transparent) → 1.0 (opaque). Ball fade-in |
| `stun_timer` | +0x300 | int32 | 150 → 0 countdown. Controls star duration |
| `is_falling` | +0x2E8 | byte | 1 = ball is off-edge, triggers respawn |
| `respawn_state` | +0x324 | byte | 1 = ball is in respawn sequence |

### 2. Impact Count (post-collision)

| Field | Offset | Type | Description |
|-------|--------|------|-------------|
| `impact_count` | +0x2F0 | int32 | 0=normal, ≥81=input blocked, 100=after trajectory hit |

Set to 100 (0x64) by `Ball_ApplyTrajectory` (0x403750) when the ball is launched by a
trajectory event (E:TRAJECTORY, E:JUMP). Decremented by 1 per frame in `Ball_Update`.
When `impact_count >= 81` (0x51), `Ball_ApplyForceWithMultipliers` and `Ball_ApplyForceV2`
**reject all force input** — the ball can't be steered.

- At 25 FPS: 100-81 = 19 frames = **0.76 seconds** of no input
- When impact_count is non-zero but < 81: force is multiplied by 0.25 (quarter power)

### 3. No-Control Timer

| Field | Offset | Type | Description |
|-------|--------|------|-------------|
| `no_control_timer` | +0x808 | int32 | Non-zero = input blocked |

Set by collision events:
- `E:JUMP` → 10 frames (0.4s at 25fps)
- Board collision handlers → 50 frames (2.0s) or 1000 frames (40s, semi-permanent)
- `N:NOCONTROL` → 10 frames

## The Dizzy Trigger Chain

### Step 1: Ball is knocked off the level

Multiple triggers set `is_falling` (+0x2E8 = 1):

- **Ball_Shatter** (0x408D70) — called from `FollowBall_Update` (0x43ECC0) when the ball
  falls off edges or hits certain kill zones. Sets +0x324=1, +0x2E8=1.
- **Ball_Shatter_OnRamp** (0x409480) — variant for ramp-based shatters.
  Sets +0x324=1, +0x2E8=1.
- **E:SWALLOW** collision event — pipe mechanics swallow the ball. Sets +0x2E8=1.
- **Ground contact loss** — `ball+0xC60` (ground contact value) starts at 1.0, decrements
  by 0.02/frame when airborne. After 50 frames (2 seconds at 25fps) of no ground contact,
  it drops below 0, triggering `is_falling = 1`.
  (Code in `FUN_004031b0` at 0x4031B0 and `Ball_Update` at 0x405E00.)

### Step 2: Scene detects falling ball → calls respawn

In `Scene_UpdateBallsAndState` (0x41B5CF):
```
for each ball in board+0x29D4 (ball list):
    ball.vtable[4]()  // update ball
    if (ball+0x2E8 != 0):  // is_falling
        if (ball+0x324 == 0 && ball+6 != -1):  // not already respawning
            Ball_FindClosestRespawnPoint(ball)
        else:
            destroy ball (vtable[0])
```

### Step 3: Ball_FindClosestRespawnPoint sets the stun

`Ball_FindClosestRespawnPoint` (0x405190) does:
1. Finds the nearest respawn point (from board+0x1518 AthenaList)
2. Teleports ball to that position
3. **Sets the stun state:**
   ```
   ball+0x2F8 = 1        // show_stars (enable star circling render)
   ball+0x2F9 = 1        // is_stunned (BLOCKS all input via force guard)
   ball+0x2FC = 0.0      // alpha (ball starts invisible)
   ball+0x300 = 150      // stun_timer (150 frame countdown)
   ball+0x76A = 0        // clear some flag
   ball+0x31C = 0        // clear flags
   ball+0x31D = 0
   ```
4. Zeros velocity (forces at +0x170/174/178 = 0)
5. Resets collision mesh

### Step 4: Stun recovery (in Ball_Update, 0x405E00)

Each frame while stunned:
```
if (is_stunned != 0):
    alpha += 0.01            // _DAT_004cf524 = 0.01
    if (alpha > 1.0):        // _DAT_004cf310 = 1.0
        alpha = 1.0
        is_stunned = 0       // CLEAR stun — input restored!

stun_timer -= 1              // decrement, clamp at 0
if (stun_timer == 0):
    show_stars = 0           // stop rendering stars
```

**Timeline at 25 FPS:**
- Frames 0-100: Ball fades in (alpha 0→1.0 at 0.01/frame). Input blocked.
- Frame 100: `is_stunned` cleared. **Player regains control** (4.0 seconds).
- Frames 100-150: Stars continue circling (stun_timer still counting down).
- Frame 150: `show_stars` cleared. Stars disappear (6.0 seconds total).

**Timeline at 144 FPS (with FPS mod):**
- Frame 100: `is_stunned` cleared after only **0.69 seconds** (not 4 seconds!)
- Frame 150: Stars gone after **1.04 seconds** (not 6 seconds!)
- The FPS mod dramatically shortens the dizzy duration.

## How Stun Blocks Input

Both force-application functions check the stun flag FIRST:

```c
// Ball_ApplyForceWithMultipliers (0x402650)
// Ball_ApplyForceV2 (0x4016F0)
void Ball_ApplyForce(ball* this, float dx, float dy, float dz, float multiplier) {
    if (this->is_stunned)   return;  // +0x2F9: STUNNED → no force
    if (this->block_flag)   return;  // +0x2CC: blocked → no force
    if (this->no_control)   return;  // +0x808: no-control timer active → no force
    if (this->impact_count >= 81) return;  // +0x2F0: post-impact → no force

    // Apply multipliers based on state:
    if (impact_count != 0)  multiplier *= 0.25;   // quarter power after hit
    if (respawn_state)      multiplier *= 0.0;     // zero power during respawn
    if (on_ground)          multiplier *= 0.20;    // reduced in tube
    if (is_tarred)          multiplier *= 0.75;    // tar slowdown

    this->vel_x += dx * multiplier;  // +0x170
    this->vel_y += dy * multiplier;  // +0x174
    this->vel_z += dz * multiplier;  // +0x178
}
```

## Ball_Shatter — The "Hit a Wall Too Hard" Trigger

`Ball_Shatter` (0x408D70) is called from `FollowBall_Update` (0x43ECC0) when the ball
leaves the playable area. It:

1. Sets `is_falling = 1` (+0x2E8)
2. Sets `respawn_state = 1` (+0x324) — gates the respawn fade sequence
3. Plays shatter sound
4. Creates 3 `Ball_Split` fragments that fly off in different directions
5. Creates trail particles
6. Calls `Scene_ForEachBall_SetVelocity` to push other balls

`Ball_Shatter_OnRamp` (0x409480) is a variant for ramp-based deaths that creates
2 fragments and spawns score particles in a circular pattern.

## E:NODIZZY — NOT a Dizzy Trigger

The `E:NODIZZY` collision event name is misleading — it does NOT prevent or remove
the dizzy state. It actually calls `Ball_RecordBest` with a TIME tag, recording the
player's best time for that section. The "NODIZZY" name likely means "this section
doesn't make you dizzy" (i.e., it's a safe zone checkpoint), not "cure dizziness."

## Visual Effects During Stun

In `Ball_RenderAI` (0x403DC0):

1. **Star circling** (when `show_stars` = 1):
   - 8 star sprites rendered at 45° intervals around the ball
   - Uses sin/cos for circular positioning
   - Radius converges as stun_timer decreases (stars spiral inward)
   - Rendered through the post-alpha pass (translucent)

2. **Ball fade-in** (when `alpha` < 1.0):
   - `SceneObj_SetScale` passes alpha as the W-component for alpha blending
   - Ball is semi-transparent during respawn, becomes opaque as alpha → 1.0

3. **Velocity zeroed** — ball doesn't drift during stun, all forces are 0.

## Key Addresses

| Address | Function | Role |
|---------|----------|------|
| 0x405190 | Ball_FindClosestRespawnPoint | Triggers stun (sets all stun fields) |
| 0x405D5B | (inside above) | Sets is_stunned=1, stun_timer=150 |
| 0x408D70 | Ball_Shatter | Sets is_falling, respawn_state |
| 0x409480 | Ball_Shatter_OnRamp | Ramp variant of shatter |
| 0x405E00 | Ball_Update | Stun recovery (alpha+0.01/frame, clear at 1.0) |
| 0x402650 | Ball_ApplyForceWithMultipliers | Input guard (checks is_stunned first) |
| 0x4016F0 | Ball_ApplyForceV2 | Alternate input guard (same checks) |
| 0x41B5CF | Scene_UpdateBallsAndState | Detects is_falling → calls respawn |
| 0x403DC0 | Ball_RenderAI | Renders star effect + alpha |
| 0x40C5D0 | DispatchCollisionEvents | E:NODIZZY (records best time, NOT dizzy cure) |

## Field Summary

| Offset | Name | Type | Default | Description |
|--------|------|------|---------|-------------|
| +0x2E8 | is_falling | byte | 0 | Set by Ball_Shatter etc. Triggers respawn |
| +0x2F0 | impact_count | int32 | 0 | Set to 100 by trajectory. ≥81 blocks input |
| +0x2F8 | show_stars | byte | 0 | 1 = render 8-star circling effect |
| +0x2F9 | is_stunned | byte | 0 | 1 = blocks ALL force application |
| +0x2FC | alpha | float | 1.0 | 0=transparent, 1=opaque. Fades in on respawn |
| +0x300 | stun_timer | int32 | 0 | 150 on stun. Countdown controls star duration |
| +0x324 | respawn_state | byte | 0 | 1 = ball in respawn sequence |
| +0x808 | no_control_timer | int32 | 0 | Non-zero = input blocked |
| +0xC60 | ground_contact | float | 1.0 | Decays by 0.02/frame when airborne → triggers fall |
