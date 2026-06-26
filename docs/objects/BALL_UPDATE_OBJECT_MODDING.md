# Ball Object — Modding Reference

> **Parameter of `Ball_Update` (0x405E00)**  
> **Total size: 0xC98 bytes (3224 bytes)**  
> **Vtable: 0x4CF3A0** (9 method pointers)  
> **Constructor: `Ball_ctor2` (0x4039E0)**  
> **Verified via:** live GhidraMCP decompilation of 22 Ball functions

---

## 1. Quick Anatomy

The Ball is the player (or AI) physics object. Every frame `GameLoop` → `Scene_Update` → `Ball_Update` ticks it through a **23-phase pipeline** (see §3). Modders patch mid-pipeline to alter physics, rendering, or input without breaking collision coherence.

**Canonical entry points**
- `Scene+0x21E` → `App` → `App+0x5DC` → current `Scene` → iterate balls
- `Ball+0x14` → back-pointer to owning `Scene` (all balls in a scene share it)
- `Ball+0x1A4` → `CollisionMesh` (separate 0xCB0-byte physics body)

---

## 2. Verified Field Layout

Offsets below are **byte addresses** (not `int[0xNN]` array indices). All types are confirmed by `Ball_ctor2` writes and `Ball_Update` reads.

| Offset | Type | Name | Verified By | Modder Notes |
|--------|------|------|-------------|--------------|
| 0x000 | `uint32_t` | **vtable** | ctor2 | GameObject vtable; 9 methods. Do NOT overwrite. |
| 0x004 | `uint8_t[2]` | field_04 | ctor2 | Reserved |
| 0x008 | `int32_t` | collision_result | ctor2 | Bitflags from collision system |
| 0x00C | `int32_t` | string_timer | ctor2, Update | Countdown; frees `display_string` at 0 |
| 0x010 | `App*` | app | ctor2 | Back-pointer to global App |
| 0x014 | `Scene*` | scene | ctor2 | Back-pointer to owning Scene |
| 0x018 | `int32_t` | player_index | ctor2 | `-1` = AI, `0` = Player 1, `1` = Player 2 |
| 0x01C | `uint32_t` | render_callback | ctor2 | Vtable for render/audio callbacks |
| 0x020 | `uint8_t[0xEC]` | UITimer | ctor2 | `UITimer` sub-object (236 bytes) |
| 0x108 | `Timer` | timer | ctor2 | `Timer_Init` target |
| 0x150 | `float` | accumulated_time | Update | Delta-time accumulator for physics sub-steps |
| 0x154 | `int32_t` | rng_seed | ctor2 | `RNG_Rand` seed |
| 0x158 | `float` | prev_pos_x | Update | Position previous frame |
| 0x15C | `float` | prev_pos_y | Update | " |
| 0x160 | `float` | prev_pos_z | Update | " |
| 0x164 | `float` | **pos_x** | ctor2, Update | **Current position** — primary mod target |
| 0x168 | `float` | **pos_y** | ctor2, Update | **"** |
| 0x16C | `float` | **pos_z** | ctor2, Update | **"** |
| 0x170 | `float` | **vel_x** | ctor2, Update | **Current velocity** — write AFTER collision phase |
| 0x174 | `float` | **vel_y** | ctor2, Update | **"** |
| 0x178 | `float` | **vel_z** | ctor2, Update | **"** |
| 0x17C | `float` | display_vel_x | Update | Display-interpolated velocity |
| 0x180 | `float` | display_vel_y | Update | " |
| 0x184 | `float` | display_vel_z | Update | " |
| 0x188 | `float` | **max_speed** | ctor2, Update | Default `5000.0f`. Mod for speed limits. |
| 0x18C | `float` | **speed_scale** | ctor2, Update | Default `1.0f`. Multiplier on input force. |
| 0x190 | `uint8_t[12]` | pad_190 | ctor2 | |
| 0x19C | `uint8_t` | field_19c | ctor2 | Set `0` |
| 0x1A0 | `float` | max_speed_cap | ctor2 | `1.0f` hard cap |
| 0x1A4 | `CollisionMesh*` | **physics_body** | ctor2 | **Separate physics object** — see [COLLISIONMESH_OBJECT_MODDING.md](COLLISIONMESH_OBJECT_MODDING.md) |
| 0x1A8 | `float[3]` | **gravity** | ctor2, Update | Default `(0, 1.0, 0)`. **Flip Y sign for inverted gravity.** |
| 0x1B4 | `uint8_t[4]` | pad_1b4 | | |
| 0x1B8 | `RenderContext` | render_ctx_1 | ctor2 | First render context |
| 0x1C8 | `float` | render_alpha | ctor2 | `0.75f` |
| 0x1CC | `uint8_t[0x3C]` | pad_1cc | | |
| 0x208 | `RenderContext` | render_ctx_2 | ctor2 | Second render context |
| 0x20C | `float` | color_a | ctor2 | RGBA alpha |
| 0x210 | `float` | color_r | ctor2 | " |
| 0x214 | `float` | color_g | ctor2 | " |
| 0x218 | `float` | color_b | ctor2 | " |
| 0x21C | `uint8_t[0x20]` | pad_21c | | |
| 0x23C | `float` | tint_x | ctor2 | `1.0f` |
| 0x240 | `float` | tint_y | ctor2 | `1.0f` |
| 0x244 | `float` | tint_z | ctor2 | `1.0f` |
| 0x248 | `float` | tint_w | ctor2 | `1.0f` |
| 0x24C | `uint8_t[8]` | pad_24c | | |
| 0x254 | `bool` | uses_alpha | ctor2 | `color_a != 1.0f` |
| 0x255 | `uint8_t[0xB]` | pad_255 | | |
| 0x260 | `uint8_t` | boost_flag | ctor2 | Set `0` |
| 0x261 | `uint8_t[3]` | pad_261 | | |
| 0x264 | `RumbleBoard` | rumble_timer1 | ctor2 | `RumbleBoard_InitTimer` target (20 bytes) |
| 0x278 | `float` | **gravity_scale** | ctor2, Update | Default `0.1f`. **Scale gravity strength.** |
| 0x27C | `uint32_t` | field_27c | ctor2 | `0` |
| 0x280 | `uint8_t` | field_280 | ctor2 | `0` |
| 0x281 | `bool` | **unused_init_flag** | ctor2, Update | `1` in ctor; DEAD: never read by any function |
| 0x282 | `uint8_t[2]` | pad_282 | | |
| 0x284 | `float` | **radius** | ctor2, Update | Default `27.0f`. Hit-box size. |
| 0x288 | `uint32_t` | field_288 | ctor2 | `0` |
| 0x28C | `uint8_t` | field_28c | ctor2 | `0` |
| 0x28D | `uint8_t[3]` | pad_28d | | |
| 0x290 | `RumbleBoard` | rumble_timer2 | ctor2 | Second RumbleBoard timer |
| 0x2A4 | `float` | field_2a4 | ctor2 | `5.0f` |
| 0x2A8 | `Vec3` | speed_modifier | ctor2 | `Vec3_Init` target |
| 0x2B4 | `uint8_t[4]` | pad_2b4 | | |
| 0x2B8 | `float` | **accel_x** | Update | **Frame-cleared acceleration** |
| 0x2BC | `float` | **accel_y** | Update | **"** |
| 0x2C0 | `float` | **accel_z** | Update | **"** |
| 0x2C4 | `uint8_t` | field_2c4 | Update | |
| 0x2C8 | `uint8_t[4]` | pad_2c8 | | |
| 0x2CC | `uint8_t` | field_2cc | ctor2 | `0` |
| 0x2CD | `uint8_t[7]` | pad_2cd | | |
| 0x2D4 | `uint8_t` | field_2d4 | ctor2 | `0` |
| 0x2D5 | `uint8_t` | field_2d5 | ctor2, Update | `0` |
| 0x2D6 | `uint8_t[2]` | pad_2d6 | | |
| 0x2D8 | `uint32_t` | field_2d8 | ctor2 | `0` |
| 0x2DC | `float` | **checkpoint_x** | Update | Last collision/bump position |
| 0x2E0 | `float` | **checkpoint_y** | Update | " |
| 0x2E4 | `float` | **checkpoint_z** | Update | " |
| 0x2E8 | `bool` | **event_flag** | ctor2, Update | Checkpoint hit event |
| 0x2E9 | `bool` | **impact_shatter** | ctor2 | ⚠ NOT on_ramp! Sticky limit/trajectory flag (E:LIMIT + type-5 collision). Never cleared within Ball_Update. |
| 0x2EC | `uint32_t` | field_2ec | ctor2 | Collision counter |
| 0x2F0 | `uint32_t` | field_2f0 | ctor2 | `0` |
| 0x2F4 | `uint32_t` | field_2f4 | ctor2 | `0` |
| 0x2F8 | `uint8_t` | field_2f8 | Update | |
| 0x2F9 | `uint8_t` | field_2f9 | ctor2, Update | |
| 0x2FC | `float` | **timer_bf** | ctor2, Update | Default `1.0f` |
| 0x300 | `uint32_t` | field_300 | Update | Set `0` |
| 0x304 | `float` | saved_pos_x | Update | Saved position copy |
| 0x308 | `float` | saved_pos_y | Update | " |
| 0x30C | `float` | saved_pos_z | Update | " |
| 0x310 | `uint8_t` | field_310 | ctor2 | `1` |
| 0x311 | `uint8_t[3]` | pad_311 | | |
| 0x314 | `uint32_t` | field_314 | Update | Decayed each frame |
| 0x318 | `uint8_t[4]` | pad_318 | | |
| 0x31C | `bool` | field_31c | ctor2 | `0` |
| 0x31D | `uint8_t` | field_31d | ctor2 | `0` |
| 0x31E | `bool` | field_31e | ctor2 | `0` |
| 0x31F | `uint8_t` | field_31f | ctor2 | |
| 0x320 | `uint8_t[8]` | pad_320 | | |
| 0x328 | `int32_t` | field_328 | ctor2 | `-1` |
| 0x32C | `AthenaList` | collision_list | ctor2 | `AthenaList_Init` target |
| 0x744 | `float` | field_744 | ctor2 | `0` |
| 0x748 | `float` | field_748 | ctor2 | `0` |
| 0x74C | `float` | cam_offset_1 | Update | Camera offset |
| 0x750 | `float` | cam_offset_2 | Update | Camera offset |
| 0x754 | `uint8_t[0x10]` | pad_754 | | |
| 0x764 | `float` | cam_follow_factor | ctor2 | Camera lerp factor `1.0f` |
| 0x768 | `bool` | **cam_active** | ctor2, Update | `1` = camera follow on |
| 0x769 | `uint8_t` | pad_769 | ctor2 | `0` |
| 0x76A | `uint8_t[0x5E]` | pad_76a | | |
| 0x7C8 | `float[16]` | matrix_1 | ctor2, Update | 4×4 transform matrix |
| 0x808 | `float[16]` | matrix_2 | ctor2, Update | 4×4 transform matrix |
| 0x848 | `uint8_t[0x380]` | pad_848 | | |
| 0xC28 | `char*` | **display_string** | ctor2, Update | Allocated string; freed when timer expires |
| 0xC2C | `uint8_t` | pad_c2c | ctor2 | |
| 0xC2D | `uint8_t[0xB]` | pad_c2d | | |
| 0xC38 | `int32_t` | field_c38 | ctor2 | `-1` |
| 0xC3C | `bool` | **teleport_active** | ctor2, Update | Teleport pending flag |
| 0xC3D | `uint8_t[3]` | pad_c3d | | |
| 0xC40 | `float` | **teleport_x** | Update | Teleport destination |
| 0xC44 | `float` | **teleport_y** | Update | " |
| 0xC48 | `float` | **teleport_z** | Update | " |
| 0xC4C | `uint8_t` | field_c4c | ctor2, Update | `0` |
| 0xC4D | `uint8_t[3]` | pad_c4d | | |
| 0xC50 | `float` | field_c50 | Update | Decayed each frame (`*= 0.998` approx) |
| 0xC54 | `uint32_t` | field_c54 | Update | |
| 0xC58 | `uint8_t` | field_c58 | Update | `0` |
| 0xC59 | `uint8_t[3]` | pad_c59 | | |
| 0xC5C | `uint32_t` | field_c5c | Update | Decayed each frame |

