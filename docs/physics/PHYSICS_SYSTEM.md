# Hamsterball Physics System — Reverse Engineering Documentation

*Decompiled from Hamsterball.exe (Athena Engine, PE32 i386, image base 0x400000)*
*Primary function: Ball_Update at 0x405E00 (9,442 bytes, 2,541 instructions)*

---

## 1. Architecture Overview

The physics system is a **single monolithic function** (`Ball_Update` at 0x405E00) that runs every frame for every ball. It handles:

- Timer decay and particle effects
- Collision tree building + spatial queries
- Collision response (3 types: floor/wall/ball-ball)
- Force application with multipliers
- Velocity integration and friction
- Camera following
- Facing angle computation
- Spin physics
- Teleport override

The function is called via **vtable[4]** (offset +0x10) from `Scene_UpdateBallsAndState`, wrapped by `Ball_UpdateAndAI` (0x408390) which adds AI targeting after the physics tick.

---

## 2. Ball State Machine

The ball has three operational modes:

| Mode | Flag | Effect |
|------|------|--------|
| **Normal** | `ball+0xC4C = 0` | Full physics, camera follow, boost, friction |
| **Dizzy/Falling** | `ball+0xC4C = 1` | Reduced physics, no camera follow, 0.75× force, no boost |
| **Launch** | `ball+0x2F0 ≥ 100` | Free trajectory for ~1.67s, no external forces |

### Ball_Shrink (0x402200)
Sets the falling/dizzy state:
- `ball+0xC4C = 1` (dizzy flag)
- `ball+0x284 = 13.0f` (shrink radius — was 26.0f)
- `ball+0x188 = 2.5f` (reduce max speed — was 5.0f)
- Plays fall sound from `Scene+0x4D4`

### Ball_Grow (0x402270)
Restores normal state:
- `ball+0xC4C = 0` (clear dizzy)
- `ball+0x284 = 26.0f` (restore radius)
- `ball+0x188 = 5.0f` (restore max speed)

### Ball_ApplyTrajectory (0x403750)
Called when ball hits a boost ramp/launch surface:
- Reads trajectory from `physics_body+0xCA4/CA8/CAC` (launch direction)
- Normalizes trajectory, scales by `_DAT_004CF3F0 = 0.5`
- Damps Y component: `body+0xCA8 *= _DAT_004CF434 = 1.25` (adds vertical boost)
- Sets `impact_counter = 100` at `ball+0x2F0` (prevents force application for ~1.67s)
- Sets `ball+0x14D = 1` (rotation dirty)
- Plays boost sound, creates trail particles

---

## 3. Force Application System

Two force application functions, both with the same multiplier chain:

### Ball_ApplyForceWithMultipliers (0x402650)
Primary force applier — used for player input and direct forces.

### Ball_ApplyForceV2 (0x4016F0)
Secondary force applier — used for collision-derived forces. Adds tube check (complete zeroing).

**Guard conditions** (all must pass for force to apply):
| Condition | Offset | Check |
|-----------|--------|-------|
| Not in tarpit | `ball+0x2F9` | `== 0` |
| Force enabled | `ball+0x2CC` | `== 0` |
| Not frozen | `ball+0x808` | `== 0` |
| Counter within limit | `ball+0x2F0` | `< 81` |

**Force multipliers** (applied in order to the magnitude parameter):

| Condition | Offset | Multiplier | Value | Notes |
|-----------|--------|------------|-------|-------|
| Recent impact | `ball+0x2F0` | `×_DAT_004CF380` | ×0.25 | First few frames after hit |
| In tube | `ball+0x324` | `×_DAT_004CF378` | ×0.0 | Complete freeze in tubes (V2 only) |
| On ice | `ball+0xC5C` | `×_DAT_004CF374` | ×0.2 | Nearly zero on ice; also sets angular velocity `×6.0` |
| Dizzy/falling | `ball+0xC4C` | `×_DAT_004CF36C` | ×0.75 | 25% reduction when falling |

**Velocity accumulation:**
```
ball+0x170 += dir_x × magnitude
ball+0x174 += dir_y × magnitude
ball+0x178 += dir_z × magnitude
```

**Facing direction** (only when direction is non-zero):
- `ball+0x748 = 0` (XZ flat): `angle = atan2(dir_x, dir_z)`
- `ball+0x748 = 1` (tilted): `angle = atan2(-dir_y, dir_z)`
- `ball+0x748 = 2` (XY vertical): `angle = atan2(dir_x, dir_y)`
- Store at `ball+0x198`, set `ball+0x19C = 1`

---

## 4. Collision System

### Collision Types

