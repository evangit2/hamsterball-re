# BadBall (8-Ball) AI Circling System

## Function: Ball_AI_ChaseNearest (vtable[4], 0x408390)

Created via GhidraMCP `create_function` (was not in function table). Body size: 1184 bytes.

### Activation Gate

```c
if ((param_1[0x31d] != 0) || (*(char *)(param_1[4] + 0x237) != '\0')) {
    // Run AI
}
```

Two paths to AI activation:
1. `ball+0xC74` (C-index 0x31D) = `is_8ball` flag ≠ 0 — set by Ball_InitBattleMode
2. `App+0x237` ≠ 0 — scene is in tournament/demo mode (App is at ball+0x10 via param_1[4])

### AI Pipeline (in order)

1. **Save previous frame's is_active state**, call `Ball_Update(param_1)` (0x405E00 — full 23-phase physics tick)
2. **Score proximity check** — if a ball just became inactive (transition from active→inactive), scan all balls within `_DAT_004cf554` (3000.0 units) and award score via Difficulty_GetTimeModifier × 2000.0
3. **Compute distance from home** — `fVar1` = 2D distance from ball pos to home position (param_1[0x318..0x31A])
4. **Find nearest player ball** — iterate board+0x29D4 AthenaList, filter by:
   - `ball+0x768` ≠ 0 (is active/alive)
   - `ball+0x2F9` == 0 (NOT falling)
   - `ball+0x300` == 0 (not in some special state)
   - `ball+0x324` == 0 (not flagged)
   - `scene+0x3A4C` ≠ 0 (countdown finished / race active)
   - `ball+0x18` ≠ -1 (has a player index)
   
   Track nearest by `Math_FastDistance2D`.

5. **Orbit computation** (if distance from home < 220.0):
   ```c
   // _DAT_004cf550 = 220.0 (orbit threshold)
   if (dist_from_home < 220.0) {
       target_x = sin(spin_angle) × SPINDISTANCE + home_x;
       target_z = cos(spin_angle) × SPINDISTANCE + home_z;
   }
   ```
   
   - `spin_angle` = `param_1[0x31e]` = ball+0xC78 (float, radians)
   - `SPINDISTANCE` = `param_1[799]` = ball+0xC7C (float, orbit radius)
   - Uses `Wave_Sin` and `Wave_Cos` (not standard C math)

6. **Angle increment** (always, every frame):
   ```c
   param_1[0x31e] += _DAT_004cf48c;  // += 2.0 radians per frame
   ```
   
   _DAT_004cf48c = 2.0 (0x40000000). This is CONSTANT — the angular speed of the orbit is always the same.

7. **Chase/flee override** (if player ball found within CHASE+HOME):
   ```c
   if (nearest_ball != 0 && dist_from_home < HOME && nearest_dist < CHASE) {
       target_x = nearest_ball->pos_x;  // +0x164
       target_z = nearest_ball->pos_z;  // +0x16C
       
       // FLEE if player is bigger
       if (ball+0x284 (own_radius) < nearest_ball+0x284 × _DAT_004cf508) {
           target_x = -target_x;
           target_z = -target_z;
       }
   }
   ```
   
   _DAT_004cf508 = 0.6666667 (2/3) — flee if own radius < player radius × 0.667

8. **Apply force toward target** — normalize direction vector, call vtable[5] (0x14 offset):
   ```c
   dx = target_x - ball_pos_x;
   dz = target_z - ball_pos_z;
   dist = sqrt(dx² + dz²);
   if (dist > 0) { inv = 1.0 / dist; dx *= inv; dz *= inv; }
   (*vtable[5])(dx, 0, dz);  // Ball_ApplyForceWithMultipliers
   ```

### Key Constants (verified from .data section)

| Address | Hex | Float | Purpose |
|---------|-----|-------|---------|
| 0x4CF550 | 00005C43 | 220.0 | Orbit threshold: if dist_from_home < this, do orbit |
| 0x4CF48C | 00000040 | 2.0 | Spin angle increment per frame (radians) |
| 0x4CF508 | 66666666 | 0.667 | Flee ratio: flee if own_radius < player_radius × 0.667 |
| 0x4CF554 | 00803B45 | 3000.0 | Score award distance (when ball deactivates near player) |
| 0x4CF558 | 0000804F | 250000.0(?) | FastDistance2D correction constant |
| 0x4CF368 | 00000000 | 0.0 | Zero constant (used as floor/clamp) |
| 0x4CF310 | 0000803F | 1.0 | One constant (used for normalization) |

### MESHWORLD Tag Values Per Level

