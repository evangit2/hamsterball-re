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

## E:NODIZZY — NOT Related to the Dizzy System

The `E:NODIZZY` collision event (string at 0x4CF8B8) is checked in
`DispatchCollisionEvents` (0x40C5D0) at 0x40C64B. Its handler:
1. Creates a `Sprite_DrawColoredRect` (0x4694F0) — visual effect
2. Iterates a list via 0x469510/0x469600
3. For each "TIME" entry: calls `Ball_RecordBest` (0x402400)
4. Removes entries via vtable[0](1)

**E:NODIZZY does NOT touch ball+0x2E9 (dizzy_lock) or ball+0x2EC (bounce_count).**
It clears TIME checkpoint entries and records best times. The name likely means
"this section doesn't make you dizzy" (a safe-zone checkpoint), not "cure dizziness."

## The Bounce-Induced "Dizzy" Counter (End Screen)

### End Screen Display

The time-trial end screen ("DIZZIED BALLS:" at 0x4D6DC0) reads:
- `*(App + player_index * 0xA0 + 0x5F8)` — per-player dizzy count (int32)

The adjacent "BROKEN BALLS:" (0x4D6DD0) reads:
- `*(App + player_index * 0xA0 + 0x5F4)` — per-player broken count (int32)

These per-player data blocks start at `App + 0x5CC + pIdx * 0xA0` and are added to
`board+0x362C` (AthenaList) during `Board_ctor` (0x419030, line 124).

### Ball_Update Two-Pass Collision Architecture

`Ball_Update` (0x405E00) processes collisions in TWO separate passes over the
same collision list (physics_body+0x424):

**Pass 1** (line 495, `while (piVar16 != 0)`):
- Line 506: `if (bounce_count > 1 AND ball+0x2E9 == 0)` → `Ball_ApplyTrajectory(ball)`
- This is the CHECK pass — it only fires if bounce_count was already incremented

**Pass 2** (line 686, `while (piVar16 != 0)`):
- Line 805: `if (collision_speed > 1.0 AND !is_shrunk)` → `ball+0x2E9 = 1` + camera change
- Line 820: `if (collision_speed >= 0.03 AND bounce_count == 0)` → `bounce_count++` (0→1)
  - Also does a raycast (Mesh_FindClosestCollision) to verify the ball moved significantly
  - Checks `ball->radius * 3.0 < distance_to_collision_point`
- Line 847: `if (bounce_count != 0 AND !is_shrunk AND collision_speed >= 0.1)` → `bounce_count++` (1→2)

### Can You Get Dizzied on Your First Fall? — YES

The bounce counter is **double-incremented in a single frame** (0→1→2) during Pass 2:
1. First increment: speed ≥ 0.03 AND bounce_count == 0 → bounce_count = 1
2. Second increment: bounce_count != 0 (now 1) AND speed ≥ 0.1 → bounce_count = 2

Both happen in the same loop iteration. On the NEXT frame, Pass 1 sees
bounce_count = 2 and fires `Ball_ApplyTrajectory` — **effectively dizzying
the ball from a single qualifying bounce**.

### Speed > 1.0 Blocks the Counter

If collision speed > 1.0 (hard hits from long falls):
- Pass 2 sets `ball+0x2E9 = 1` (dizzy_lock) BEFORE the bounce counter can trigger
  the trajectory on the next frame
- This BLOCKS `Ball_ApplyTrajectory` from firing (Pass 1 checks `ball+0x2E9 == 0`)
- The ball gets a camera change but NO dizzy counter increment, NO speed halving,
  and NO impact_count = 100

So the end-screen "DIZZIED BALLS" counter only counts **medium-speed bounces**
(0.1 ≤ speed ≤ 1.0), not hard impacts from long falls.

### ball+0x2E9 (dizzy_lock) Lifecycle

Set to 1 by:
- `Ball_ApplyTrajectory` (0x403750) — after trajectory effect applied
- Speed > 1.0 collision check (Ball_Update line 806)
- `E:LIMIT` / `E:LIMITX` / `E:LIMITZ` — level boundary events
- `E:LIMITPIPE1` / `E:LIMITPIPE2` — pipe limit events
- `E:SWALLOW` — fall off edge

Reset to 0 ONLY by `Ball_InitPhysicsDefaults` (0x405100) — called on respawn/new race.

### Ball_ApplyTrajectory (0x403750) — The Dizzy Effect

