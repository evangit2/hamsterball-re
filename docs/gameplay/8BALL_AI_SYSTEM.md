# Hamsterball 8-Ball AI System

## Overview

The "8-ball" or "BADBALL" is an AI-controlled enemy ball that appears in single-player race levels. It is a fully autonomous Ball object with a simple chase-and-spin behavior designed to interfere with the player. The AI is **stateless** — no pathfinding, no planning, just a position-seeking force vector updated every frame.

**Key insight:** The 8-ball is NOT a separate object type. It is a standard `Ball` instance with NPC flags set. The AI code lives **inside the same `Ball_Update` function** (vtable `[0x10]` at `0x408390` wrapping `0x405E00`) that runs on **every ball every frame**. For player balls, the AI block is skipped because `is_8ball = 0`. There is no separate 8-ball update function — all balls share the same vtable and the same tick.

**Critical consequence:** If you set `ball[0x31D] = 1` on a player ball, that player ball will execute the 8-ball AI and chase other balls automatically. The AI is a conditional block inside the universal update, not a separate object system.

---

## BADBALL MESHWORLD Format

In MESHWORLD level data, BADBALL objects are defined with typed parameters:

```
BADBALL pos_x pos_y pos_z
  CHASE distance
  HOME distance
  SIZE radius
  SPINDISTANCE radius
```

### Parameters

| Tag | Type | Default | Description |
|-----|------|---------|-------------|
| `CHASE` | float | 0 | Max distance to target — AI shuts down if player is farther than this |
| `HOME` | float | 0 | Max distance from spawn — AI shuts down if 8-ball is farther than this from its HOME position |
| `SIZE` | float | 27 | Ball radius (overrides default) |
| `SPINDISTANCE` | float | 0 | Radius of the circular "spin" wobble added to target position |

### Example from Level Data

```
BADBALL 120.0 50.0 200.0
  CHASE 800
  HOME 400
  SPINDISTANCE 50
```

This 8-ball:
- Spawns at (120, 50, 200)
- Only chases when player is within 800 units
- Only chases when itself is within 400 units of spawn
- Adds a 50-unit circular wobble to its target position

---

## AI Activation Conditions

The 8-ball AI runs inside `Ball_Update` (vtable slot `[0x10]` at `0x408390`) ONLY when either condition is true:

```cpp
if (ball[0x31d] != 0 || scene[0x237] != 0) {
    // Run 8-ball AI
}
```

| Condition | Meaning | When Set |
|-----------|---------|----------|
| `ball[0x31d] != 0` | Ball was spawned as a BADBALL | Set by `CreateBadBall()` when parsing MESHWORLD |
| `scene[0x237] != 0` | Battle mode / multiplayer | Set by `Ball_InitBattleMode()` for CPU players |

**Key:** `0x31d` is set to `1` during `CreateBadBall()` and cleared during level transitions. The AI block is skipped entirely for normal player balls.

---

## AI Behavior: Two Modes

### Mode 1: Direct Chase (when no BADBALL config)

If no `CHASE`/`HOME` parameters were set, the AI simply seeks the nearest valid ball:

```cpp
// Find nearest valid target ball
float best_dist = 999999.0;
Ball* target = nullptr;
for (each ball in scene->ball_list) {
    if (ball->race_active &&               // [0x768] — ball is in active race
        !ball->is_stunned &&               // [0x2f9] — not currently falling/respawning
        ball->player_index != -1 &&        // [0x18] — has a valid player
        !ball->is_teleporting &&           // [0x324] — not in teleport
        scene->some_mode_flag) {           // [0x3a4c]
        
        float dist = Math_FastDistance2D(8ball->pos.x, 8ball->pos.z,
                                          ball->pos.x, ball->pos.z);
        if (dist < best_dist) {
            best_dist = dist;
            target = ball;
        }
    }
}
```

**Target validity check breakdown:**
- `race_active (0x768)`: Ball is actively racing (not in menu, not finished)
- `!is_stunned (0x2f9)`: Ball is not in a fall/respawn state
- `player_index != -1 (0x18)`: Ball has a player/controller assigned
- `!is_teleporting (0x324)`: Ball is not currently teleporting
- `scene[0x3a4c]`: Some scene-level mode flag (possibly "race in progress")

### Mode 2: BADBALL Configured Chase

When `CHASE` and `HOME` are set, additional distance checks gate the behavior:

```cpp
// Only chase if:
// 1. Random roll succeeds (RNG < threshold)
// 2. Player is within CHASE distance
// 3. 8-ball is within HOME distance from its spawn

float rng = RandomFloat();
if (rng < threshold &&
    dist_to_player < ball[0x31c] &&    // CHASE parameter
    dist_from_home < ball[0x31b]) {   // HOME parameter
    
    target = player_ball;
}
```

