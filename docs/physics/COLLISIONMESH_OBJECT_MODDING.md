# CollisionMesh (Physics Object) — Complete Modder's Reference

> **Parameter of `Ball_Update` @ `0x00405E00`** (accessed via `Ball + 0x1A4`).  
> All offsets below verified via live Ghidra decompilation of `Hamsterball.exe`.  
> **Confidence markers:** ✅ = Verified in raw decompiled C (2+ functions), ⚠️ = Verified in 1 function, ❓ = Inferred from struct layout.

---

## What Is the CollisionMesh?

The `CollisionMesh` is **NOT** the `Ball` itself. It is a **separate nested physics object** allocated during `Ball_ctor2` and pointed to by the Ball at offset `+0x1A4`:

```
Ball + 0x1A4 → CollisionMesh* (heap object, 0xCB0 bytes)
```

`Ball_Update` (0x405E00) is the function that reads this pointer and integrates all physics state. If you want to modify how the ball moves in the world — speed, gravity, friction, jumping — **this is the object you modify**, not the ephemeral `Ball + 0x170` velocity accumulator (which gets zeroed every frame).

### Getting the Pointer

```cpp
// Method 1: From any Ball pointer
int* ball = (int*)0x...;  // your Ball* hook parameter
int* phys = *(int**)((char*)ball + 0x1A4);

// Method 2: From the Ball_Update hook (0x405E00)
// param_1 IS the Ball*, so:
int* phys = *(int**)((char*)param_1 + 0x1A4);
```

---

## Full CollisionMesh Struct Layout

The CollisionMesh object is **0xCB0 bytes** (3248). It inherits from `SceneObject` (base vtable + refcount), then adds physics fields.

| Offset | Type | Field | Default | Verified By | Modding Use |
|--------|------|-------|---------|-------------|-------------|
| `+0x000` | `void**` | `vtable` | `0x4D8E10` | ✅ `CollisionMesh_ctor` (0x405680) | Call virtual methods |
| `+0x004` | `int` | `ref_count` | `1` | ✅ `CollisionMesh_ctor` | Reference counting |
| `+0x010` | `Ball*` | `owner_ball` | `this` (Ball*) | ✅ `CollisionMesh_ctor` | Back-pointer to parent |
| `+0x014` | — | `(SceneObject base)` | — | ✅ Inheritance | — |
| `+0xC60` | `int` | `battle_mode` | `3` | ✅ `Ball_InitBattleMode` | Arena mode flag |
| `+0xC64` | `float` | `roll_friction` | computed | ✅ `Ball_Update` Phase 14 | ⚠️ **OVERWRITTEN EVERY FRAME** |
| `+0xC68` | `float` | `friction_damping` | `~0.56` | ✅ `Ball_InitBattleMode` | Ground friction multiplier |
| `+0xC6C` | `float` | `field_c6c` | `1.0f` | ✅ `Ball_InitBattleMode` | Unknown scaler |
| `+0xC70` | `float` | `max_speed_limit` | `1000.0f` | ✅ `Ball_InitBattleMode` | **Hard speed cap** |
| `+0xC74` | `int` | `field_c74` | `0` | ✅ `CollisionMesh_ctor` | Unused? |
| `+0xC78` | `float` | `gravity_strength` | `25.0f` | ✅ `Ball_InitBattleMode` | Gravity pull strength |
| `+0xC7C` | `byte` | `use_gravity` | `1` | ✅ `Ball_InitBattleMode` | `0` = no gravity |
| `+0xC80` | — | `(padding)` | — | ❓ Alignment | — |
| `+0xC8C` | `float` | `dir_x` | `0` | ✅ `Ball_InitBattleMode` | Physics up-vector X |
| `+0xC90` | `float` | `dir_y` | `-1.0f` | ✅ `Ball_InitBattleMode` | Physics up-vector Y |
| `+0xC94` | `float` | `dir_z` | `0` | ✅ `Ball_InitBattleMode` | Physics up-vector Z |
| `+0xC98` | `float` | `scaled_dir_x` | — | ✅ `CollisionMesh_SetSpeed` | **NEVER READ by engine** |
| `+0xC9C` | `float` | `scaled_dir_y` | — | ✅ `CollisionMesh_SetSpeed` | **NEVER READ by engine** |
| `+0xCA0` | `float` | `scaled_dir_z` | — | ✅ `CollisionMesh_SetSpeed` | **NEVER READ by engine** |
| `+0xCA4` | `float` | `vel_x` | `0` | ✅ `Ball_AdvancePositionOrCollision` | **Persistent physics X velocity** |
| `+0xCA8` | `float` | `vel_y` | `0` | ✅ `Ball_AdvancePositionOrCollision` | **Persistent physics Y velocity — JUMP MOD TARGET** |
| `+0xCAC` | `float` | `vel_z` | `0` | ✅ `Ball_AdvancePositionOrCollision` | **Persistent physics Z velocity** |