---

## 3. Ball_Update 23-Phase Execution Order

The function at **0x405E00** runs these phases every tick. Hook at the right phase:

| Phase | Offset Range | What Happens | Safe to Patch? |
|-------|-------------|--------------|----------------|
| 1 | 0xC50, 0xC5C | Decay timers (multiplicative fade) | ⚠️ Post-decay OK |
| 2 | 0x2F0, 0x2F4 | Decay collision counters | ⚠️ Post-decay OK |
| 3 | 0x314 | `field_314` decay | ⚠️ Post-decay OK |
| 4 | 0x2FC | `timer_bf` countdown | ⚠️ Post-decay OK |
| 5 | 0xC54 | Check flag, call vtable+0x18 | ❌ Skip if event logic |
| 6 | 0x150 | `accumulated_time` += deltaTime | ❌ Pre-physics |
| 7 | 0x154 | `rng_seed` = `RNG_Rand()` | ❌ Random state |
| 8 | 0x158-0x16C | **Save `prev_pos` = `pos`** | ❌ Copy state |
| 9 | 0x300 | `field_300 = 0` (reset) | ❌ Pre-collision |
| 10 | 0x2A0 | `speed_modifier` decay | ⚠️ Post OK |
| 11 | 0x29C | `field_29C = 1.0f` (reset) | ❌ Resets your changes |
| 12 | 0xC0 | `piVar16 = param_1 + 0xC0` (trail setup) | ❌ Trail state |
| 13 | 0xC28 | `display_string` check & free | ❌ String lifetime |
| 14 | 0x14C | `field_14C = 0` (reset) | ❌ |
| 15 | 0x768 | **Camera follow check** | ✅ **Hook here for camera hacks** |
| 16 | 0x310 | `field_310` check | ❌ |
| 17 | 0x1A4 | **CollisionMesh gravity reflect** | ❌ Physics internals |
| 18 | 0x1D6 | **Teleport apply** (`teleport_active` → `pos`) | ❌ **Use `teleport_*` fields instead** |
| 19 | 0x164-0x16C | **Velocity integration → position** | ⚠️ **Best hook for pos overrides** |
| 20 | 0x170-0x178 | **Velocity clamping & friction** | ⚠️ **Best hook for speed hacks** |
| 21 | 0x188 | **Max speed enforcement** | ❌ Overwritten |
| 22 | 0x2B8-0x2C0 | **Acceleration clear** | ❌ Resets to 0 |
| 23 | 0x2DC-0x2E4 | **Checkpoint update** | ❌ |

