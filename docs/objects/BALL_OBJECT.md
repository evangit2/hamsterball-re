# Hamsterball — The Ball Object: Complete Modding Reference

**Scope:** Original `Hamsterball.exe` (PE32, i386, Athena engine)  
**Last Updated:** 2026-06-04  
**Target Audience:** Modders, DLL injectors, reverse-engineers

---

## Table of Contents

1. [What Is the Ball Object?](#what-is-the-ball-object)
2. [Memory Layout (0xC98 bytes)](#memory-layout)
3. [Vtable Methods](#vtable-methods)
4. [Key Modifiable Fields](#key-modifiable-fields)
5. [Physics Pipeline (Ball_Update)](#physics-pipeline)
6. [Collision & Events](#collision--events)
7. [Rendering](#rendering)
8. [AI & Special Modes](#ai--special-modes)
9. [Modding Hook Points](#modding-hook-points)
10. [Quick Reference](#quick-reference)

---

## What Is the Ball Object?

The Ball is the player-controlled hamsterball. It is the **first parameter** of `Ball_Update` (0x405E00) and the core physics actor in every race and arena mode. The Ball derives from `GameObject` and has a vtable at **0x4CF3A0** with 9 method pointers.

**Constructor chain:**
```
Ball_ctor (0x40AFE0)      → allocate 0xC98 bytes, set base vtable
Ball_ctor2 (0x4039E0)     → init physics defaults, radius, gravity, timers
Scene_SpawnBallsAndObjects → place at START object, set player_index
```

**Destructor:** `Ball_dtor` (0x4027F0) → `Ball_Cleanup` → free strings, timers, render contexts.

---

## Memory Layout

**Total size:** 0xC98 bytes (3,224 bytes)  
**Base class:** `GameObject` (vtable 0x4CF314)  
**Own vtable:** 0x4CF3A0

### Top-Level Layout

| Range | Section | Size |
|-------|---------|------|
| 0x000–0x01C | Identity / vtable / scene links | 0x1C |
| 0x01C–0x107 | UITimer sub-object | 0xEC |
| 0x108–0x1A7 | Timer sub-object | 0xA0 |
| 0x1A8–0x1B7 | Gravity data | 0x10 |
| 0x1B8–0x207 | Render Context #1 | 0x50 |
| 0x208–0x253 | Render Context #2 + tint | 0x4C |
| 0x254–0x263 | Alpha / boost flags | 0x10 |
| 0x264–0x277 | RumbleBoard Timer #1 | 0x14 |
| 0x278–0x28F | Gravity scale / falling / radius | 0x18 |
| 0x290–0x2DB | RumbleBoard Timer #2 + spin | 0x4C |
| 0x2DC–0x31B | Checkpoint / collision state | 0x40 |
| 0x31C–0x767 | (camera / matrix / audio gap) | ~0x44C |
| 0x768–0x7C7 | Camera flags + follow factor | ~0x60 |
| 0x7C8–0x807 | Matrix #1 (4×4) | 0x40 |
| 0x808–0x847 | Matrix #2 (4×4) | 0x40 |
| 0x848–0xC27 | (collision / trail lists / misc) | ~0x3E0 |
| 0xC28–0xC4B | Display string + teleport | 0x24 |
| 0xC4C–0xC87 | Airborne / misc flags | 0x3C |
| 0xC88–0xC97 | World transform matrix (4×4) | 0x40 |

### Critical Fields (Modder-Useful Offsets)

| Offset | Type | Name | Default | Description |
|--------|------|------|---------|-------------|
| 0x000 | void** | vtable | 0x4CF314 | Ball vtable (switches to 0x4CF3A0 after ctor2) |
| 0x014 | void* | scene | — | Pointer to parent Scene object |
| 0x018 | int32 | player_index | -1 | -1 = AI / none, 0 = Player 1, 1 = Player 2 |
| 0x150 | float | accumulated_time | 0.0 | Delta-time accumulator per frame |
| 0x158 | float | prev_pos_x | 0.0 | Previous frame X |
| 0x15C | float | prev_pos_y | 0.0 | Previous frame Y |
| 0x160 | float | prev_pos_z | 0.0 | Previous frame Z |
| **0x164** | **float** | **pos_x** | **0.0** | **Current position X** |
| **0x168** | **float** | **pos_y** | **0.0** | **Current position Y** |
| **0x16C** | **float** | **pos_z** | **0.0** | **Current position Z** |
| **0x170** | **float** | **vel_x** | **0.0** | **Velocity X** |
| **0x174** | **float** | **vel_y** | **0.0** | **Velocity Y** |
| **0x178** | **float** | **vel_z** | **0.0** | **Velocity Z** |
| 0x17C | float | accel_x | 0.0 | Acceleration X (cleared each frame) |
| 0x180 | float | accel_y | 0.0 | Acceleration Y |
| 0x184 | float | accel_z | 0.0 | Acceleration Z |
| **0x188** | **float** | **max_speed** | **5000.0** | **Speed cap (default = 5.0f)** |
| **0x18C** | **float** | **speed_scale** | **1.0** | **Global speed multiplier** |
| 0x1A0 | float | max_speed_cap | 1.0 | Secondary cap multiplier |
| 0x1A4 | void* | collision_mesh | — | CollisionMesh ptr (from CollisionMesh_ctor) |
| 0x1A8 | float[3] | gravity_vec | (0,1,0) | Gravity direction vector |
| 0x1C8 | float | render_alpha | 0.75 | Render context #1 alpha |
| 0x20C | float | color_r | 1.0 | RGBA red |
| 0x210 | float | color_g | 1.0 | RGBA green |
| 0x214 | float | color_b | 1.0 | RGBA blue |
| 0x218 | float | color_a | 1.0 | RGBA alpha |
| 0x254 | uint8 | uses_alpha | 0 | True if color_a != 1.0 |
| 0x260 | uint8 | boost_hit_flag | 0 | Set on boost pad contact |
| 0x264 | uint8[0x14] | rumble_timer1 | — | RumbleBoard timer sub-object |
| 0x278 | float | gravity_scale | 0.1 | Gravity multiplier |
| 0x281 | uint8 | is_falling | 1 | Set to 1 in ctor; 0 when on surface |
| **0x284** | **float** | **radius** | **27.0** | **Ball radius (collision + render)** |
| 0x290 | uint8[0x14] | rumble_timer2 | — | Second RumbleBoard timer |
| 0x2A4 | float | spin_rate | 5.0 | Angular spin factor |
| 0x2BC | float | force_x | 0.0 | Accumulated force X (from input) |
| 0x2C0 | float | force_y | 0.0 | Accumulated force Y |
| 0x2C4 | float | force_z | 0.0 | Accumulated force Z |
| 0x2CC | uint8 | force_disable | 0 | If 1, Ball_ApplyForce is skipped |
| **0x2DC** | **float** | **checkpoint_x** | **0.0** | **Last safe position X** |
| **0x2E0** | **float** | **checkpoint_y** | **0.0** | **Last safe position Y** |
| **0x2E4** | **float** | **checkpoint_z** | **0.0** | **Last safe position Z** |
| 0x2E8 | uint8 | event_flag | 0 | Checkpoint-hit event marker |
| 0x2E9 | uint8 | impact_shatter | 0 | ⚠ **NOT on_ramp/ground flag!** Sticky limit/trajectory flag (E:LIMIT + type-5 collision). Never cleared within Ball_Update. See docs/agent-knowledge/ball-ground-detection.md |
| 0x2F0 | uint32 | force_count | 0 | Number of forces applied this frame |
| 0x2F8 | uint8 | update_in_progress | 0 | Set 1 during Ball_Update |
| 0x2F9 | uint8 | frozen | 0 | Stuck on surface (velocity zeroed) |
| 0x2FC | uint32 | freeze_timer | 0 | Countdown while frozen |
| 0x300 | uint32 | freeze_val | 150 | Constant written when surface found |
| 0x310 | uint8 | state_active | 1 | General active flag |
| 0x324 | uint8 | in_tube | 0 | If true, Ball_Update returns immediately |
| 0x768 | uint8 | cam_active | 1 | Camera follow enabled |
| 0x76A | uint8 | cam_flag | 0 | Cleared on surface snap |
| 0x7C8 | float[16] | matrix_1 | identity | 4×4 transform matrix |
| 0x808 | float[16] | matrix_2 | identity | 4×4 transform matrix |
| 0x700 | int32 | sound_3d_handle | — | 3D sound effect handle |
| **0xC28** | **char*** | **display_string** | **NULL** | **Floating text above ball** |
| 0xC3C | uint8 | teleport_active | 0 | Teleport in progress |
| 0xC40 | float | teleport_x | 0.0 | Teleport destination X |
| 0xC44 | float | teleport_y | 0.0 | Teleport destination Y |
| 0xC48 | float | teleport_z | 0.0 | Teleport destination Z |
| 0xC4C | uint8 | airborne | 0 | Airborne state flag |
| 0xC88 | float[16] | world_matrix | identity | 4×4 world transform for rendering |

> **Modding note:** To teleport the ball, write to `pos_x/y/z` (0x164) AND set `teleport_active=1` with destination coords at 0xC40. The engine will lerp toward the destination over several frames.

---

## Vtable Methods

The Ball vtable lives at **0x4CF3A0** (9 entries). Each entry is a `__thiscall` function taking `Ball*` in ECX.

| Vtable Offset | Address | Name | Description |
|---------------|---------|------|-------------|
| +0x00 | 0x4027F0 | Ball_dtor | Destructor — calls Ball_Cleanup, optionally frees memory |
| +0x04 | 0x405100 | Ball_Update_thunk | Jumps to Ball_Update (0x405E00). **The main physics tick.** |
| +0x08 | 0x402DE0 | Ball_CollisionCheck | Per-frame collision against level mesh + planes |
| +0x0C | 0x402A70 | Ball_OnCollision | Collision response dispatcher |
| +0x10 | 0x408390 | Ball_Render | D3D8 ball rendering — sphere mesh + texture |
| +0x14 | 0x401590 | Ball_vtable5 | Unknown (called from collision path) |
| +0x18 | 0x402650 | Ball_ApplyForce | Apply directional force vector to velocity |
| +0x1C | 0x402C10 | Ball_vtable7 | Unknown (render-related) |
| +0x20 | 0x409480 | Ball_vtable8 | Called on E:BREAK collision events |

**Base GameObject vtable** (0x4CF314) provides shared destructor logic used before Ball-specific cleanup.

---

## Key Modifiable Fields

### Position / Velocity (Direct Write Safe)

Writing to these offsets from a DLL hook is **safe** during `Ball_Update` or `Scene_UpdateBallsAndState`:

```cpp
// Instant teleport (no animation)
*(float*)(ball + 0x164) = newX;
*(float*)(ball + 0x168) = newY;
*(float*)(ball + 0x16C) = newZ;

// Zero velocity (emergency stop)
*(float*)(ball + 0x170) = 0.0f;
*(float*)(ball + 0x174) = 0.0f;
*(float*)(ball + 0x178) = 0.0f;
```

### Physics Constants (Game Data Section)

These global floats at **0x4CF3xx** affect ALL balls. Patch once, affects every frame:

| Address | Value | Effect |
|---------|-------|--------|
| 0x4CF368 | 0.0 | Float epsilon |
| 0x4CF36C | 0.75 | Force multiplier when dizzy |
| 0x4CF374 | 0.2 | Force multiplier on ice |
| 0x4CF378 | 0.0 | Force multiplier in tube (0 = no control!) |
| 0x4CF380 | 0.25 | Force multiplier after first frame of input |
| 0x4CF39C | 0.037 | Collision radius multiplier |
| 0x4CF3E8 | 6.0 | Ice friction factor |
| 0x4CF3F0 | 0.95 | Damping constant |
| 0x4CF484 | 40.0 | Collision mesh distance threshold |
| 0x4CF48C | 2.0 | Y offset threshold (radius + epsilon) |

### Per-Ball Physics Overrides

| Field | Offset | What It Does |
|-------|--------|--------------|
| radius | 0x284 | Collision + visual size. Default 27.0. Shrunk to 13.0 on fall. |
| max_speed | 0x188 | Hard velocity cap. Default 5000.0 (=5.0f). |
| speed_scale | 0x18C | Global multiplier on ALL velocity changes. Default 1.0. |
| gravity_scale | 0x278 | Gravity strength multiplier. Default 0.1. |
| in_tube | 0x324 | If non-zero, Ball_Update returns immediately — no physics! |
| force_disable | 0x2CC | If non-zero, Ball_ApplyForce is skipped entirely. |

> **Mod idea:** Setting `speed_scale = 2.0f` gives a permanent speed boost. Setting `gravity_scale = 0.0f` makes the ball weightless (but collision snapping still applies).

---

## Physics Pipeline

### Ball_Update (0x405E00) — The Main Tick

Called once per frame for every active ball from `Scene_UpdateBallsAndState` (0x41B540).

**Phase 1 — Reset:**
```
accumulated_time = 0
force_count = 0
collision_count = 0
string_timer = 0  (frees display_string at 0xC28 when it hits 0)
event_flag = 0
impact_shatter = 0  # ⚠ NOT on_ramp! Sticky limit/trajectory flag
update_in_progress = 1
```

**Phase 2 — Early-outs:**
- If `in_tube` (0x324) is set: **return immediately** — no physics at all
- If collision mesh flag set: call `Ball_ResetCollisionMesh` (0x4030B0)

**Phase 3 — Surface Finding (Gravity Planes):**

The game has **3 gravity plane modes** set by `Ball_SetTiltedGravity` (0x403100) and `Ball_SetFlatGravity` (0x403150):

| Plane | Normal | Tag Filter | Snap Behavior |
|-------|--------|------------|---------------|
| 0 | (0, -1, 0) | Skip `[X]` | `y = surface.y + radius` |
| 1 | (-1, 0, 0) | Skip `[Z]` | `x = surface.x + radius`, `y = surface.y`, `z = surface.z` |
| 2 | (0, 0, 1) | Skip `[X]` | `z = surface.z - radius` |

**Phase 4 — Surface Snap:**
When the closest surface is found:
1. Ball position snaps to surface + radius offset
2. `vel_x = vel_y = vel_z = 0`
3. `ang_vel_x = ang_vel_y = 0`
4. `frozen = 1`
5. `freeze_timer = 150`

**Phase 5 — No Surface (Falling):**
If no collision surface is found, the ball falls under gravity. Gravity is applied in `Ball_AdvancePositionOrCollision` (0x4564C0), not inside `Ball_Update` itself.

**Phase 6 — Ball-to-Ball (2-Player):**
If `level+0x234` (2-player mode) is active and `player_index != -1`:
- Get the other ball at `level + 0x5DC + (1 - player_index) * 0xA0`
- Skip collision surfaces that are within `radius` of the other ball

### Ball_AdvancePositionOrCollision (0x4564C0) — Core Physics

Called from the scene update pipeline. 6-phase physics:

1. **Free Lists** — Release trail points and collision markers from last frame
2. **Input Velocity** — Add input force to velocity, clamp to `max_speed`
3. **Damping** — `velocity *= (1-dt) + (1-damping)*dt`
4. **Collision** — If collision flag → vtable[0x1C] dispatch
5. **Gravity** — Apply `gravity_vec * gravity_scale` with 0.95 damping
6. **Trail Recording** — Record trail point on non-collision frames

---

## How to Use Ball_ApplyForce (0x402650)

`Ball_ApplyForce` is the **primary way to externally influence ball movement**. It is called from input code (`Ball_GetInputForce` at 0x46EC30), AI code (`Ball_AI_ChaseNearest` at 0x408390), and event handlers (`E:TRAJECTORY`, `E:JUMP`).

### Function Signature

```cpp
void __thiscall Ball_ApplyForce(
    void* ball,      // ECX: Ball* (this)
    float force_x,   // EDX: X component of force direction
    float force_y,   // Stack: Y component
    float force_z,   // Stack: Z component
    float magnitude  // Stack: scalar multiplier
);
```

**What it does:**
1. Normalize the `(force_x, force_y, force_z)` direction vector
2. Multiply by `magnitude` and several conditional multipliers
3. Accumulate the result into **velocity at ball+0x170**
4. Increment `force_count` at **ball+0x2F0**

### Conditional Multipliers (Applied in Order)

| Condition | Field / Global | Multiplier | Effect |
|-----------|---------------|------------|--------|
| `force_count == 0` (first frame) | `DAT_004CF380` | `1.0` | Full force on first press |
| `force_count > 0` (held) | `DAT_004CF380` | `0.25` | Quarter force while held |
| `in_tube` (ball+0x324) | `DAT_004CF378` | `0.0` | **Zero force — completely disabled** |
| `on_ice` flag | `DAT_004CF374` | `0.2` | Ice: 20% force + friction 6.0 |
| `is_dizzy` flag | `DAT_004CF36C` | `0.75` | Dizzy: 75% force |
| `alternate_state` (ball+0xC5C) | `DAT_004CF374` | `0.2` | Alternate physics mode |
| Direction tweak | `DAT_004CF3E8` | `6.0` | Scales the final direction vector |

### Early-Out Checks

`Ball_ApplyForce` **returns immediately** (does nothing) if ANY of these are true:

| Condition | Offset | Value |
|-----------|--------|-------|
| `frozen` | ball+0x2F9 | `1` — ball is snapped to a surface |
| `force_disable` | ball+0x2CC | `1` — force application globally disabled |
| `freeze_counter` | ball+0x808 | `> 0` — input freeze timer active |

### Writing Your Own Force Calls

**Example 1: Push the ball forward instantly**
```cpp
// From a DLL hook — ball is the Ball* pointer
typedef void (__thiscall *tApplyForce)(void* ball, float fx, float fy, float fz, float mag);
tApplyForce ApplyForce = (tApplyForce)0x402650;

// Push forward (+Z) with strength 50.0
ApplyForce(ball, 0.0f, 0.0f, 1.0f, 50.0f);
```

**Example 2: Reverse gravity burst**
```cpp
// Strong upward force — overrides gravity briefly
ApplyForce(ball, 0.0f, -1.0f, 0.0f, 200.0f);
// Note: -Y is "up" because gravity is +Y down
```

**Example 3: Homing force toward a target**
```cpp
float dx = targetX - *(float*)(ball + 0x164);
float dy = targetY - *(float*)(ball + 0x168);
float dz = targetZ - *(float*)(ball + 0x16C);

// Normalize
float len = sqrtf(dx*dx + dy*dy + dz*dz);
if (len > 0.0f) {
    dx /= len; dy /= len; dz /= len;
    ApplyForce(ball, dx, dy, dz, 30.0f);
}
```

### Forcing a Call Through the Vtable

If you want to be binary-compatible with potential mods that hook `Ball_ApplyForce`, call through the vtable instead of the raw address:

```cpp
void** vtable = *(void***)ball;          // vtable at ball+0x00
tApplyForce pfn = (tApplyForce)vtable[6]; // vtable[0x18] = ApplyForce
pfn(ball, fx, fy, fz, magnitude);
```

> **Pitfall:** Writing directly to `ball+0x170` (velocity) bypasses the multipliers, clamping, and surface-snap logic. This is fine for teleport-style hacks, but for gameplay-compatible movement, **always use `Ball_ApplyForce`** so the engine handles ice, tubes, dizzy states, and max_speed correctly.

### Ball_ApplyForceV2 (0x4016F0)

There is an **alternate force application** at 0x4016F0 that is gravity-plane-aware. It applies the same multipliers but respects the current gravity plane when computing facing angle. Use this if you are building a mod that supports tilted-gravity levels (Plane 1 or 2).

---

## Collision & Events

### Two-Tier Collision

**Tier 1 — Planes:** `Ball_CheckCollisionPlanes` (0x402810)
- Tests ball position against 6 collision planes stored at ball+0x0C
- Each plane: `ax + by + cz + d = 0`
- Quick boundary rejection before expensive mesh tests

**Tier 2 — Mesh:** `Mesh_FindClosestCollision` (0x465D90)
- Octree traversal (`Collision_TraverseSpatialTree` at 0x465EF0)
- AABB tests per triangle
- Returns closest hit point with 0.01 precision

### 3-Tier Event Dispatch

When the ball hits a collision object, the event string (at `object+0x864`) is parsed:

```
Scene vtable determines which handler runs:
  ├─ ExpertCollisionEvents (0x40E6A0)   ← Rumble arenas
  │    └─→ DispatchCollisionEvents (0x40C5D0)   ← Shared base (ALL events)
  └─ TowerCollisionEvents (0x40DCD0)   ← Race levels
       └─→ DispatchCollisionEvents (0x40C5D0)   ← Shared base (ALL events)
```
Note: Arena and Level handlers are **parallel**, not chained. Ball_AdvancePositionOrCollision (0x4564C0) handles geometric collision only; event dispatch happens from the ball update chain.

**Event prefixes:**
- `N:` = Named physical object (bumpers, trapdoors, water)
- `E:` = Event trigger (score, jump, limit, action)

**Key events affecting the Ball:**

| Event | Effect on Ball |
|-------|----------------|
| `E:JUMP` | Bounce + 3D sound + +200pts. Sets impact=10, force=0.025. |
| `E:BREAK` | Calls ball vtable[0x20] (0x409480) |
| `E:TRAJECTORY` | Reads `<X>`, `<Y>`, `<Z>` XML tags, calls `Ball_SetTrajectory` |
| `N:WATER` | Sets in-water flag + 10 frame timer |
| `N:TARPIT` | 3D sound, tar state, clears velocity |
| `N:GOAL` | Race finish — plays music, sets finish flags |
| `N:NOCONTROL` | 10 frame freeze (input disabled) |
| `N:MOUSETRAP` | Deflects ball: normalize direction × speed |
| `DROPIN` | Pipe drop-in: sound + +200pts if speed > threshold |
| `PIPEBONK` | Random sound + +100pts, 10 frame cooldown |
| `POPOUT` | Sound + +100pts, 50 frame cooldown |

### Ball State Used by Events

| Ball Offset | Event Using It |
|-------------|----------------|
| 0x164–0x16C (pos) | 3D sound positioning |
| 0x170–0x178 (vel) | Cleared by TARPIT, modified by JUMP |
| 0x2DC–0x2E4 (checkpoint) | `Ball_Shrink` respawns here |
| 0xC2C (section_filter) | `E:SAFESWITCH` copies data here |
| 0xC4C (airborne) | Set by fall code |

---

## Rendering

### Ball_Render (0x402860)

Sets D3D8 states and draws the ball mesh:
```
SetRenderState(D3DRS_ZENABLE, 1)
SetRenderState(D3DRS_CULLMODE, 1)
SetRenderState(D3DRS_SPECULARENABLE, 1)
SetTexture(0, HamsterBall.png)
SetRenderState(D3DRS_LIGHTING, flag from ball+0x700)
DrawPrimitiveUP(triangle_list, vertices)
```

### Ball_RenderShadow (0x401920)

Renders a shadow quad scaled by `radius * constant`, positioned at ball XYZ.

### Ball_RenderWithCollision (0x402C10)

Full render pass: check collision planes → render shadow → apply scaling → end frame.

### Ball_RenderWithMaterial (0x45D8F0)

Used for 3D text (Font_DrawGlyph3D). Renders ball mesh with a custom material override.

### Ball_CreateTrailParticles (0x401DD0)

Spawns **9 trail particles** in a ring around the ball:
- Each particle is a `RumbleScore` object (0x28 bytes)
- Position = ball_pos + (radius × camera-right × sinθ) − (radius × camera-up × cosθ)
- Velocity = offset × random_scale
- Appended to scene particle list at `scene+0x3B00`

---

## AI & Special Modes

### Ball_AI_ChaseNearest (0x408390)

AI steering for computer-controlled balls:
1. Find nearest opponent ball (iterates ball list)
2. Compute direction vector toward target
3. Call `Ball_ApplyForce` toward target
4. If no target found: sine-wave wandering fallback

**Hook tip:** To make AI balls target the player, patch the nearest-ball search to always return the player ball index.

### Ball_FallUpdate (0x408830)

Called when the ball falls off the track:
1. Shrink radius from 27.0 → 13.0
2. Handle scale change smoothly
3. Clean up trail particles
4. After fall animation completes, respawn at last checkpoint (0x2DC)

### Ball_Shrink (0x402200)

Marks ball as fallen:
- `airborne = 1`
- `radius = 13.0`
- Play 3D sound at ball position

### Ball_Grow (0x402270)

Resets from fallen state:
- `airborne = 0`
- `radius = 26.0` (note: different from default 27.0!)
- Restore physics defaults

### Split Ball Mechanics

| Function | Address | What It Does |
|----------|---------|--------------|
| Ball_Split_ctor | 0x408D10 | Constructor for split ball (vtable 0x4CF560) |
| Ball_Shatter | 0x408D70 | Arena 8-ball mechanic: marks parent ball for despawn, spawns 3 AI split balls (called from FollowBall_Update 0x43ECC0) |
| Ball_SplitAndExplode | 0x409480 | Creates 2 split balls + circular RumbleScore explosion (0–360°) |

Split balls are temporary physics objects that scatter from the original ball position and expire after a timer.

### Ball_InitBattleMode (0x456CD0)

Initializes ball for Rodent Rumble arena mode:
- Friction = 0.18
- Bounciness = 1.0
- Radius = 400.0 (much larger than race mode!)
- Speed scale adjusted for arena physics

---

## Modding Hook Points

### High-Value Hooks for Ball Modding

| # | Target | Address | When to Hook | What You Can Do |
|---|--------|---------|--------------|-----------------|
| 1 | Ball_Update | 0x405E00 | Every frame | Override entire physics, teleport, noclip |
| 2 | Ball_ApplyForce | 0x402650 | On input | Custom forces, reverse gravity, zero gravity |
| 3 | Ball_GetInputForce | 0x46EC30 | On input | Add custom actions (brake, jump, camera snap) |
| 4 | Ball_CollisionCheck | 0x402DE0 | Every frame | Disable collision (noclip), custom bounce logic |
| 5 | Ball_Render | 0x402860 | Every frame | Custom visual effects, wireframe, size changes |
| 6 | Ball_Shrink | 0x402200 | On OOB | Prevent falling, teleport instead |
| 7 | Scene_UpdateBallsAndState | 0x41B540 | Every frame | Modify ball list iteration, add/remove balls |
| 8 | Ball_AI_ChaseNearest | 0x408390 | AI tick | Change AI behavior, make AI friendly/hostile |

### Example: Speed Hack (Single Write)

```cpp
// Multiply max_speed for all balls by 2x
*(float*)(ball + 0x188) = 10000.0f;  // 10.0f max speed
```

### Example: Noclip (Hook Ball_CollisionCheck)

```cpp
typedef void (__thiscall *tCollisionCheck)(void* ball);
tCollisionCheck oCollisionCheck;

void __fastcall hkCollisionCheck(void* ball) {
    // Do nothing — skip ALL collision
    // Ball will fly through walls and floors
}
```

### Example: Custom Action in Ball_GetInputForce

```cpp
// After calling original, check your custom key
oBallGetInputForce(ball, outForce);

if (g_CustomBrakeKey.isDown) {
    float* vel = (float*)((char*)ball + 0x170);
    vel[0] *= 0.85f;
    vel[1] *= 0.85f;
    vel[2] *= 0.85f;
}
```

### Example: Gravity Modifier

```cpp
// Change gravity direction (default is +Y down)
float* gravity = (float*)((char*)ball + 0x1A8);
gravity[0] = 0.0f;   // X
gravity[1] = -1.0f;  // Y (upward gravity = anti-gravity!)
gravity[2] = 0.0f;   // Z
```

---

## Quick Reference

### All Ball Functions by Address

| Address | Name | Lines | Description |
|---------|------|-------|-------------|
| 0x401590 | Ball_vtable5 | — | Unknown vtable slot |
| 0x401920 | Ball_RenderShadow | — | Shadow quad renderer |
| 0x401DD0 | Ball_CreateTrailParticles | — | 9-particle trail ring |
| 0x402200 | Ball_Shrink | — | Mark fallen, shrink radius |
| 0x402270 | Ball_Grow | — | Reset from fallen state |
| 0x402650 | Ball_ApplyForce | 47 | Apply force vector to velocity |
| 0x4027F0 | Ball_dtor | — | Destructor |
| 0x402810 | Ball_CheckCollisionPlanes | — | 6-plane collision test |
| 0x402860 | Ball_Render | — | D3D8 ball mesh render |
| 0x402A70 | Ball_OnCollision | — | Collision response |
| 0x402C10 | Ball_vtable7 | — | Render-related vtable slot |
| 0x402DE0 | Ball_CollisionCheck | — | Per-frame collision entry |
| 0x4030B0 | Ball_ResetCollisionMesh | — | Reset collision state |
| 0x403100 | Ball_SetTiltedGravity | — | Set gravity plane = 1 |
| 0x403150 | Ball_SetFlatGravity | — | Set gravity plane = 2 |
| 0x4039E0 | Ball_ctor2 | — | Secondary constructor (physics init) |
| 0x405100 | Ball_Update_thunk | — | Vtable trampoline to 0x405E00 |
| **0x405E00** | **Ball_Update** | **400+** | **Main physics tick** |
| 0x408390 | Ball_AI_ChaseNearest | 60 | AI opponent steering |
| 0x408830 | Ball_FallUpdate | 40 | Fall animation + respawn |
| 0x408D10 | Ball_Split_ctor | 14 | Split ball constructor |
| 0x408D70 | Ball_Shatter | 50 | Arena: split parent ball into 3 AI balls |
| 0x409480 | Ball_SplitAndExplode | 70 | Split + RumbleScore ring |
| 0x40AFE0 | Ball_ctor | 30 | Primary allocator + base init |
| 0x40AF90 | Ball_GetTransform | — | Read transform into struct |
| 0x4564C0 | Ball_AdvancePositionOrCollision | — | 6-phase physics pipeline |
| 0x456CD0 | Ball_InitBattleMode | — | Arena physics defaults |
| 0x46EC30 | Ball_GetInputForce | 7 | Convert input to force vector |

### Ball Struct C Header

A machine-readable C header is maintained at:
```
analysis/ghidra/structs/ball_struct.h
```

It contains the full 0xC98 layout with verified offsets from `Ball_ctor2` decompilation.

---

## See Also

- `docs/INPUT_SYSTEM.md` — Input device, key remapping, DIK codes
- `docs/COLLISION_EVENT_SYSTEM.md` — Full event dispatch chain
- `docs/COLLISION_SYSTEM_DEEP.md` — Octree, AABB, mesh collision
- `docs/MODDING_NEW_CONTROLS.md` — Step-by-step DLL hook guide
- `docs/SCENE_SYSTEM_DECOMP.md` — Scene update order, camera modes
- `docs/BALL_PHYSICS_DECOMP.md` — Legacy physics documentation
- `analysis/ghidra/structs/ball_struct.h` — C struct definition

---

*Document compiled from 3,700+ documented functions, live Ghidra decompilation, and cross-referenced struct analysis. All offsets verified against PE32 Hamsterball.exe loaded at 0x00400000.*