When triggered:
1. Reads trajectory direction from `physics_body+0xCA4/CA8/CAC`
2. Normalizes and scales velocity by 0.5 (`_DAT_004CF3F0` = 0.5)
3. Damps Y velocity by 1.25× (`_DAT_004CF434` = 1.25)
4. Plays boost sound
5. Sets `ball+0x2F0 = 100` (impact_count → wobble + control loss)
6. Creates trail particles
7. Sets `ball+0x2E9 = 1` (dizzy_lock)
8. Sets `ball+0x14D = 1` (has_trajectory flag)
9. If `ball+0x18 (player_index) != -1`:
   - Increments `*(App + pIdx * 0xA0 + 0x5F8)` — **THE DIZZY COUNTER**

### Bounce Counter Reset

`ball+0x2EC` (bounce_count) is reset to 0 when:
- `ball+0x14D` (has_trajectory) is set (Ball_Update line 642-644) — next frame after trajectory
- `Ball_RecordBest` (0x402400) — clears to 0
- `Ball_InitPhysicsDefaults` (0x405100) — full physics reset

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
| 0x405E00 | Ball_Update | Two-pass collision: Pass 1 checks bounce_count, Pass 2 increments. Also stun recovery |
| 0x403750 | Ball_ApplyTrajectory | The dizzy effect: halves speed, sets impact_count=100, increments dizzy counter |
| 0x402650 | Ball_ApplyForceWithMultipliers | Input guard (checks is_stunned first) |
| 0x4016F0 | Ball_ApplyForceV2 | Alternate input guard (same checks) |
| 0x41B5CF | Scene_UpdateBallsAndState | Detects is_falling → calls respawn |
| 0x403DC0 | Ball_RenderAI | Renders star effect + alpha |
| 0x40C5D0 | DispatchCollisionEvents | E:NODIZZY handler (clears TIME checkpoints, NOT dizzy cure) |
| 0x402400 | Ball_RecordBest | Clears bounce_count(+0x2EC)=0, updates max streak(+0x2F4) |
| 0x405100 | Ball_InitPhysicsDefaults | Resets dizzy_lock(+0x2E9)=0, bounce_count(+0x2EC)=0, all physics defaults |
| 0x4039E0 | Ball_ctor2 | Init: sets dizzy_lock=0, bounce_count=0 |
| 0x419030 | Board_ctor | Initializes per-player data blocks at App+0x5CC, adds to board+0x362C list |
| 0x44DF70 | RaceGoalReached_Render | Draws end screen: reads App+pIdx×0xA0+0x5F8 (dizzy), +0x5F4 (broken) |

## End Screen String Locations

| VA | String |
|----|--------|
| 0x4D6DC0 | "DIZZIED BALLS:" |
| 0x4D6DD0 | "BROKEN BALLS:" |
| 0x4D6DE0 | "WEASEL'S TIME:" |
| 0x4D6DF0 | "BEST RACE TIME:" |
| 0x4D6E00 | "FINAL SCORE: %.0f" |
| 0x4D6E14 | "YOUR NEMESIS: %s" |
| 0x4D6E28 | "RACES FINISHED: %d" |

## Physics Constants

| VA | Value | Role |
|----|-------|------|
| 0x4CF418 | 3.0 (float) | Radius multiplier for bounce speed threshold |
| 0x4CF3F0 | 0.5 (float) | Trajectory velocity scale (halves speed) |
| 0x4CF434 | 1.25 (float) | Y velocity damping factor |
| 0x4CF4E8 | 0.03 (double) | First bounce threshold: speed ≥ 0.03 → bounce_count 0→1 |
| 0x4CF308 | 0.1 (double) | Second bounce threshold: speed ≥ 0.1 → bounce_count 1→2 |
| 0x4CF310 | 1.0 (float) | Speed > 1.0 → sets dizzy_lock (blocks trajectory) |
| 0x4CF368 | ~0.0 (float) | Epsilon for zero-velocity checks |

## Ball_Update Two-Pass Collision Flow

```
Frame N — First qualifying bounce:
  Pass 1 (line 495): bounce_count == 0 → skip trajectory check
  Pass 2 (line 686):
    speed >= 0.03 AND bounce_count == 0 → bounce_count = 1
    bounce_count != 0 AND speed >= 0.1 → bounce_count = 2
    (if speed > 1.0: dizzy_lock = 1, camera change)

Frame N+1 — Next collision:
  Pass 1 (line 495):
    bounce_count > 1 (is 2) AND dizzy_lock == 0
    → Ball_ApplyTrajectory(ball)
      → impact_count = 100 (0.8s full lockout + 3.2s quarter power)
      → speed halved (trajectory scale 0.5)
      → dizzy_lock = 1 (prevents re-trigger)
      → App + pIdx*0xA0 + 0x5F8 += 1  (DIZZY COUNTER INCREMENTED)
      → trail particles + boost sound

Frame N+2:
  Pass 2: has_trajectory (ball+0x14D) set → bounce_count reset to 0
  dizzy_lock stays 1 until Ball_InitPhysicsDefaults (respawn)
```