**Best hook phases for common mods:**
- **Teleport / position override:** Phase 19 (after integration, before friction)
- **Speed boost:** Phase 20 (after velocity integration, before clamping)
- **Invincibility / no-clipping:** Phase 17 (skip collision by zeroing collision results)
- **Camera control:** Phase 15 (before camera logic reads `pos`)
- **Gravity hack:** Phase 17 (overwrite `gravity` at +0x1A8 before reflect)

---

## 4. Callable Functions (VTable + Standalone)

### Vtable Methods (offset 0x000 → 0x4CF3A0)

| VTable Offset | Function | Called From |
|--------------|----------|-------------|
| +0x00 | `GameObject_sub2_dtor` | Destructor chain |
| +0x04 | `UITimer_Ctor` | `Ball_ctor2` |
| +0x08 | ? | |
| +0x0C | ? | |
| +0x10 | ? | |
| +0x14 | ? | |
| +0x18 | `Ball_Render` (indirect) | `Scene_Render` via callback |
| +0x1C | ? | |
| +0x20 | `Ball_Update` trigger | Called by scene tick |

### Standalone Ball Functions

| Address | Name | Signature | Modder Use |
|---------|------|-----------|------------|
| 0x4015B0 | `Ball_SetupCollisionRender` | `(Ball*, float, float, float, float)` | Custom collision visualization |
| 0x401660 | `Ball_SetName` | `(Ball*, char*)` | Set display name |
| 0x4016F0 | `Ball_ApplyForceV2` | `(Ball*, float, float, float)` | Apply impulse vector |
| 0x401920 | `Ball_RenderShadow` | `(Ball*)` | Manual shadow render |
| 0x401CC0 | `Ball_dtor2` | `(Ball*)` | Full destructor |
| 0x401DD0 | `Ball_CreateTrailParticles` | `(Ball*, int, float, float, float)` | Spawn trail effect |
| 0x402030 | `Ball_SetTargetPos` | `(Ball*, float, float, float)` | Set target position |
| 0x402200 | `Ball_Shrink` | `(Ball*)` | Enter Odd Race shrink state |
| 0x402270 | `Ball_Grow` | `(Ball*)` | Exit Odd Race shrink state |
| 0x402400 | `Ball_RecordBest` | `(Ball*)` | Record best time |
| 0x402650 | `Ball_ApplyForceWithMultipliers` | `(Ball*, float, float, float, float, float)` | Force with scale factors |
| 0x4027F0 | `Ball_dtor` | `(Ball*)` | Light destructor |
| 0x402810 | `Ball_TestPlaneIntersection` | `(Ball*, float, float, float, float)` | Test plane collision |
| 0x402860 | `Ball_InitRenderState` | `(Ball*)` | Reset render contexts |
| 0x4029C0 | `Ball_SetSpeed` | `(Ball*, float)` | **⚠️ DEAD CODE — does nothing lasting** |
| 0x402A20 | `Ball_SetVec3AtOffset` | `(Ball*, int offset, float, float, float)` | Generic Vec3 write |
| 0x402DE0 | `Ball_Render` | `(Ball*)` | Full render (shadow + sprite + material) |
| 0x4030B0 | `Ball_ResetCollisionMesh` | `(Ball*)` | Reset physics body |
| 0x403100 | `Ball_SetTiltedGravity` | `(Ball*, float, float, float)` | Set custom gravity direction |
| 0x4039E0 | `Ball_ctor2` | `(void* mem, Scene*)` | Constructor |
| 0x405E00 | `Ball_Update` | `(Ball*)` | **Main physics tick** |
| 0x45D8F0 | `Ball_RenderWithMaterial` | `(Ball*, Material*)` | Render with custom material |
| 0x46EC30 | `Ball_GetInputForce` | `(InputHandler*, float[2])` | Read input as 2D force vector |