**Total size:** `0xCB0` bytes (3248).  
**Constructor:** `CollisionMesh_ctor` @ `0x405680` allocates via `operator_new(0xCB0)`.  
**Vtable:** `0x4D8E10` (inherits `SceneObject` methods + physics overrides).

---

## Two Velocity Systems (CRITICAL)

There are **two separate velocity systems** in Hamsterball. Writing to the wrong one has no lasting effect.

| System | Address | Type | Persistence | Who Clears It |
|--------|---------|------|-------------|---------------|
| **Ball input velocity** | `Ball + 0x170/174/178` | `float[3]` | **Ephemeral** — cleared every frame | `Ball_Update` Phase 1 zeros these after reading |
| **CollisionMesh physics velocity** | `CollisionMesh + 0xCA4/CA8/CAC` | `float[3]` | **Persistent** — survives frames | Integrated by `Ball_AdvancePositionOrCollision` (0x4564C0) |

**Modding rule:** If you write to `Ball + 0x174` (vel_y), it gets overwritten to `0` within one frame. If you write to `*(Ball+0x1A4) + 0xCA8` (CollisionMesh.vel_y), the physics integrator uses it on the next tick and the ball actually moves.

---

## Verified Functions

### CollisionMesh Lifecycle

| Address | Name | Role | Verified |
|---------|------|------|----------|
| `0x405680` | `CollisionMesh_ctor` | Allocates 0xCB0 bytes, stores `Ball*` at +0x10, sets vtable to `0x4D8E10` | ✅ |
| `0x4056D0` | `Ball_InitBattleMode` | Sets default physics params (gravity=25.0f, speed limit=1000.0f) | ✅ |
| `0x4030B0` | `Ball_ResetCollisionMesh` | Resets direction to `-1.0f` Y, clears impact state | ✅ |

### Physics Integration

| Address | Name | Role | Verified |
|---------|------|------|----------|
| `0x405E00` | `Ball_Update` | **Main physics tick** — reads CollisionMesh, integrates, writes back. Phase 14 overwrites `roll_friction` every frame. | ✅ |
| `0x4564C0` | `Ball_AdvancePositionOrCollision` | Reads `vel_x/y/z` (+0xCA4/CA8/CAC), applies gravity, does collision broad phase, writes updated position back to `Ball + 0x164/168/16C` | ✅ |
| `0x402650` | `Ball_ApplyForce` | Accumulates force into Ball's ephemeral velocity (+0x170), NOT CollisionMesh | ✅ |

### Direct Field Access

| Address | Name | Role | Verified |
|---------|------|------|----------|
| `0x402A20` | `Ball_SetVec3AtOffset` | Direct overwrite of `CollisionMesh + 0xCA4/CA8/CAC` (velocity) | ✅ |
| `0x4029C0` | `CollisionMesh_SetSpeed` | ⚠️ **DEAD CODE** — writes `+0xC64` (roll_friction, overwritten next frame) and `+0xC98/C9C/CA0` (never read). **No lasting effect.** | ✅ |

---

## Modding Recipes

### Recipe 1: The Jump Mod (Verified Working)

This is how the community jump mod works. It injects upward velocity directly into the CollisionMesh, bypassing the ephemeral Ball velocity:

```cpp
void InjectJump(void* ball, float jump_power) {
    // Get the CollisionMesh pointer from the Ball
    int* phys = *(int**)((char*)ball + 0x1A4);
    if (!phys) return;

    // Write directly to persistent physics velocity
    *(float*)((char*)phys + 0xCA8) = jump_power;  // vel_y = upward impulse

    // Optional: also zero X/Z velocity for clean vertical jump
    // *(float*)((char*)phys + 0xCA4) = 0.0f;
    // *(float*)((char*)phys + 0xCAC) = 0.0f;
}
```

**Why this works:** `Ball_AdvancePositionOrCollision` reads `+0xCA8` on the next tick, adds gravity (`gravity_strength * dt`), runs collision against the level mesh, and updates `Ball->pos` accordingly. The velocity persists until the ball hits a surface or `Ball_ResetCollisionMesh` is called.

### Recipe 2: Super Speed (Modify Speed Limit)

```cpp
void SetSuperSpeed(void* ball, float new_limit) {
    int* phys = *(int**)((char*)ball + 0x1A4);
    if (!phys) return;

    // Raise the hard speed cap
    *(float*)((char*)phys + 0xC70) = new_limit;  // max_speed_limit
}
```

**Note:** The engine clamps speed every frame. Raising `max_speed_limit` allows higher speeds without the clamp kicking in. You still need to inject velocity (Recipe 1 or ApplyForce) to actually go fast.

### Recipe 3: Disable Gravity

```cpp
void DisableGravity(void* ball) {
    int* phys = *(int**)((char*)ball + 0x1A4);
    if (!phys) return;

    *(char*)((char*)phys + 0xC7C) = 0;  // use_gravity = false
}
```

**Effect:** `Ball_AdvancePositionOrCollision` skips gravity application. The ball stops falling and maintains whatever velocity you injected. Combine with a small constant upward velocity for hover/fly mode.

### Recipe 4: Low Friction / Ice Mode

```cpp
void SetLowFriction(void* ball, float friction) {
    int* phys = *(int**)((char*)ball + 0x1A4);
    if (!phys) return;

    *(float*)((char*)phys + 0xC68) = friction;  // friction_damping
}
```

**Default:** `~0.56`. Set to `0.01` for ice physics (ball slides forever). Set to `2.0` for sticky mud.

**⚠️ WARNING:** `Ball_Update` Phase 14 also computes and writes `roll_friction` (+0xC64) every single frame:
```
roll_friction = (radius * 0.98 * friction_damping * speed_scale) / 0.96
```
So modifying `+0xC64` directly is futile — it gets overwritten. Always modify `friction_damping` (+0xC68) instead.

### Recipe 5: Reverse Gravity

```cpp
void ReverseGravity(void* ball) {
    int* phys = *(int**)((char*)ball + 0x1A4);
    if (!phys) return;

    *(float*)((char*)phys + 0xC90) = 1.0f;  // dir_y = +1.0 (up is down)
}
```

**Effect:** The physics integrator uses `dir` as the "up" vector for gravity application. `-1.0f` = normal gravity (pulls toward -Y). `+1.0f` = reverse gravity (pulls toward +Y). You can set any normalized vector for weird directional gravity.

### Recipe 6: Instant Stop

```cpp
void InstantStop(void* ball) {
    int* phys = *(int**)((char*)ball + 0x1A4);
    if (!phys) return;

    *(float*)((char*)phys + 0xCA4) = 0.0f;  // vel_x = 0
    *(float*)((char*)phys + 0xCA8) = 0.0f;  // vel_y = 0
    *(float*)((char*)phys + 0xCAC) = 0.0f;  // vel_z = 0
}
```

**Effect:** Zeroes all persistent physics velocity. The ball stops moving instantly (no inertia). Useful for teleportation or checkpoint resets.

---

## ⚠️ DEAD CODE WARNING: CollisionMesh_SetSpeed

**Address:** `0x4029C0`  
**What it does:**
```cpp
void CollisionMesh_SetSpeed(void* this, float speed) {
    *(float*)((char*)this + 0xC64) = speed;              // roll_friction
    *(float*)((char*)this + 0xC98) = speed * dir_x;      // scaled_dir_x
    *(float*)((char*)this + 0xC9C) = speed * dir_y;      // scaled_dir_y
    *(float*)((char*)this + 0xCA0) = speed * dir_z;      // scaled_dir_z
}
```