---

## Target Computation: Spin Wobble

The 8-ball's target position is NOT the player's exact position. It adds a circular wobble using `SPINDISTANCE`:

```cpp
// Base target = spawn position (HOME position, stored at [0x318/0x319/0x31A])
Vec3 target = {
    ball[0x318],  // spawn X
    ball[0x319],  // spawn Y
    ball[0x31A]   // spawn Z
};

// Add spin wobble
float spin_angle = ball[0x31e] * spin_speed;  // [0x31e] increments each frame
target.x += sin(spin_angle) * ball[799];      // ball[799] = SPINDISTANCE
target.z += cos(spin_angle) * ball[799];

// Increment spin counter
ball[0x31e] += 0.05;  // _DAT_004cf48c ≈ 0.05
```

**Key insight:** The spin makes the 8-ball orbit around its HOME point while chasing, creating a "probing" behavior rather than a direct homing missile.

---

## Force Application

Once the target is computed, the AI applies force toward it:

```cpp
// Compute direction to target
float dx = target.x - ball->pos.x;   // [0x59]
float dy = target.y - ball->pos.z;   // [0x5B] — note: using Z as Y in this code
float dz = target.z - ball->pos.y;   // [0x5A]

// Normalize
float dist_sq = dx*dx + dy*dy + dz*dz;
float dist = sqrt(dist_sq);
if (dist > 1.0) {
    dx /= dist;
    dy /= dist;
    dz /= dist;
}

// If 8-ball is smaller than target ball, REVERSE direction (flee!)
if (8ball->radius < target->radius * 0.9) {
    dx = -dx;
    dy = -dy;
    dz = -dz;
}

// Apply force via vtable call
// vtable[0x14] = Ball_ApplyForceWithMultipliers(this, dirX, dirY, dirZ, multiplier)
ball->vtable[0x14](ball, dx, 0.0, dz);
```

**Flee behavior:** If the 8-ball's radius is smaller than the target's radius × 0.9, it **runs away** instead of chasing. This can happen with power-up balls or when the player has grown.

---

## Ball Struct: AI-Relevant Fields

### 8-Ball Config (set by `CreateBadBall()`)

| Byte Offset | Int Index | Type | Name | Description |
|------------|-----------|------|------|-------------|
| +0xC60 | [0x318] | Vec3 | home_position | Spawn position (X/Y/Z) |
| +0xC6C | [0x31B] | float | home_distance | `HOME` parameter — max distance from spawn |
| +0xC70 | [0x31C] | float | chase_distance | `CHASE` parameter — max distance to target |
| +0xC74 | [0x31D] | bool | is_8ball | Set to 1 by `CreateBadBall()`, cleared on level transition |
| +0xC78 | [0x31E] | float | spin_counter | Increments each frame, drives spin wobble angle |
| +0xC7C | [0x31F] | float | spin_distance | `SPINDISTANCE` parameter — radius of spin wobble |

### Physics/Collision (shared with all balls)

| Byte Offset | Int Index | Type | Name | Description |
|------------|-----------|------|------|-------------|
| +0x164 | [0x59] | float | pos.x | Current X position |
| +0x168 | [0x5A] | float | pos.y | Current Y position |
| +0x16C | [0x5B] | float | pos.z | Current Z position |
| +0x284 | [0xA1] | float | radius | Ball collision radius |
| +0x2F0 | [0xBC] | int | impact_counter | Frames since last impact (damps force) |
| +0x2F9 | [0x7E] | bool | is_stunned | Currently in fall/respawn |
| +0x324 | [0xC9] | bool | is_teleporting | Currently teleporting |
| +0x768 | [0x1DA] | bool | race_active | Ball is in active race |
| +0xC28 | [0x30A] | char* | display_name | Ball name string (for score display) |
| +0xC5C | [0x317] | int | battle_mode_flags | Friction/speed modifiers for battle |

---

## Scene Struct: AI-Relevant Fields

| Byte Offset | Int Index | Type | Name | Description |
|------------|-----------|------|------|-------------|
| +0x237 | [0x8D] | bool | battle_mode | Set when in battle/multiplayer mode |
| +0x3A4C | [0xE93] | bool | race_in_progress | Some mode flag checked for target validity |
| +0x29D4 | [0xA75] | AthenaList | ball_list | List of all balls in the scene |
| +0x29D8 | [0xA76] | int | ball_count | Number of balls |
| +0x2DE0 | [0xB78] | void** | ball_array | Pointer array to ball objects |