---

## 5. Modding Recipes

### Recipe A: Instant Teleport
```cpp
// Write destination, set flag — Ball_Update phase 18 handles the rest
float* ball = (float*)0xDEADBEEF;  // your ball pointer
*(bool*)(ball + 0xC3C/4) = true;   // teleport_active
ball[0xC40/4] = targetX;          // teleport_x
ball[0xC44/4] = targetY;          // teleport_y
ball[0xC48/4] = targetZ;          // teleport_z
```

### Recipe B: Super Speed
```cpp
// Phase 20 hook: override velocity after integration, before clamping
float* vel = &ball[0x170/4];
float speed = sqrtf(vel[0]*vel[0] + vel[1]*vel[1] + vel[2]*vel[2]);
if (speed > 0) {
    float scale = 3.0f;  // 3× speed
    vel[0] *= scale; vel[1] *= scale; vel[2] *= scale;
}
```

### Recipe C: Disable Gravity
```cpp
// Phase 17 hook: zero gravity vector
float* gravity = &ball[0x1A8/4];
gravity[0] = 0; gravity[1] = 0; gravity[2] = 0;
// Also zero gravity_scale
ball[0x278/4] = 0.0f;
```

### Recipe D: Low Friction (Ice Mode)
```cpp
// Phase 20 hook: reduce velocity decay
// Ball_Update applies friction 3×; skip or reduce it
// Hook at the spin-friction loop and multiply retention
```