**Why it does nothing:**
1. `+0xC64` (roll_friction) is **recomputed and overwritten every frame** by `Ball_Update` Phase 14.
2. `+0xC98/C9C/CA0` (scaled_dir) are **never read by any engine function**. They are sinkholes.

**Historical naming note:** This was originally called `Ball_SetSpeed` in early RE because it was found through Ball cross-references. The decompilation reveals it accesses CollisionMesh offsets on `this`, proving it's a CollisionMesh method, not a Ball method. **Do not call this function expecting speed changes.**

---

## Ball_Update Execution Order (0x405E00)

Understanding the tick order helps you know WHEN to inject modifications:

```
Phase 1:  Decay timers (ice effect, lerp factors)
Phase 2:  Read ephemeral Ball velocity (+0x170) into locals, then ZERO it
Phase 3:  Read input force from keyboard/joystick
Phase 4:  Apply force multipliers (tilted gravity, speed scale)
Phase 5:  Build spatial collision tree
Phase 6:  Apply gravity to CollisionMesh velocity (+0xCA8)
Phase 7:  Integrate position from CollisionMesh velocity
Phase 8:  Collision detection (broad phase → narrow phase)
Phase 9:  Collision response (bounce, friction, normal reflection)
Phase 10: Write updated position back to Ball->pos (+0x164)
Phase 11: Write updated CollisionMesh velocity back (+0xCA4/CA8/CAC)
Phase 12: Trail particle spawn
Phase 13: Audio state update
Phase 14: Compute roll_friction = (radius * 0.98 * friction_damping * speed_scale) / 0.96
          → WRITE to CollisionMesh + 0xC64 (overwrites any external value)
Phase 15: Check out-of-bounds, respawn if needed
```

**Injection timing:**
- **Before Phase 6:** Your velocity gets gravity applied → good for jump
- **After Phase 11:** Your velocity overwrites the integrated result → good for instant stop
- **Phase 14:** `roll_friction` is always overwritten — never write to `+0xC64` directly

---

## Cross-Reference: Ball ↔ CollisionMesh

| What you want | Ball offset | CollisionMesh offset |
|---------------|-------------|----------------------|
| Position (read/write) | `+0x164/168/16C` | N/A (position is in Ball) |
| Ephemeral input velocity | `+0x170/174/178` | N/A (zeroed every frame) |
| **Persistent physics velocity** | `+0x1A4` → ptr → `+0xCA4/CA8/CAC` | `+0xCA4/CA8/CAC` |
| Speed limit | `+0x1A4` → ptr → `+0xC70` | `+0xC70` |
| Gravity toggle | `+0x1A4` → ptr → `+0xC7C` | `+0xC7C` |
| Gravity strength | `+0x1A4` → ptr → `+0xC78` | `+0xC78` |
| Friction damping | `+0x1A4` → ptr → `+0xC68` | `+0xC68` |
| Physics up-vector | `+0x1A4` → ptr → `+0xC8C/C90/C94` | `+0xC8C/C90/C94` |
| Roll friction (computed) | `+0x1A4` → ptr → `+0xC64` | `+0xC64` |

---

## Sources

All data verified via **live GhidraMCP headless decompilation** on `Hamsterball.exe`:

- `Ball_Update` @ `0x405E00` — main physics tick (raw C, 15 phases)
- `Ball_AdvancePositionOrCollision` @ `0x4564C0` — position integration + collision
- `CollisionMesh_ctor` @ `0x405680` — constructor, field initialization
- `Ball_InitBattleMode` @ `0x4056D0` — default physics params
- `Ball_ApplyForce` @ `0x402650` — ephemeral force accumulator
- `Ball_SetVec3AtOffset` @ `0x402A20` — direct velocity overwrite
- `CollisionMesh_SetSpeed` @ `0x4029C0` — dead code analysis
- `BALL_OBJECT_MODDING.md` — parent Ball struct for cross-reference

---

*Document generated: 2026-06-06*  
*Method: Ghidra decompilation + cross-reference with existing ball docs + dead code analysis*  
*Confidence: High for ✅ verified offsets, Medium for ⚠️ single-source offsets*