---

## Key Functions

| Address | Name | Role |
|---------|------|------|
| `0x40BCA0` | `CreateBadBall()` | Scans MESHWORLD for `BADBALL` objects, constructs Ball with NPC flags |
| `0x408390` | `Ball_AI_Update()` | Vtable slot `[0x10]` — contains the chase AI logic |
| `0x405E00` | `Ball_Update()` | Main physics tick, calls AI update if conditions met |
| `0x456CD0` | `Ball_InitBattleMode()` | Sets battle parameters (speed, friction, battle flags) |
| `0x401660` | `Ball_SetName()` | Sets display name (used for score announcements) |
| `0x402650` | `Ball_ApplyForceWithMultipliers()` | Vtable `[0x14]` — applies directional force |
| `0x458130` | `Math_FastDistance2D()` | Fast approximated 2D distance (no sqrt, used for target selection) |
| `0x457DA0` | `Wave_Sin()` | Sine lookup for spin wobble |
| `0x457DC0` | `Wave_Cos()` | Cosine lookup for spin wobble |

---

## RNG and "Randomness"

The AI uses the game's global RNG (`FUN_004BA754`) for two purposes:

1. **Target selection jitter:** Random float < 0.5 gates whether the AI runs at all this frame
2. **Score event RNG:** When an 8-ball hits a player, RNG determines score bonus and plays a random taunt sound

```cpp
// When 8-ball hits a player ball:
float rng = RandomFloat();
if (rng < 0.5) {
    AddScore(2000);  // Bonus points
    PlayRandomTauntSound();  // "Gotcha!" etc.
    target_ball->score += rng * 100;  // Small random bonus
}
```

---

## Decompilation Sources

All analysis is derived from raw Ghidra decompiled C:

- **`analysis/ghidra/decompilations/ball/decomp_ball_vtable_0x408390.c`** — Main AI update logic
- **`analysis/ghidra/decompilations/scene/decomp_createbadball.c`** — `CreateBadBall()` function
- **`analysis/ghidra/decompilations/ball/decomp_ball_vtable_update_physics.c`** — Ball struct field map
- **`analysis/ghidra/decompilations/ball/decomp_ball_initbattlemode.c`** — Battle mode initialization

---

## Modding Notes

### To disable 8-balls in a level:

Option 1 — Remove from MESHWORLD: Delete the `BADBALL` object entries from the level file.

Option 2 — Patch at runtime: Set `ball[0x31D] = 0` for all 8-ball instances. The AI block is skipped.

Option 3 — Patch the scene: Set `scene[0x237] = 0` to disable battle mode AI entirely.

### To make 8-balls more aggressive:

- Increase `CHASE` distance (default is often 0 = unlimited)
- Decrease `HOME` distance (makes them chase from farther spawn radius)
- Increase `SPINDISTANCE` (makes them wobble more wildly)
- Patch `0x408390` to remove the RNG gate (always chase instead of 50% chance)

### To make 8-balls flee instead of chase:

Set their radius very small: `ball[0xA1] = 5.0f` (player radius is ~27). The flee branch triggers when `8ball_radius < target_radius * 0.9`.

---

## Verified Confidence Markers

| Claim | Marker | Evidence |
|-------|--------|----------|
| `CreateBadBall` scans for `BADBALL` string | ✅ | Raw C string compare in `decomp_createbadball.c` |
| `CHASE`/`HOME`/`SIZE`/`SPINDISTANCE` parsed | ✅ | Raw C `__stricmp` + `atol` chain in `decomp_createbadball.c` |
| `ball[0x31D]` gates AI execution | ✅ | Raw C `if (param_1[0x31d] != 0)` in `decomp_ball_vtable_0x408390.c` |
| `ball[0x318..0x31A]` = HOME position | ✅ | Copied from MESHWORLD spawn pos in `CreateBadBall` |
| `ball[0x31E]` = spin counter | ✅ | Incremented each frame, passed to `Wave_Sin`/`Wave_Cos` |
| Spin wobble uses `SPINDISTANCE` | ✅ | `ball[799]` (0xC7C) × sin/cos added to target |
| Flee when smaller radius | ✅ | `if (8ball_radius < target_radius * 0.9) { negate direction }` |
| Target validity checks | ✅ | 5-condition compound `if` in raw C |
| Force applied via vtable[0x14] | ✅ | `(**(code **)(*param_1 + 0x14))(...)` call |
| `Math_FastDistance2D` is approximate | ✅ | Raw C: `max*0.96 + min*0.4` approximation |