| Type | Meaning | Response |
|------|---------|----------|
| 1 | Ball-ball | Push apart, score award, clack sound |
| 2 | Wall | Reflect velocity, friction, spin update |
| 5 | Floor | Camera follow, boost counter, slope gravity |

### Type 1: Ball-Ball Collision (at 0x406BD3)

When two balls collide:
1. If `ball+0x2EC` (boost counter) > 1 and not on_ramp: `Ball_ApplyTrajectory` (launch)
2. Compute push direction between ball centers
3. Apply push via `vtable[6]` (SetPosition) on both balls
4. `Sound_Play3D` at collision point
5. **Knockoff scoring**: If one ball is heavier (`ball+0x284`), award `Difficulty_GetTimeModifier(app, 500.0)` points to the heavier ball's player
6. Display "+%d" popup via `AthenaString_Format` at address `0x4CF500`
7. Set string timer to 200 frames (`ball+0x0C = 0xC8`)

### Type 2: Wall Collision (at 0x406B80)

When ball hits a wall surface:
1. Read collision normal from result struct (`piVar16[8..10]`)
2. Reflect velocity across normal: `v_reflected = v - 2(v·n)n`
3. If on ice (`ball+0x324 == 0`): compute surface friction using the collision node's direction vector
4. The **direction vector** from the collision node is used to:
   - Determine if the ball is moving "with" or "against" the surface
   - Apply friction proportional to the surface's physical properties
   - Update the ball's up-direction (`ball+0x6A..0x6C`) for ramp/slope handling

**Key insight**: The collision node's direction vector (`CollisionNode+0x20..0x28` in the spatial tree) stores the surface's tangent/normal information. This is what makes the ball slide along walls instead of stopping dead.

### Type 5: Floor Collision (at 0x407319)

When ball touches floor geometry:
1. Check floor depth (`[edi+0x54]`) against threshold (`_DAT_004CF420 ≈ 0.0`)
2. If NOT dizzy (`ball+0xC4C == 0`):
   - Set `ball+0x2E9 = 1` (on_ramp flag)
   - `Scene_SetCamera(ball, 1)` — camera follows ball
   - `Graphics_SetViewport` — update camera viewport
3. If player (`ball+0x18 != -1`):
   - Boost counter logic: increment `ball+0x2EC` when touching floor
   - OOB detection: compare ball position against viewport bounds
   - If out of bounds: set `ball+0x2E8 = 1` (fell off flag)

**When dizzy (`ball+0xC4C = 1`)**:
- SKIPS `Scene_SetCamera` call — camera doesn't follow falling ball
- SKIPS `on_ramp` flag set — no ramp detection during fall
- SKIPS boost counter increment — can't charge boost while falling
- Jump target: `0x40743D` which goes to player_index check

### How Dizzy Affects Collision Direction

**The collision normal is NOT modified when dizzy.** The collision tree is built and queried identically regardless of the dizzy flag. The direction vector flows through the same reflection/friction code. The dizzy flag only affects:

1. **Camera**: No `Scene_SetCamera` call when dizzy (camera stays at last position)
2. **Boost**: No boost counter accumulation when dizzy
3. **Friction**: Different spin friction calculation (0.25× multiplier at 0x4CF380 when dizzy vs normal friction)
4. **Force**: 0.75× multiplier on all applied forces when dizzy

---

## 5. Gravity and Velocity Integration

### Original Game Flow (Ball_Update at 0x405E00)

The original does NOT use simple Euler integration. The flow is:

1. **Save previous position**: `prev_pos = pos` (ball+0x158..0x160 = ball+0x164..0x16C)
2. **Clear external velocity**: `ball+0x170..0x178 = 0`
3. **Build collision tree**: `SpatialTree_ctor` with gravity scale from `ball+0x278`
4. **Query collision results**: `Scene vtable[1]` with AABB from `pos ± radius`
5. **Process collision results** (types 1, 2, 5)
6. **Compute velocity from position delta**: `velocity = pos - prev_pos`
7. **Apply external forces**: `pos += external_velocity`
8. **Clear external velocity**: `ball+0x170..0x178 = 0` again
9. **Facing angle**: `atan2` from velocity components
10. **Spin physics**: 3 iterations of friction/accumulation
11. **Display position lerp**: `display_pos += (pos - display_pos) × lerp_factor`
12. **Teleport override**: If `ball+0xC3C`, override pos from `ball+0xC40..0xC48`

**Critical difference from simple Euler**: The original computes velocity AFTER collision resolution, not before. This means the velocity stored at `ball+0x170..0x178` is actually the RESULT of collision processing, not an input to it. The collision tree determines where the ball ends up, and velocity is derived from the position change.