## E:NODIZZY vs Dizzy System — Not Related

| Feature | E:NODIZZY | Bounce Dizzy |
|---------|-----------|-------------|
| String VA | 0x4CF8B8 | (no string — physics-based) |
| Handler | DispatchCollisionEvents 0x40C64B | Ball_ApplyTrajectory 0x403750 |
| Sets ball+0x2E9? | NO | YES |
| Sets ball+0x2EC? | NO | YES (via Ball_Update) |
| Increments App+0x5F8? | NO | YES |
| Touches TIME entries? | YES (removes them) | NO |
| Calls Ball_RecordBest? | YES | NO |
| End screen counter? | NO | YES (DIZZIED BALLS) |

## Events That Set dizzy_lock (ball+0x2E9 = 1)

| Source | Address | Context |
|--------|---------|---------|
| Ball_ApplyTrajectory | 0x403750+0x87 | After applying trajectory effect |
| Ball_Update speed>1.0 | 0x407391 | When collision speed exceeds 1.0 |
| DispatchCollision E:LIMIT | 0x40C767 | Level boundary hit |
| E:LIMITX | ~0x40F242 | X-axis boundary |
| E:LIMITZ | ~0x40F27D | Z-axis boundary |
| E:LIMITPIPE1 | ~0x40F2B5 | Pipe limit variant 1 |
| E:LIMITPIPE2 | ~0x40F2F7 | Pipe limit variant 2 |
| E:SWALLOW | ~0x40F317 | Fall off edge / pipe swallow |

## Field Summary

| Offset | Name | Type | Default | Description |
|--------|------|------|---------|-------------|
| +0x2E8 | is_falling | byte | 0 | Set by Ball_Shatter etc. Triggers respawn |
| +0x2E9 | dizzy_lock | byte | 0 | Sticky flag: prevents Ball_ApplyTrajectory re-firing. Set by trajectory, E:LIMIT*, E:SWALLOW, speed>1.0. Reset only by Ball_InitPhysicsDefaults |
| +0x2EC | bounce_count | int32 | 0 | Bounce counter for dizzy system. Double-increments 0→1→2 in one frame when speed ≥ 0.03/0.1. Triggers trajectory when > 1 |
| +0x2F0 | impact_count | int32 | 0 | Set to 100 by trajectory. ≥81 blocks input. Decays by 1/frame |
| +0x2F8 | show_stars | byte | 0 | 1 = render 8-star circling effect |
| +0x2F9 | is_stunned | byte | 0 | 1 = blocks ALL force application |
| +0x2FC | alpha | float | 1.0 | 0=transparent, 1=opaque. Fades in on respawn |
| +0x300 | stun_timer | int32 | 0 | 150 on stun. Countdown controls star duration |
| +0x324 | respawn_state | byte | 0 | 1 = ball in respawn sequence |
| +0x808 | no_control_timer | int32 | 0 | Non-zero = input blocked |
| +0xC60 | ground_contact | float | 1.0 | Decays by 0.02/frame when airborne → triggers fall |
| +0x14D | has_trajectory | byte | 0 | Set by Ball_ApplyTrajectory. Triggers bounce_count reset next frame |

## App Per-Player Data Block (at App + 0x5CC + pIdx × 0xA0)

| Offset from block start | App offset (pIdx=0) | Type | Field |
|--------------------------|---------------------|------|-------|
| +0x00 | App+0x5CC | — | Block base (added to board+0x362C AthenaList) |
| +0x0A | App+0x5D6 | byte | finished flag |
| +0x0B | App+0x5D7 | byte | active flag (0=active, checked in Board_ctor) |
| +0x1C | App+0x5E8 | int32 | race_time |
| +0x28 | App+0x5F4 | int32 | broken_balls_count (end screen "BROKEN BALLS:") |
| +0x2C | App+0x5F8 | int32 | dizzied_balls_count (end screen "DIZZIED BALLS:") |
| +0x30 | App+0x5FC | int32 | (cleared to 0 in Board_ctor) |
