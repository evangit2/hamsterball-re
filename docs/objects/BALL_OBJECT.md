# Hamsterball — The Ball Object: Complete Modding Reference

**Scope:** Original `Hamsterball.exe` (PE32, i386, Athena engine)  
**Last Updated:** 2026-06-25  
**Target Audience:** Modders, DLL injectors, reverse-engineers

---

## Table of Contents

1. [What Is the Ball Object?](#what-is-the-ball-object)
2. [Memory Layout (0xC98 bytes)](#memory-layout)
3. [PhysicsObject / CollisionMesh (at +0x1A4, size 0xCB0)](#physics-object)
4. [Vtable Methods](#vtable-methods)
5. [Key Modifiable Fields](#key-modifiable-fields)
6. [Physics Pipeline (Ball_Update)](#physics-pipeline)
7. [Collision & Events](#collision--events)
8. [Rendering](#rendering)
9. [AI & Special Modes](#ai--special-modes)
10. [Modding Hook Points](#modding-hook-points)
11. [Quick Reference](#quick-reference)

---

## What Is the Ball Object?

The Ball is the player-controlled hamsterball. It is the **first parameter** of `Ball_Update` (0x405E00) and the core physics actor in every race and arena mode. The Ball derives from `GameObject` and has a vtable at **0x4CF3A0** with **65+ entries** (not 9 — see [Vtable Methods](#vtable-methods)).

**Constructor chain:**
```
Ball_ctor (0x40AFE0)      → allocate 0xC98 bytes, set base vtable
Ball_ctor2 (0x4039E0)     → init physics defaults, radius, gravity, timers
Scene_SpawnBallsAndObjects → place at START object, set player_index
```

**Constructor field overrides:** `Ball_ctor2` sets initial defaults. Then `Ball_InitPhysicsDefaults` (vtable[1], 0x405100) is called during setup and **overrides many fields** with different values. The runtime values differ from the raw ctor2 defaults:

| Field | ctor2 value | InitPhysicsDefaults value |
|-------|-------------|---------------------------|
| +0x188 max_speed | 5000.0 | **6.0** |
| +0x1A0 speed_scale | 1.0 | **0.2** |
| +0x278 gravity_scale | 0.1 | **0.5** |
| +0x27C (unknown) | 0.0 | **0.2** |
| +0x284 radius | 27.0 | **35.0** |

Always reference the InitPhysicsDefaults column for actual in-game defaults.

**Destructor:** `Ball_dtor` (0x4027F0) → `Ball_dtor2` → `GameObject_dtor` → optionally `free(this)`.

---

## Memory Layout

**Total size:** 0xC98 bytes (3,224 bytes)  
**Base class:** `GameObject` (vtable 0x4CF314)  
**Own vtable:** 0x4CF3A0

### Critical Fields (Modder-Useful Offsets)

| Offset | Type | Name | Default (runtime) | Description |
|--------|------|------|---------|-------------|
| 0x000 | void** | vtable | 0x4CF3A0 | Ball vtable (65+ entries) |
| 0x00C | int32 | string_timer | 0 | Countdown to free display_string |
| 0x010 | void* | app_state | — | App pointer (from param_1+0x878) |
| 0x014 | void* | scene | — | Pointer to parent Scene/Board object |
| 0x018 | int32 | player_index | -1 | -1 = AI / none, 0 = Player 1, 1 = Player 2, etc. |
| 0x014 | void* | render_callback | — | UITimer sub-object start |
| 0x150 | float | accumulated_time | 0.0 | Delta-time accumulator per frame |
| 0x154 | int32 | rng_seed | — | Random seed (set by RNG_Rand) |
| 0x158 | float | prev_pos_x | 0.0 | Previous frame X |
| 0x15C | float | prev_pos_y | 0.0 | Previous frame Y |
| 0x160 | float | prev_pos_z | 0.0 | Previous frame Z |
| **0x164** | **float** | **pos_x** | **0.0** | **Current position X** |
| **0x168** | **float** | **pos_y** | **0.0** | **Current position Y** |
| **0x16C** | **float** | **pos_z** | **0.0** | **Current position Z** |
| **0x170** | **float** | **force_x** | **0.0** | **Force accumulator X (cleared each frame, filled by ApplyForce)** |
| **0x174** | **float** | **force_y** | **0.0** | **Force accumulator Y** |
| **0x178** | **float** | **force_z** | **0.0** | **Force accumulator Z** |
| 0x17C | float | accel_x | 0.0 | Acceleration X |
| 0x180 | float | accel_y | 0.0 | Acceleration Y |
| 0x184 | float | accel_z | 0.0 | Acceleration Z |
| **0x188** | **float** | **max_speed** | **6.0** | **Speed cap (ctor2=5000.0, overridden to 6.0 by InitPhysicsDefaults)** |
| **0x18C** | **float** | **speed_scale** | **1.0** | **Global speed multiplier** |
| 0x190 | float | unknown_190 | -1.0 | Unknown (set to -1.0 in ctor2) |
| 0x194 | float | unknown_194 | -1.0 | Unknown (set to -1.0 in ctor2) |
| 0x19C | byte | unknown_19C | 0 | Unknown flag |
| **0x1A0** | **float** | **speed_cap_multiplier** | **0.2** | **Secondary cap multiplier (ctor2=1.0, overridden to 0.2)** |
| **0x1A4** | **PhysicsObject*** | **physics_object** | — | **PhysicsObject (CollisionMesh) pointer, size 0xCB0 — see [PhysicsObject](#physics-object)** |
| 0x1A8 | float[3] | gravity_vec | (0,1,0) | Gravity direction vector |
| 0x1B8 | — | render_ctx_1 | — | RenderContext sub-object |
| 0x1C8 | float | render_alpha | 0.75 | Render context #1 alpha |
| 0x1BC | float | render_scale_x | 0.25 | Render context scale |
| 0x1C0 | float | render_scale_y | 0.25 | Render context scale |
| 0x1C4 | float | render_scale_z | 0.25 | Render context scale |
| 0x204 | byte | render_flag_204 | 1 | Unknown render flag |
| 0x208 | — | render_ctx_2 | — | RenderContext sub-object #2 |
| 0x20C | float | color_r | 1.0 | RGBA red |
| 0x210 | float | color_g | 1.0 | RGBA green |
| 0x214 | float | color_b | 1.0 | RGBA blue |
| 0x218 | float | color_a | 1.0 | RGBA alpha |
| 0x23C | float | render2_r | 1.0 | Render context #2 RGBA |
| 0x240 | float | render2_g | 1.0 | Render context #2 RGBA |
| 0x244 | float | render2_b | 1.0 | Render context #2 RGBA |
| 0x248 | float | render2_a | 1.0 | Render context #2 RGBA |
| 0x254 | uint8 | uses_alpha | 0 | True if color_a != 1.0 |
| 0x25C | float | unknown_25C | 0.0 | Unknown (modified in Ball_Update spin friction) |
| 0x260 | uint8 | sweat_flag | 0 | Grip-climbing flag: 1 when airborne on slope (sweat mode), 0 at high speed |
| 0x264 | uint8[0x14] | toggle_timer1 | — | ArenaBoard timer sub-object |
| 0x26C | int32 | unknown_26C | 20 | Unknown int (0x14) |
| **0x278** | **float** | **gravity_scale** | **0.5** | **Gravity multiplier (ctor2=0.1, overridden to 0.5)** |
| 0x27C | float | unknown_27C | 0.2 | Unknown (ctor2=0.0, overridden to 0.2) |
| 0x280 | uint8 | unknown_280 | 0 | Unknown flag |
| 0x281 | uint8 | unused_init_flag | 1 | DEAD: set to 1 in ctor, 0 on spawn; never read by any function |
| **0x284** | **float** | **radius** | **35.0** | **Ball radius (ctor2=27.0, overridden to 35.0; shrunk to 13.0 on Odd Race shrink)** |
| 0x288 | float | unknown_288 | 0.0 | Unknown |
| 0x290 | uint8[0x14] | toggle_timer2 | — | Second ArenaBoard timer |
| 0x29C | float | unknown_29C | 1.0 | Unknown (modified in Ball_Update) |
| **0x2A4** | **float** | **spin_rate** | **5.0** | **Angular spin factor** |
| 0x2A8 | float[3] | speed_modifier | (0,0,0) | Vec3 speed modifier (init by Vec3_Init) |
| 0x2B8 | float[3] | accel_vec | (0,0,0) | Acceleration vector |
| 0x2C0 | float | force_x_2C0 | 0.0 | Secondary force accumulator X |
| 0x2C4 | float | force_y_2C4 | 0.0 | Secondary force accumulator Y |
| 0x2C8 | float | force_z_2C8 | 0.0 | Secondary force accumulator Z |
| 0x2CC | uint8 | force_disable | 0 | If 1, Ball_ApplyForce is skipped. Set by N:TARPIT and vacuum capture. |
| 0x2D5 | uint8 | unknown_2D5 | 0 | Unknown (cleared by timer decay) |
| 0x2D8 | int32 | unknown_2D8 | 0 | Unknown |
| **0x2DC** | **float** | **lgp_x** | **0.0** | **Last Grounded Position X (LGP)** |
| **0x2E0** | **float** | **lgp_y** | **0.0** | **Last Grounded Position Y (LGP)** |
| **0x2E4** | **float** | **lgp_z** | **0.0** | **Last Grounded Position Z (LGP)** |
| 0x2E8 | uint8 | event_flag | 0 | Needs-respawn (shattered) flag. Set by Ball_Shatter, Ball_FallUpdate (ground contact timer expiry), Ball_Update (off-screen/viewport), and E:SWALLOW (Odd Race pipe swallow in OddBoard_CollisionHandler) |
| 0x2E9 | uint8 | death_pending | 0 | ⚠ **NOT on_ramp/ground flag!** Sticky flag that prevents Ball_ApplyTrajectory from re-firing. Set by E:LIMIT, E:LIMITX, E:LIMITZ, E:LIMITPIPE1/2, speed>1.0 collision, and Ball_ApplyTrajectory itself. **NOT set by E:SWALLOW** (which sets +0x2E8 instead). Reset to 0 ONLY by Ball_InitPhysicsDefaults(0x405100) at 0x405262 and Ball_ctor2(0x4039E0) at 0x403BDE. See docs/agent-knowledge/death-pending-flag-deep-dive.md |
| 0x2EC | int32 | bounce_count | 0 | Dizzy system bounce counter. Double-incremented (0→1→2) when collision speed exceeds thresholds 0.03 and 0.1. When bounce_count > 1 AND death_pending==0 → Ball_ApplyTrajectory fires. Reset by Ball_DizzyImmunity, Ball_InitPhysicsDefaults, and when has_trajectory(+0x14D) is set |
| 0x2F0 | uint32 | force_count | 0 | Number of forces applied this frame |
| 0x2F4 | int32 | unknown_2F4 | 0 | Unknown |
| 0x2F8 | uint8 | update_in_progress | 0 | Set 1 during Ball_Update |
| 0x2F9 | uint8 | frozen | 0 | Stuck on surface (velocity zeroed) |
| 0x2FC | float | freeze_timer | 1.0 | Countdown while frozen (ctor2=1.0, not 150 as previously documented) |
| 0x300 | uint32 | freeze_val | 0 | (ctor2=0, not 150 as previously documented) |
| 0x30A | char* | display_string_ptr | NULL | Floating text string (freed when string_timer hits 0) |
| 0x30F | uint8 | teleport_flag | 0 | Teleport pending flag |
| 0x310 | uint8 | state_active | 1 | General active flag |
| 0x311 | float | teleport_x | 0.0 | Teleport destination X |
| 0x312 | float | teleport_y | 0.0 | Teleport destination Y |
| 0x313 | uint8 | unknown_313 | 0 | Camera/limit-related flag |
| 0x314 | float | ambient_sound_timer | 0.0 | Timer for ambient sound decay |
| 0x324 | uint8 | in_tube | 0 | If true, Ball_Update returns immediately — no physics! |
| 0x328 | int32 | unknown_328 | -1 | Unknown (set to 0xFFFFFFFF) |
| 0x32C | AthenaList | trail_list_32C | — | Trail particle list |
| 0x700 | int32 | sound_3d_handle | — | 3D sound effect handle |
| 0x744 | int32 | unknown_744 | 0 | Unknown |
| 0x748 | int32 | unknown_748 | 0 | Unknown |
| 0x74C | int32 | unknown_74C | 0 | Unknown |
| 0x750 | int32 | unknown_750 | 0 | Unknown |
| 0x754 | int32 | unknown_754 | 0 | Unknown |
| 0x768 | uint8 | cam_active | 1 | Camera follow enabled |
| 0x769 | uint8 | cam_flag_769 | 0 | Camera snap flag |
| 0x76A | uint8 | cam_flag_76A | 0 | Cleared on surface snap |
| 0x764 | float | cam_follow_factor | 1.0 | Camera follow lerp factor |
| 0x778-0x784 | float[4] | unknown_778 | 0.0 | Unknown camera-related fields |
| 0x788 | float[16] | matrix_2 | zeros | Second matrix (4×4, zeroed in ctor2) |
| 0x7C8 | float[16] | matrix_1 | identity | 4×4 transform matrix |
| 0x808 | int32 | impact_freeze_counter | 0 | If non-zero, Ball_ApplyForce is skipped. Counts down each frame via Ball_Update. Set by N:NOCONTROL (=10), E:CATAPULTBOTTOM (=1000), vacuum capture (=1000). |
| 0x810 | AthenaList | list_810 | — | Unknown list |
| **0xC28** | **char*** | **display_string** | **NULL** | **Floating text above ball** |
| 0x0C38 | int32 | unknown_C38 | -1 | Unknown (set to 0xFFFFFFFF) |
| 0xC3C | uint8 | teleport_active | 0 | Teleport in progress |
| 0xC40 | float | teleport_x | 0.0 | Teleport destination X |
| 0xC44 | float | teleport_y | 0.0 | Teleport destination Y |
| 0xC48 | float | teleport_z | 0.0 | Teleport destination Z |
| 0xC4C | uint8 | is_shrunk | 0 | Odd Race shrunk state (E:SHRINK=1, E:GROW=0) |
| 0xC50 | int32 | unknown_C50 | 0 | Unknown |
| 0xC54 | int32 | unknown_C54 | 0 | Unknown (AthenaList data pointer) |
| 0xC58 | uint8 | unknown_C58 | 0 | Unknown |
| 0xC5C | int32 | unknown_C5C | 0 | Unknown (alternate_state flag used by ApplyForce) |
| 0xC88 | float[16] | world_matrix | identity | 4×4 world transform for rendering |

> **Field naming note:** Fields at +0x170/+0x174/+0x178 were previously documented as "vel_x/y/z". They are actually **force accumulators** — cleared to zero at the start of each Ball_Update frame, then populated by Ball_ApplyForce. The actual per-frame velocity is the delta between `pos` (0x164) and `prev_pos` (0x158), computed internally by Ball_Update and stored temporarily.

---

## Physics Object

The field at **+0x1A4** is a pointer to a **PhysicsObject** (internally called `CollisionMesh`), allocated as `operator_new(0xCB0)` (3264 bytes) and constructed via `CollisionMesh_ctor` (0x456D80).

This is NOT just a collision mesh — it stores the ball's runtime physics state including gravity normal, computed velocity, collision entries, and AI parameters.

### Construction

```
CollisionMesh_ctor(this, ball_ptr)    // 0x456D80
  → Sets vtable to 0x4D8E10 (Mesh_DeletingDtor)
  → Stores ball back-reference at +0x10
  → Inits 3 AthenaLists at +0x18, +0x430, +0x848
  → Calls Ball_InitBattleMode(this)   // 0x456CD0
```

### PhysicsObject Internal Layout (size 0xCB0)

| Offset | Type | Default | Description |
|--------|------|---------|-------------|
| +0x000 | void** | 0x4D8E10 | Vtable (Mesh_DeletingDtor) |
| +0x010 | void* | ball ptr | Back-reference to owner Ball |
| +0x018 | AthenaList | — | **Collision entry list** (type 1=ball-ball, 2=wall, 5=floor entries) |
| +0x430 | AthenaList | — | Second collision node list |
| +0x848 | AthenaList | — | Third list (event collision objects) |
| +0xC60 | int32 | 3 | Battle mode state (3 = arena default) |
| +0xC64 | float | — | (not set in ctor) — written by Ball_Update as **speed** |
| +0xC68 | float | 0.555 | Battle mode friction parameter |
| +0xC6C | float | 1.0 → 600.0 | **CHASE distance** (overridden to 600.0 by InitPhysicsDefaults) |
| +0xC70 | float | 1000.0 → 1200.0 | **HOME distance** (overridden to 1200.0 by InitPhysicsDefaults) |
| +0xC74 | float | 0 | Speed value (written by Ball_SetSpeed) |
| +0xC78 | float | 25.0 → 0.0 | **spin_angle** (overridden to 0.0 by InitPhysicsDefaults) |
| +0xC7C | byte | 1 | Unknown flag |
| +0xC80 | float[3] | (0,0,0) | Unknown velocity vector |
| +0xC8C | float[3] | (0,-1.0,0) | **Gravity normal** (default points down) |
| +0xC98 | float[3] | — | **Computed velocity** (written every frame by Ball_Update) |
| +0xCA4 | float[3] | (0,0,0) | Direction vector (surface normal cache) |

### Collision Entry Structure

Each entry in the PhysicsObject+0x18 AthenaList is a struct accessed by Ball_Update as `int*`. Known fields:

| Index | Byte Offset | Type | Description |
|-------|-------------|------|-------------|
| [0] | +0x00 | int32 | **Type**: 1=ball-ball, 2=wall, 5=floor |
| [1] | +0x04 | int32 | Unknown (not read by Ball_Update — verify before using) |
| [3] | +0x0C | Ball* | **Other ball** (for type 1 = ball-ball collision) |
| [4] | +0x10 | Scene* | Board pointer (used for +0x434, +0x43C lookups) |
| [8] | +0x20 | float | Collision normal vector X |
| [9] | +0x24 | float | Collision normal vector Y |
| [10] | +0x28 | float | Collision normal vector Z |
| [12] | +0x30 | float | Secondary vector X |
| [13] | +0x34 | float | Secondary vector Y |
| [14] | +0x38 | float | Secondary vector Z |
| [21] | +0x54 | float | Collision depth/distance |
| [25] | +0x64 | PhysicsObject* | Owner physics object (compared against ball's physics) |

> **Note:** The `currCollision[1] == 4` check used in some mod frameworks targets offset +0x04, which is not read by Ball_Update. This field may be set during collision creation in `Ball_AdvancePositionOrCollision` (0x4564C0), but its meaning is unverified. Test with MessageBoxA dumps before relying on it.

---

## Vtable Methods

The Ball vtable lives at **0x4CF3A0** with **65+ entries** (not 9 as previously documented). The vtable is NOT terminated at index 9 — the NULL at index 9 is a valid entry, not an end marker.

### Documented Vtable Entries

| Vtable Offset | Address | Name | Description |
|---------------|---------|------|-------------|
| +0x00 | 0x4027F0 | Ball_dtor | Destructor — calls Ball_dtor2, optionally frees memory |
| +0x04 | 0x405100 | Ball_InitPhysicsDefaults | Sets runtime physics defaults (overrides ctor2 values) |
| +0x08 | 0x402DE0 | Ball_CollisionCheck | Per-frame collision against level mesh + planes |
| +0x0C | 0x402A70 | Ball_OnCollision | Collision response dispatcher |
| +0x10 | 0x408390 | Ball_AI_ChaseNearest | AI opponent steering (finds nearest ball, applies force) |
| +0x14 | 0x401590 | Ball_vtable5 | Unknown (called from collision path) |
| +0x18 | 0x402650 | Ball_ApplyForceWithMultipliers | Apply directional force vector to velocity accumulators |
| +0x1C | 0x402C10 | Ball_vtable7 | Unknown (render-related) |
| +0x20 | 0x409480 | Ball_SplitAndExplode | Called on E:BREAK collision events |
| ... | ... | ... | (many more entries up to 65+) |
| +0x104 | 0x408830 | Ball_FallUpdate | Fall animation + respawn logic |

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

// Zero force accumulators (emergency stop)
*(float*)(ball + 0x170) = 0.0f;
*(float*)(ball + 0x174) = 0.0f;
*(float*)(ball + 0x178) = 0.0f;
```

> **Note:** +0x170/+0x174/+0x178 are force accumulators, NOT persistent velocity. They are cleared to zero at the start of each Ball_Update frame. The actual per-frame velocity is computed internally as `pos - prev_pos`.

### Physics Constants (Game Data Section)

These global floats at **0x4CF3xx** affect ALL balls. Patch once, affects every frame:

| Address | Value | Effect |
|---------|-------|--------|
| 0x4CF368 | 0.0 | Float epsilon |
| 0x4CF36C | 0.75 | Force multiplier when is_shrunk (Odd Race) |
| 0x4CF374 | 0.2 | Force multiplier on ice |
| 0x4CF378 | 0.0 | Force multiplier in tube (0 = no control!) |
| 0x4CF380 | 0.25 | Force multiplier after first frame of input |
| 0x4CF39C | 0.037 | Collision radius multiplier |
| 0x4CF3E8 | 6.0 | Ice friction factor |
| 0x4CF3F0 | 0.95 | Damping constant |
| 0x4CF484 | 40.0 | Collision mesh distance threshold |
| 0x4CF48C | 2.0 | Y offset threshold (radius + epsilon) |

### Per-Ball Physics Overrides

| Field | Offset | Runtime Default | What It Does |
|-------|--------|-----------------|--------------|
| radius | 0x284 | 35.0 | Collision + visual size. Shrunk to 13.0 on Odd Race shrink. |
| max_speed | 0x188 | 6.0 | Hard velocity cap. ctor2=5000.0, InitPhysicsDefaults=6.0. |
| speed_scale | 0x18C | 1.0 | Global multiplier on ALL velocity changes. |
| gravity_scale | 0x278 | 0.5 | Gravity strength multiplier. ctor2=0.1, InitPhysicsDefaults=0.5. |
| speed_cap_multiplier | 0x1A0 | 0.2 | Secondary cap multiplier. ctor2=1.0, InitPhysicsDefaults=0.2. |
| in_tube | 0x324 | 0 | If non-zero, Ball_Update returns immediately — no physics! |
| force_disable | 0x2CC | 0 | If non-zero, Ball_ApplyForce is skipped entirely. |

> **Mod idea:** Setting `speed_scale = 2.0f` gives a permanent speed boost. Setting `gravity_scale = 0.0f` makes the ball weightless (but collision snapping still applies).

---

## Physics Pipeline

### Ball_Update (0x405E00) — The Main Tick

Called once per frame for every active ball from `Scene_UpdateBallsAndState` (0x41B540).

**Phase 1 — Reset & Timer Decay:**
```
ambient_sound_timer *= decay_factor
Various timer decrements (trail, boost, sound cooldowns)
string_timer decrements (frees display_string at 0xC28 when it hits 0)
```

**Phase 2 — Trail Particles:**
If trail timer active, spawn ArenaScoreParticle particle at ball position + random offset.

**Phase 3 — Force Accumulator Save & Clear:**
```
prev_pos = pos                    // Save current position
saved_force = force_accumulators   // Copy +0x170/+0x174/+0x178
force_accumulators = (0,0,0)       // Clear for next frame
```

**Phase 4 — Spatial Tree Build:**
Build a temporary SpatialTree from the scene's collision mesh for this frame's collision queries.

**Phase 5 — Collision Iteration:**
Iterate PhysicsObject+0x18 AthenaList entries:
- **Type 1 (ball-ball):** Compute collision response, apply forces, play sound, award points
- **Type 2 (wall):** Reflect velocity, update surface normal, apply friction
- **Type 5 (floor):** Set ground flag, snap to surface, trigger limit/trajectory events

**Phase 6 — Spin & Roll Physics:**
3 iterations of spin friction computation using gravity normal and surface velocity.

**Phase 7 — Position Integration:**
```
pos += computed_velocity           // Apply final velocity
display_pos = lerp(display_pos, pos, follow_factor)
```

**Phase 8 — Teleport Override:**
If teleport_flag (0x30F) is set, override position with teleport destination.

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
3. Accumulate the result into **force accumulators at ball+0x170**
4. Increment `force_count` at **ball+0x2F0**

### Conditional Multipliers (Applied in Order)

| Condition | Field / Global | Multiplier | Effect |
|-----------|---------------|------------|--------|
| `force_count == 0` (first frame) | `DAT_004CF380` | `1.0` | Full force on first press |
| `force_count > 0` (held) | `DAT_004CF380` | `0.25` | Quarter force while held |
| `in_tube` (ball+0x324) | `DAT_004CF378` | `0.0` | **Zero force — completely disabled** |
| `on_ice` flag | `DAT_004CF374` | `0.2` | Ice: 20% force + friction 6.0 |
| `is_shrunk` flag (ball+0xC4C) | `DAT_004CF36C` | `0.75` | Shrunk (Odd Race): 75% force |
| `alternate_state` (ball+0xC5C) | `DAT_004CF374` | `0.2` | Alternate physics mode |
| Direction tweak | `DAT_004CF3E8` | `6.0` | Scales the final direction vector |

### Early-Out Checks

`Ball_ApplyForce` **returns immediately** (does nothing) if ANY of these are true:

| Condition | Offset | Value |
|-----------|--------|-------|
| `frozen` | ball+0x2F9 | `1` — ball is snapped to a surface |
| `force_disable` | ball+0x2CC | `1` — force application globally disabled |
| `freeze_counter` | ball+0x2FC | `> 0` — input freeze timer active |

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

> **Pitfall:** Writing directly to `ball+0x170` (force accumulators) bypasses the multipliers, clamping, and surface-snap logic. This is fine for teleport-style hacks, but for gameplay-compatible movement, **always use `Ball_ApplyForce`** so the engine handles ice, tubes, is_shrunk state, and max_speed correctly.

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
| 0x170–0x178 (force) | Cleared by TARPIT, modified by JUMP |
| 0x2DC–0x2E4 (checkpoint) | `Ball_FindClosestRespawnPoint` respawns here |
| 0xC2C (section_filter) | `E:SAFESWITCH` copies data here |
| 0xC4C (is_shrunk) | Set by Odd Race E:SHRINK/E:GROW |

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
- Each particle is a `ArenaScoreParticle` object (0x28 bytes)
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
1. Shrink radius from 35.0 → 13.0
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
- `radius = 26.0` (note: different from runtime default 35.0!)
- Restore physics defaults

### Split Ball Mechanics

| Function | Address | What It Does |
|----------|---------|--------------|
| Ball_Split_ctor | 0x408D10 | Constructor for split ball (vtable 0x4CF560) |
| Ball_Shatter | 0x408D70 | Arena 8-ball mechanic: marks parent ball for despawn, spawns 3 AI split balls (called from FollowBall_Update 0x43ECC0) |
| Ball_SplitAndExplode | 0x409480 | Creates 2 split balls + circular ArenaScoreParticle explosion (0–360°) |

Split balls are temporary physics objects that scatter from the original ball position and expire after a timer.

### Ball_InitBattleMode (0x456CD0)

Initializes ball for Rodent Rumble arena mode:
- Friction = 0.555
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
| 6 | Ball_Shrink | 0x402200 | On Odd Race | Prevent shrink, teleport instead |
| 7 | Scene_UpdateBallsAndState | 0x41B540 | Every frame | Modify ball list iteration, add/remove balls |
| 8 | Ball_AI_ChaseNearest | 0x408390 | AI tick | Change AI behavior, make AI friendly/hostile |
| 9 | Scene_dtor | 0x419770 | Scene destroy | Null out cached ball/scene pointers (use-after-free prevention) |
| 10 | Board_ctor | 0x419030 | Scene create | Detect new level load, acquire scene pointer |

### Scene Lifecycle Hook Pattern

For mods that cache ball/scene pointers across frames, hook both construction and destruction to avoid use-after-free:

```cpp
// In Ball_Update hook (acquire pointers)
void Hooked_BallUpdate(Ball* ball) {
    if (ball->player_index == 0) {
        g_Player = ball;
        if (g_Scene == nullptr) {
            g_Scene = ball->scene;  // nullptr→addr transition = new level loaded
        }
    }
    Original_BallUpdate(ball);
}

// In Scene_dtor hook (null pointers BEFORE calling original)
void Hooked_SceneDtor(Scene* scene) {
    if (scene == g_Scene) {
        g_Scene = nullptr;       // null FIRST, before balls get freed
        g_Player = nullptr;
        // g_Player2/3/4 = nullptr;
        // g_Enemies.clear();
    }
    Original_SceneDtor(scene);   // now safe — original frees balls
}
```

**Destruction order inside Scene_dtor:**
1. Vtable pointer overwritten (data fields still intact)
2. Ball list iterated — every ball gets `Ball_dtor(this, 1)` → `free()`
3. Effect/object lists iterated and freed
4. AthenaList containers freed
5. `SceneObject_dtor` base cleanup
6. Returns → caller calls `free(scene)`

The scene's own inline fields (camera, timers, race state) remain readable throughout, but every **pointer** the scene holds becomes dangling partway through. Always null your cached pointers at the top of your hook before calling the original.

### Example: Speed Hack (Single Write)

```cpp
// Multiply max_speed for all balls by 2x
*(float*)(ball + 0x188) = 12.0f;  // 6.0 default × 2
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
    float* force = (float*)((char*)ball + 0x170);
    force[0] *= 0.85f;
    force[1] *= 0.85f;
    force[2] *= 0.85f;
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
| 0x402200 | Ball_Shrink | — | Odd Race shrink ball, reduce radius |
| 0x402270 | Ball_Grow | — | Exit Odd Race shrink state |
| 0x402650 | Ball_ApplyForceWithMultipliers | 47 | Apply force vector to velocity accumulators |
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
| 0x405100 | Ball_InitPhysicsDefaults | — | vtable[1] — overrides ctor2 physics defaults |
| 0x405E00 | Ball_Update | 400+ | Main physics tick |
| 0x408390 | Ball_AI_ChaseNearest | 60 | AI opponent steering |
| 0x408830 | Ball_FallUpdate | 40 | Fall animation + respawn |
| 0x408D10 | Ball_Split_ctor | 14 | Split ball constructor |
| 0x408D70 | Ball_Shatter | 50 | Arena: split parent ball into 3 AI balls |
| 0x409480 | Ball_SplitAndExplode | 70 | Split + ArenaScoreParticle ring |
| 0x40AFE0 | Ball_ctor | 30 | Primary allocator + base init |
| 0x40AF90 | Ball_GetTransform | — | Read transform into struct |
| 0x4564C0 | Ball_AdvancePositionOrCollision | — | 6-phase physics pipeline |
| 0x456CD0 | Ball_InitBattleMode | — | Arena physics defaults |
| 0x456D80 | CollisionMesh_ctor | — | PhysicsObject constructor (0xCB0 bytes) |
| 0x46EC30 | Ball_GetInputForce | 7 | Convert input to force vector |

### Scene Lifecycle Functions

| Address | Name | Description |
|---------|------|-------------|
| 0x419030 | Board_ctor | Base board constructor (calls Gadget_ctor → Scene init) |
| 0x419770 | Scene_dtor | Scene destructor (frees balls, objects, lists) |
| 0x425020 | Scene_DeletingDtor | Calls Scene_dtor + free(this) |
| 0x458CE0 | Scene_Destroy | Calls Scene_ScalarDtor (alternate dtor path) |

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

*Document compiled from 3,700+ documented functions, live Ghidra decompilation, and cross-referenced struct analysis. All offsets verified against PE32 Hamsterball.exe loaded at 0x00400000. PhysicsObject layout verified via CollisionMesh_ctor (0x456D80) and Ball_InitBattleMode (0x456CD0) decompilation. Runtime defaults verified via Ball_InitPhysicsDefaults (0x405100) decompilation.*