### Reimpl Physics Flow (UpdatePhysics in win32_main.c)

The reimpl uses standard semi-implicit Euler:
1. Apply gravity: `vy -= gravity * 500 * dt`
2. Apply input force: `vx/vz += input * force_scale * dt`
3. Apply damping: `vx/vz *= friction`
4. Integrate position: `pos += vel * dt`
5. Resolve collisions: push out + reflect velocity
6. Slope gravity: `vx/vz += gravity * normal × dt`

**Key discrepancy**: The reimpl applies forces BEFORE collision, the original applies them AFTER. This is why the reimpl can "kill momentum" during jumps — the collision response removes the normal component of velocity that was added by input forces, while the original never adds input forces to a velocity that would be immediately removed by collision.

---

## 6. Airborne Physics

### How the Ball Goes Airborne

In the original game, there is NO explicit "jump" mechanic. The ball goes airborne through:

1. **Rolling off an edge**: Ball passes beyond floor geometry → no type-5 results → gravity accumulates
2. **Launch ramp**: `Ball_ApplyTrajectory` sets `impact_counter = 100` and applies trajectory vector
3. **Ball-ball collision**: Knockoff sends ball upward with reflected velocity
4. **Slope/ramp transition**: Ball rolls up a ramp, normal changes, velocity points up

### What Happens While Airborne

When the ball is NOT touching any floor surface:
- **No type-5 collision results** → floor processing doesn't fire
- **Gravity accumulates** through the collision tree's gravity_scale parameter
- **Camera stops following** (Scene_SetCamera not called without type-5)
- **Input forces still apply** at 0.75× if dizzy, full if not
- **Friction changes**: No ground friction, only air friction (0.85× per frame = `_DAT_004CF4C0`)

### The "Momentum Kill" Issue

The problem in the reimpl: when the ball is airborne and moving horizontally, the collision response code treats any contact with geometry as a "wall hit" and reflects the velocity. This can kill horizontal momentum if the ball clips a ledge or edge while airborne.

In the original, this doesn't happen because:
1. Forces are applied AFTER collision resolution
2. The collision tree computes the final position directly
3. The ball's velocity is DERIVED from the position change, not used as an input
4. The collision node's direction vector guides the ball along surfaces, not just reflecting

---

## 7. Key Physics Constants (Verified from .rdata)

| Address | Value | Name | Usage |
|---------|-------|------|-------|
| `0x4CF310` | 1.0 | `UNIT_VALUE` | General purpose 1.0 constant |
| `0x4CF368` | 0.0 | `GROUND_THRESHOLD` | Floor detection threshold |
| `0x4CF36C` | 0.75 | `DIZZY_MULT` | Force multiplier when dizzy/falling |
| `0x4CF374` | 0.2 | `ON_ICE_MULT` | Force multiplier on ice surfaces |
| `0x4CF378` | 0.0 | `IN_TUBE_MULT` | Force multiplier in tube sections |
| `0x4CF380` | 0.25 | `FIRST_FRAME_MULT` | Force multiplier on first frame / after impact |
| `0x4CF3E8` | 6.0 | `ICE_ANGULAR_SCALE` | Angular velocity scale on ice |
| `0x4CF3F0` | 0.5 | `LAUNCH_TRAJECTORY_SCALE` | Launch direction normalization scale |
| `0x4CF418` | 3.0 | `SPEED_ACCUM_WRAP` | Speed gauge wrap value |
| `0x4CF434` | 1.25 | `Y_DAMP` | Vertical velocity damping on launch |
| `0x4CF4C0` | 0.85 | `SPEED_FRICTION` | Per-frame velocity friction |
| `0x4CF520` | 0.025 | `LAUNCH_DIR_SCALE` | Launch direction scaling |
| `0x4CF540` | 0.98 | `SPIN_DECAY_MULT` | Per-frame spin timer decay |

---

## 8. Ball Struct Layout (Key Physics Fields)