| Level File | Race | CHASE | HOME | SPINDISTANCE | Notes |
|------------|------|-------|------|--------------|-------|
| LevelCascade | Beginner | 300 | 375 | **NOT SPECIFIED** (default 50.0) | No SPINDISTANCE tag — uses InitPhysics default |
| Level2 | Intermediate | — | — | — | (BADBALL not present in this level) |
| Level3 | Dizzy | 200 | 300 | 45 | Explicit orbit radius |

### SPINDISTANCE Initialization — VERIFIED (June 2026)

**VERIFIED by decompilation of Ball_InitPhysicsDefaults (0x00405100):**

```c
void __fastcall Ball_InitPhysicsDefaults(void *param_1) {
    Ball_SetupCollisionRender((int)param_1);
    Vec3_Init(&stack, 0, 0, 0);
    Ball_SetTrajectory(param_1, ...);
    *(int*)(param_1 + 0x18) = -1;           // player_index = none
    *(float*)(param_1 + 0x278) = 0.5;       // gravity_scale
    *(float*)(param_1 + 0x27c) = 0.2;       // unknown
    *(float*)(param_1 + 0x284) = 35.0;      // radius
    *(float*)(param_1 + 0x1a0) = 0.2;       // speed_scale
    *(float*)(param_1 + 0x188) = 6.0;       // max_speed
    *(float*)(param_1 + 0xC78) = 0.0;       // spin_angle = 0 (radians)
    *(float*)(param_1 + 0xC7C) = 50.0;      // SPINDISTANCE = 50.0 (orbit radius)
    *(float*)(param_1 + 0xC6C) = 600.0;     // CHASE default = 600.0
    *(float*)(param_1 + 0xC70) = 1200.0;    // HOME default = 1200.0
}
```

**SPINDISTANCE IS initialized to 50.0** — it is NOT uninitialized heap garbage. The orbit radius on Beginner (where no SPINDISTANCE MESHWORLD tag is present) is deterministically 50.0 units.

This means behavioral variance on Beginner comes from:

1. **Timing of player entry** — spin_angle increments 2.0 rad/frame. At 25fps that's ~50 rad/s ≈ 8 full revolutions/s. The orbit position when the player enters determines initial approach direction.
2. **Orbit vs chase transition** — entering CHASE range (300 on Beginner) overrides orbit with direct chase. The transition point relative to the orbit cycle makes it look more or less aggressive.
3. **Player radius vs 8-ball radius** — if player is bigger, 8-Ball flees instead of chases (× 0.667 ratio check).
4. **Ball_Update phase interactions** — Ball_AI_ChaseNearest calls Ball_Update every frame; physics state (velocity, position on slopes) at the moment of chase transition affects how aggressively the ball curves.

### Ball Field Layout (AI-relevant offsets)

| Byte Offset | C-Index | Type | Field | Set By |
|-------------|---------|------|-------|--------|
| 0xC60 | 0x318 | float | home_x | CreateBadBall (from MW coords) |
| 0xC64 | 0x319 | float | home_y | CreateBadBall |
| 0xC68 | 0x31A | float | home_z | CreateBadBall |
| 0xC6C | 0x31B | float | CHASE distance | CreateBadBall (from MW tag) |
| 0xC70 | 0x31C | float | HOME distance | CreateBadBall (from MW tag) |
| 0xC74 | 0x31D | byte | is_8ball flag | Ball_InitBattleMode |
| 0xC78 | 0x31E | float | spin_angle (radians) | Ball_ctor2 (byte 0 only? or InitPhysics?) |
| 0xC7C | 0x31F (799) | float | SPINDISTANCE (orbit radius) | CreateBadBall (from MW tag, or InitPhysics default?) |
| 0x164 | 0x59 | float | pos_x | Ball_Update (physics) |
| 0x168 | 0x5A | float | pos_y | Ball_Update |
| 0x16C | 0x5B | float | pos_z | Ball_Update |
| 0x284 | 0xA1 | float | radius | Ball_InitPhysicsDefaults (35.0) or SIZE tag |

### GhidraMCP Notes

- `Ball_AI_ChaseNearest` at 0x408390 was NOT in Ghidra's function table — had to `create_function` first, then `decompile_function` worked.
- The decompiled output uses `param_1[N]` (int* indexing, ×4 for byte offset) and `param_1[799]` (decimal index, ×4 = 0xC7C).
- `__ftol2()` calls in the decompilation are FPU register spills from float→int conversions used by `Math_FastDistance2D` — not actual function calls.
- `unaff_EBX`, `unaff_EBP`, `unaff_ESI`, `unaff_EDI` are register variables that Ghidra couldn't fully recover — they carry ball position coordinates through the distance calculations.

### See Also
- `references/object-spawning-createbadball.md` — CreateBadBall spawning analysis, Ball_InitPhysicsDefaults defaults
- `references/ball-vtable-player-vs-ai-path.md` (in hamsterball-dll-modding skill) — vtable[4] dispatch for AI vs player balls
