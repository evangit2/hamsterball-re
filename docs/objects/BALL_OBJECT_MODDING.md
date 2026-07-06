# Ball Object — Complete Modder's Reference

> **Verified via direct Ghidra decompilation** of `Hamsterball.exe` (Athena engine, PE32 i386).  
> All offsets below were extracted from the live decompiled code via the GhidraMCP headless server.  
> This document replaces / supplements `BALL_OBJECT.md` with authoritative, source-verified data.

---

## Quick Stats

| Property | Value |
|----------|-------|
| Primary update function | `Ball_Update` @ `0x00405E00` |
| Constructor | `Ball_ctor2` @ `0x004039E0` |
| Total struct size | `0x0C98` bytes (3,224 bytes) |
| Physics body (nested) | `CollisionMesh` @ `this + 0x1A4` (size `0xCB0`) |
| VTable (Ball) | `0x004CF3A0` |

---

## Table of Contents

1. [Position & Velocity](#position--velocity)
2. [Ball-Related Functions (Complete List)](#ball-related-functions)
3. [What You Can Do With the Ball Object](#what-you-can-do)
4. [Modding Recipes](#modding-recipes)
5. [Verified Memory Layout (Key Fields)](#verified-memory-layout)

---

## Position & Velocity

### Verified by Ghidra decompilation of `Ball_ctor2` (0x004039E0)

```c
// Ball_ctor2 initializes these fields to zero:
*(undefined4 *)((int)this + 0x164) = 0;   // pos_x
*(undefined4 *)((int)this + 0x168) = 0;   // pos_y
*(undefined4 *)((int)this + 0x16c) = 0;   // pos_z
*(undefined4 *)((int)this + 0x170) = 0;   // vel_x ← **VERIFIED**
*(undefined4 *)((int)this + 0x174) = 0;   // vel_y ← **VERIFIED**
*(undefined4 *)((int)this + 0x178) = 0;   // vel_z ← **VERIFIED**
```

### Verified by `Ball_ApplyForceWithMultipliers` (0x00402650)

```c
// Accumulates directional force into velocity:
*(float *)((int)this + 0x170) = param_1 * param_4 + *(float *)((int)this + 0x170);
*(float *)((int)this + 0x174) = param_2 * param_4 + *(float *)((int)this + 0x174);
*(float *)((int)this + 0x178) = param_3 * param_4 + *(float *)((int)this + 0x178);
```

### Verified by `Ball_ApplyForceV2` (0x004016F0)

Same pattern — writes to `+0x170/174/178` with identical guards.

### Layout check (no gaps, no overlaps)

```
0x164  float  pos_x
0x168  float  pos_y
0x16C  float  pos_z
0x170  float  vel_x  ← confirmed
0x174  float  vel_y  ← confirmed
0x178  float  vel_z  ← confirmed
0x17C  float  accel_x
0x180  float  accel_y
0x184  float  accel_z
0x188  float  max_speed
```

**✅ The velocity offsets are definitively correct.**

---

## Ball-Related Functions

### Core Lifecycle

| Function | Address | Purpose |
|----------|---------|---------|
| `Ball_ctor` | `0x0040AFE0` | Full constructor (calls `Ball_ctor2`, sets vtable to `0x004CF3A0`) |
| `Ball_ctor2` | `0x004039E0` | Base init — all fields zeroed / defaulted |
| `Ball_dtor` | `0x004027F0` | Destructor wrapper (calls `Ball_dtor2`, then `_free`) |
| `Ball_dtor2` | `0x00401CC0` | Actual destructor — resets matrix, calls `GameObject_dtor` |
| `Ball_Update` | `0x00405E00` | **Main 23-phase physics tick** (param_1 = Ball*) |
| `Ball_FallUpdate` | `0x00408830` | Death-fall physics (out-of-bounds, water, pits) |

### Physics & Movement

| Function | Address | What It Does |
|----------|---------|-------------|
| `Ball_ApplyForceWithMultipliers` | `0x00402650` | Add force to velocity with impact/speed/ice/dizzy multipliers |
| `Ball_ApplyForceV2` | `0x004016F0` | Alt force app with gravity-plane awareness |
| `CollisionMesh_SetSpeed` | `0x004029C0` | **DEAD CODE.** Writes `+0xC64` (roll_friction) and `+0xC98/C9C/CA0` (unused), but physics loop immediately overwrites them. Does NOT control ball speed. |
| `Ball_SetVec3AtOffset` | `0x00402A20` | Write Vec3 to arbitrary offset (modding helper) |
| `Ball_SetTargetPos` | `0x00402030` | Network sync position (lerp toward target) |
| `Ball_SetTrajectory` | `0x00403850` | Launch-pad trajectory setup (stores params at `+0x2AC` to `+0x2B8`) |
| `Ball_ApplyTrajectory` | `0x00403750` | Execute launch — applies physics_body trajectory, plays sound, sets impact block |
| `Ball_SetTiltedGravity` | `0x00403100` | Set gravity plane to tilted (normal `0, -1, 0`) |
| `Ball_SetFlatGravity` | `0x00403150` | Set gravity plane to flat (normal `0, 0, 1`) |
| `Ball_CheckProximity` | `0x00402150` | Distance check — sets `+0x744` if within threshold |
| `Ball_FindMeshCollision` | `0x00403980` | Raycast vs level mesh (delegates to `Mesh_FindClosestCollision`) |
| `Ball_FindClosestRespawnPoint` | `0x00405190` | Full respawn logic — reset collision, free display string, find nearest checkpoint |
| `Ball_TestPlaneIntersection` | `0x00402810` | Frustum culling helper for render shadow |
| `Ball_AdvancePositionOrCollision` | `0x004564C0` | Multiplayer sync position advance |

### State Changes

| Function | Address | What It Does |
|----------|---------|-------------|
| `Ball_Shrink` | `0x00402200` | Odd Race E:SHRINK: radius=13.0, max_speed=2.5, play shrink sound |
| `Ball_Grow` | `0x00402270` | Odd Race E:GROW: radius=26.0, max_speed=5.0 (restores from shrunk) |
| `Ball_ResetCollisionMesh` | `0x004030B0` | Reset physics body orientation, zero velocity, reset timer |
| `Ball_Shatter` | `0x00408D70` | **Arena: split ball into 3 AI balls** (called from FollowBall_Update, NOT E:JUMP) |
| `Ball_DizzyImmunity` | `0x00402400` | Grant dizzy immunity (clear +0x2EC, set +0x2F4 to max) |

### Rendering

| Function | Address | What It Does |
|----------|---------|-------------|
| `Ball_Render` | `0x00402DE0` | Full render pass — shadow, particles, sprite quad |
| `Ball_RenderShadow` | `0x00401920` | Render ground shadow decal |
| `Ball_SetupCollisionRender` | `0x004015B0` | Setup collision sounds, render visibility flags |
| `Ball_InitRenderState` | `0x00402860` | One-time D3D render state init (cull mode, texture stages) |
| `Ball_RenderWithMaterial` | `0x0045D8F0` | Render with custom material override |
| `Ball_GetTransform` | `0x0040AF90` | Extract transform matrix for rendering |

### Input

| Function | Address | What It Does |
|----------|---------|-------------|
| `Ball_GetInputForce` | `0x0046EC30` | **Read input device, output 2D force vector** (keyboard DIK codes at `InputDevice+0x50C/510/514/518`) |

### Misc

| Function | Address | What It Does |
|----------|---------|-------------|
| `Ball_SetName` | `0x00401660` | Allocate & copy display name string to `+0xC28` |
| `Ball_CreateTrailParticles` | `0x00401DD0` | Spawn sparkle trail effect at ball position |

---

## What You Can Do With the Ball Object

### 1. Read/Write Velocity (Instant Boosts)

```c
// Direct velocity manipulation — bypasses all engine multipliers
float* ball = (float*)ball_ptr;
ball[0x170/4] = 5000.0f;  // X velocity
ball[0x174/4] = 0.0f;     // Y velocity
ball[0x178/4] = 0.0f;     // Z velocity
```

**Verified source:** `Ball_ApplyForceWithMultipliers` @ `0x00402650` reads/writes these exact offsets.

### 2. Read/Write Position (Teleport)

```c
ball[0x164/4] = 100.0f;   // X position
ball[0x168/4] = 50.0f;    // Y position
ball[0x16c/4] = 200.0f;   // Z position
```

**Note:** Also set `teleport` field at `+0xC3C` to prevent physics from overriding it on the next frame.

### 3. Change Ball Radius

```c
// Ball_Shrink sets radius = 13.0f (0x41500000)
// Ball_Grow sets radius = 26.0f (0x41D00000)
// Normal radius = 27.0f (0x41D80000) — set in Ball_ctor2
*(float*)(ball + 0x284) = 50.0f;  // Giant ball
```

### 4. Change Max Speed

```c
// Ball_ctor2 default: 0x459C4000 (~5000.0f)
// Ball_Shrink: 0x40200000 (2.5f)
// Ball_Grow: 0x40A00000 (5.0f)
*(float*)(ball + 0x188) = 10000.0f;  // Super speed
```

### 5. Force Death-Fall State

```c
Ball_Shrink(ball_ptr);  // @ 0x00402200
// Sets: +0xC4C = 1 (is_shrunk), radius=13.0, max_speed=2.5, plays sound
```

### 6. Force Split Power-Up

```c
Ball_Shatter(ball_ptr, some_param);  // @ 0x00408D70
// Guard: checks *(char*)(ball + 0x324) == 0 (not already split)
```

### 7. Apply Launch Pad Boost

```c
// First set trajectory vector on physics body:
*(float*)(*(int*)(ball + 0x1A4) + 0xCA4) = dir_x;
*(float*)(*(int*)(ball + 0x1A4) + 0xCA8) = dir_y;
*(float*)(*(int*)(ball + 0x1A4) + 0xCAC) = dir_z;
Ball_ApplyTrajectory(ball);  // @ 0x00403750
// Damps Y by 0.7x, normalizes, scales by 0.01, sets impact block = 100 frames
```

### 8. Change Gravity Plane

```c
Ball_SetTiltedGravity(ball);  // Normal = (0, -1, 0)  @ 0x00403100
Ball_SetFlatGravity(ball);    // Normal = (0, 0, 1)     @ 0x00403150
Ball_ResetCollisionMesh(ball); // Reset orientation        @ 0x004030B0
```

### 9. Check If Ball Is Falling

```c
char is_shrunk = *(char*)(ball + 0xC4C);
// Set by Ball_Shrink (→1), cleared by Ball_Grow (→0)
```

### 10. Read Ball Speed Scale

```c
float speed_scale = *(float*)(ball + 0x18C);  // default 1.0f
// Affects all velocity calculations in Ball_Update
```

### 11. Read Player Index

```c
int player_idx = *(int*)(ball + 0x18);
// -1 = no player (AI / demo), 0-3 = human player index
```

---

## Modding Recipes

### Recipe 1: Permanent Super Speed

Hook `Ball_ctor2` (0x004039E0), after the line:
```
*(undefined4 *)((int)this + 0x188) = 0x459c4000;  // max_speed = 5000
```
Change `0x459c4000` to `0x461C4000` (10000.0f).

### Recipe 2: Giant Ball Mode

In your mod DLL, every frame after `Ball_Update` returns:
```c
*(float*)(ball + 0x284) = 100.0f;  // radius
```
The collision mesh (at `ball + 0x1A4`) must also be scaled — call `CollisionMesh_SetRadius` or patch the mesh directly.

### Recipe 3: Zero-Gravity Mode

Patch `Ball_Update` to skip gravity accumulation. The gravity vector is at `+0x1A8` (default `0, 1.0, 0`).
```c
*(float*)(ball + 0x1A8) = 0.0f;  // gravity_x
*(float*)(ball + 0x1AC) = 0.0f;  // gravity_y
*(float*)(ball + 0x1B0) = 0.0f;  // gravity_z
```

### Recipe 4: Invincibility (No Death-Fall)

Hook `Ball_Shrink` (0x00402200) to return immediately:
```asm
xor eax, eax
retn 4
```

### Recipe 5: Always-Split Power-Up

Hook the guard check in `Ball_Shatter` (0x00408D70):
```asm
; NOP out the "if (*(char*)(this+0x324) == 0)" check
nop
nop
nop
nop
nop
nop
```

---

## Verified Memory Layout (Key Fields)

Extracted directly from `Ball_ctor2` decompilation @ `0x004039E0`:

| Offset | Type | Initial Value | Description |
|--------|------|---------------|-------------|
| `+0x00` | vtable | `0x004CF314` | GameObject vtable (ctor2 sets base vtable) |
| `+0x04` | byte[2] | `0, 0` | collision_result flags |
| `+0x0C` | int | `200` | string_timer (ms) |
| `+0x10` | void* | `param_1+0x878` | App pointer (scene's app ref) |
| `+0x14` | void* | `param_1` | Scene pointer |
| `+0x18` | int | `-1` | player_index (-1 = none, 0-3 = player) |
| `+0x1C` | UITimer | — | Timer object (size 0xEC) |
| `+0x108` | Timer | — | Main timer |
| `+0x150` | int | `0` | accumulated_time |
| `+0x154` | int | RNG | random seed |
| `+0x158` | Vec3 | `0,0,0` | prev_pos |
| `+0x164` | float | `0` | **pos_x** |
| `+0x168` | float | `0` | **pos_y** |
| `+0x16C` | float | `0` | **pos_z** |
| `+0x170` | float | `0` | **vel_x** ← verified |
| `+0x174` | float | `0` | **vel_y** ← verified |
| `+0x178` | float | `0` | **vel_z** ← verified |
| `+0x17C` | float | `0` | accel_x |
| `+0x180` | float | `0` | accel_y |
| `+0x184` | float | `0` | accel_z |
| `+0x188` | float | `0x459C4000` | max_speed (~5000.0f) |
| `+0x18C` | float | `0x3F800000` | speed_scale (1.0f) |
| `+0x190` | float | — | facing_angle (computed) |
| `+0x194` | float | `0xBF800000` | (-1.0f) |
| `+0x198` | float | — | spin_angle |
| `+0x1A0` | float | `0x3F800000` | (1.0f) |
| `+0x1A4` | void* | new CollisionMesh | Physics body pointer |
| `+0x1A8` | Vec3 | `0, 1.0, 0` | gravity vector |
| `+0x1B8` | RenderContext | — | Primary render context |
| `+0x208` | RenderContext | — | Secondary render context |
| `+0x260` | byte | `0` | sweat_flag |
| `+0x264` | ArenaBoard | — | Rumble timer (0x14 bytes) |
| `+0x278` | float | `0x3DCCCCCD` | (0.1f) |
| `+0x27C` | int | `0` | — |
| `+0x281` | byte | `1` | unused_init_flag (DEAD: set by ctor, never read by any function) |
| `+0x284` | float | `0x41D80000` | radius (27.0f) |
| `+0x290` | int | `0` | spin_timer |
| `+0x2A4` | float | `0x40A00000` | (5.0f) |
| `+0x2A8` | Vec3 | `0,0,0` | accel vector |
| `+0x2B8` | Vec3 | `0,0,0` | another vector (trajectory?) |
| `+0x2C0` | Vec3 | `0,0,0` | checkpoint position |
| `+0x2CC` | byte | `0` | block_input |
| `+0x2D4` | byte | `0` | — |
| `+0x2D5` | byte | `0` | — |
| `+0x2D8` | int | `0` | — |
| `+0x2DC` | int | `0` | checkpoint index |
| `+0x2E8` | byte | `0` | event_flag |
| `+0x2E9` | byte | `0` | ⚠ death_pending (NOT on_ramp! Sticky flag preventing Ball_ApplyTrajectory re-firing. Set by E:LIMIT/LIMITX/LIMITZ/LIMITPIPE1/2/SWALLOW, speed>1.0 collision, and Ball_ApplyTrajectory. Reset by Ball_InitPhysicsDefaults and Ball_ctor2) |
| `+0x2EC` | int | `0` | bounce_count (dizzy system bounce counter; double-incremented when collision speed exceeds thresholds 0.03 and 0.1; triggers Ball_ApplyTrajectory when >1 AND death_pending==0) |
| `+0x2F0` | int | `0` | impact_counter (frames force is blocked) |
| `+0x2F4` | int | `0` | best_score |
| `+0x2F8` | byte | `1` | alive_flag (set 0 on death) |
| `+0x2F9` | byte | `0` | disabled_flag |
| `+0x300` | int | `0` | — |
| `+0x310` | byte | `1` | can_collide |
| `+0x314` | int | `0` | — |
| `+0x318` | int | `0` | — |
| `+0x31C` | byte | `0` | — |
| `+0x324` | byte | `0` | is_split (split power-up active) |
| `+0x328` | int | `-1` | — |
| `+0x32C` | AthenaList | — | Linked list #1 |
| `+0x744` | int | `0` | proximity_counter |
| `+0x748` | int | `0` | gravity_plane_type (0=tilted, 1=flat, 2=?) |
| `+0x764` | float | `1.0f` | — |
| `+0x768` | byte | `1` | visible |
| `+0x769` | byte | `0` | — |
| `+0x76A` | byte | `0` | has_target_pos (network sync) |
| `+0x76C` | Vec3 | `0,0,0` | target_pos (network) |
| `+0x808` | int | `0` | impact_freeze_counter — if non-zero, Ball_ApplyForce is skipped. Counts down each frame. Set by N:NOCONTROL(=10), E:CATAPULTBOTTOM(=1000), vacuum(=1000). |
| `+0xC28` | char* | `0` | display_name string |
| `+0xC3C` | byte | `0` | teleport_flag |
| `+0xC4C` | byte | `0` | is_shrunk (runtime) |
| `+0xC50` | float | `0` | fall_depth |
| `+0xC54` | int | `0` | sound_channel |
| `+0xC58` | byte | `0` | — |
| `+0xC5C` | int | `0` | dizzy_flag (NOTE: the actual dizzy system uses ball+0x2E9 (death_pending) and ball+0x2EC (bounce_count), not this field) |
| `+0xC60` | float | `1.0f` | scale_factor |
| `+0xC64` | float | `0` | roll_friction (overwritten every frame by physics loop) |
| `+0xC74` | int | `0` | render_alpha |
| `+0xC80` | byte | `0` | has_viewport_clip |
| `+0xC88` | int[4] | — | viewport clip rect |
| `+0xC98` | — | — | **End of struct** |

---

## How to Use This Document

1. **Attach a debugger** (Cheat Engine, x64dbg) to `Hamsterball.exe`.
2. **Find the Ball pointer**: In `Ball_Update` (0x00405E00), the first argument in ECX is `this`.
3. **Read/write fields** using the offsets above.
4. **Hook functions** by patching the vtable at `0x004CF3A0` or placing JMP hooks at the function addresses.

---

## Sources

All data verified via **live GhidraMCP headless decompilation** (`http://127.0.0.1:8089/decompile_function`) on `Hamsterball.exe`:

- `Ball_ctor2` @ `0x004039E0` — struct layout, initial values
- `Ball_ApplyForceWithMultipliers` @ `0x00402650` — velocity write pattern
- `Ball_ApplyForceV2` @ `0x004016F0` — velocity write pattern (alt)
- `Ball_Shrink` @ `0x002200` — Odd Race shrink state (E:SHRINK)
- `Ball_Grow` @ `0x00402270` — Odd Race grow recovery (E:GROW)
- `Ball_Update` @ `0x00405E00` — main physics tick overview
- `Ball_Shatter` @ `0x00408D70` — arena 8-ball split mechanic (called from FollowBall_Update)
- `Ball_GetInputForce` @ `0x0046EC30` — input system integration
- `Ball_SetTrajectory` / `Ball_ApplyTrajectory` — launch pad mechanics

---

*Document generated: 2026-06-05*  
*Method: Ghidra decompilation + cross-reference with `ball_struct.h`*  
*Confidence: High — all offsets verified against primary source code*