| Byte Offset | Decomp Index | Type | Name | Description |
|-------------|-------------|------|------|-------------|
| +0x0C | param_1[3] | int | `string_timer` | Countdown (200=show). Frees display string at 0 |
| +0x10 | param_1[4] | ptr | `app` | App pointer |
| +0x14 | param_1[5] | ptr | `scene` | Scene pointer |
| +0x18 | param_1[6] | int | `player_index` | -1 = AI, 0+ = human |
| +0x158 | param_1[0x56] | Vec3 | `prev_pos` | Previous frame position |
| +0x164 | param_1[0x59] | Vec3 | `pos` | Current position |
| +0x170 | param_1[0x5C] | Vec3 | `vel` | Velocity (cleared each frame, recomputed) |
| +0x188 | param_1[0x62] | float | `max_speed` | 5000.0 (normal) / 2.5 (falling) |
| +0x198 | param_1[0x66] | float | `facing_angle` | Target rotation angle |
| +0x19C | param_1[0x67] | byte | `facing_dirty` | 1 = rotation needs update |
| +0x1A4 | param_1[0x69] | ptr | `physics_body` | Scene physics body (trajectory, friction) |
| +0x284 | param_1[0xA1] | float | `radius` | 26.0 (normal) / 13.0 (falling) |
| +0x2E8 | | byte | `fell_off` | Ball fell off level edge |
| +0x2E9 | | byte | `on_ramp` | Ball is on a ramp/slope surface |
| +0x2F0 | param_1[0xBC] | int | `impact_counter` | ≥100 = in launch, ≥81 = no force |
| +0x2F9 | | byte | `in_tarpit` | Tarpit state (blocks force) |
| +0x313 | | byte | `is_8ball` | 1 = AI 8-ball (dispatches differently) |
| +0x324 | param_1[0xC9] | byte | `in_tube` | In tube section (zeroes force) |
| +0x748 | param_1[0x1D2] | int | `gravity_plane` | 0=XZ flat, 1=tilted, 2=XY vertical |
| +0x808 | | int | `freeze_counter` | >0 = frozen, no force |
| +0xC4C | | byte | `dizzy` | 1 = falling/dizzy state |
| +0xC50 | param_1[0x314] | float | `spin_timer` | Decayed by 0.98× per frame |
| +0xC5C | param_1[0x317] | int | `momentum_transfer` | Flag for ice momentum |
| +0xC3C | param_1[0x30F] | byte | `teleport_active` | 1 = teleport pending |
| +0xC40 | param_1[0x310] | Vec3 | `teleport_dest` | Teleport destination |

---

## 9. Function Call Graph (Ball_Update)

```
Ball_Update (0x405E00)
├── Sound_Play3DAtPosition (0x458EE0) ×2
├── operator_new (0x4BA57B) ×4
│   ├── RumbleScore_ctor (0x44AD50)
│   ├── SpatialTree_ctor (0x463330)
│   └── CollisionNode_ctor (0x466CF0)
├── RNG_Rand (0x45DD60) ×4
├── AthenaList_Append (0x453810/0x453780) ×4
├── Difficulty_GetTimeModifier (0x428ED0) ×5
├── AthenaString_Format (0x466C70) ×2
├── __ftol2 (0x4BA754) ×5
├── Mesh_FindClosestCollision (0x465D90) ×2
├── Scene_SetCamera (0x419FA0) ×1
├── Graphics_SetViewport (0x454B50) ×1
├── Math_Atan2Angle (0x457DE0) ×4
├── Scene_CheckPath (0x457EC0) ×1
├── Wave_Sin (0x457DA0) ×1
├── Wave_Cos (0x457DC0) ×1
├── Collision_GradientEval_Stub (0x458190) ×1
├── Sound_CalculateDistanceAttenuation (0x466750) ×1
├── Vec3List_Free (0x453250) ×2
├── _free (0x4BA576) ×2
├── Gfx_RotateY (0x457BB0) ×1
├── Timer_Init (0x457A40) ×1
├── AthenaList_Init/Clear ×6
└── [vtable indirect calls] ×8
    ├── vtable[1] (QueryCollision)
    ├── vtable[5] (BuildSpatialTree)
    ├── vtable[6] (SetPosition)
    ├── vtable[8] (SpecialAction/Teleport)
    ├── vtable[0x34] (RenderCallback)
    └── vtable[0x74] (SceneCallback)
```

---

## 10. Known Issues in Reimpl Physics

1. **Momentum kill on airborne collision**: Reimpl applies forces before collision, original applies after. This causes horizontal momentum to be lost when the ball clips geometry while airborne.

2. **No collision tree**: Reimpl uses sphere-vs-triangle direct tests. Original uses SpatialTree + CollisionNode hierarchy with direction vectors that guide the ball along surfaces.

3. **Simple velocity reflection**: Reimpl reflects velocity with fixed bounce=0.3. Original uses the collision node's direction vector for smooth sliding along walls.

4. **No dizzy/falling state**: Reimpl has no `ball+0xC4C` equivalent. Ball falling off edges doesn't shrink radius or reduce max speed.

5. **No launch trajectory system**: Reimpl has no `Ball_ApplyTrajectory` equivalent. Boost ramps and launch surfaces are not implemented.

6. **Gravity before collision**: Reimpl applies gravity at the start of the frame, original integrates it through the collision tree's gravity_scale parameter.