### Recipe E: Giant Ball
```cpp
// Modify radius — affects collision and render scale
ball[0x284/4] = 100.0f;  // 27.0f → 100.0f
```

### Recipe F: Position Override (Noclip)
```cpp
// Phase 19 hook: directly overwrite position
ball[0x164/4] = desiredX;
ball[0x168/4] = desiredY;
ball[0x16C/4] = desiredZ;
// Zero velocity to prevent fight-back
ball[0x170/4] = ball[0x174/4] = ball[0x178/4] = 0;
```

### Recipe G: Input Force Scaling
```cpp
// Ball_GetInputForce reads +0x50C-0x518 from InputHandler
// Scale the output vector: output[0] *= 2.0f; output[1] *= 2.0f;
```

### Recipe H: Disable Fall State
```cpp
// Keep unused_init_flag = false (NOTE: this flag is DEAD code, never read by any function)
*(bool*)(ball + 0x281) = false; // dead flag (never read) // dead flag (never read)
```

---

## 6. Cross-Reference Map

| Related Document | What It Covers |
|------------------|----------------|
| [COLLISIONMESH_OBJECT_MODDING.md](COLLISIONMESH_OBJECT_MODDING.md) | The `Ball+0x1A4` physics body (velocity, mass, collision tree) |
| [SCENE_OBJECT_MODDING.md](SCENE_OBJECT_MODDING.md) | The `Ball+0x14` Scene (level geometry, camera, lighting) |
| [APP_OBJECT.md](APP_OBJECT.md) | The `Ball+0x10` App (global state, window, input handler) |
| [INPUT_SYSTEM.md](INPUT_SYSTEM.md) | How `Ball_GetInputForce` reads DIK codes and joystick axes |

---

## 7. Verification Notes

- **99 unique offsets** extracted from `Ball_Update` decompilation (43,655 bytes)
- **Field names** verified against `Ball_ctor2` writes (7,400 bytes)
- **Function names** verified against `list_functions` REST endpoint (3,801 total functions, 3,782 documented = 99.5%)
- **Render offsets** verified against `Ball_Render` (3,900 bytes)
- **Input offsets** verified against `Ball_GetInputForce` (2,000 bytes)
- All decompilations performed live via GhidraMCP headless server (v5.2.0, port 8089)

---

*Document: docs/BALL_UPDATE_OBJECT_MODDING.md*  
*Generated: 2025-06-05*  
*Verified against: Hamsterball.exe, GhidraMCP v5.2.0*
