# CollisionMesh Object — Complete Modder's Reference

> **Analysis of the `playerObject + 0x1A4` pointer chain used by the jump mod and other physics hacks.**  
> All offsets verified via live Ghidra MCP decompilation of `Hamsterball.exe`.  
> This document describes the `CollisionMesh` object — a nested physics body that lives inside the Ball.

---

## Quick Stats

| Property | Value |
|----------|-------|
| Primary update function | `Ball_AdvancePositionOrCollision` @ `0x00405640` |
| Constructor | `CollisionMesh_ctor` @ `0x00405680` |
| Total struct size | `0x0CB0` bytes (3,248 bytes) |
| Parent backref | `Ball*` stored at `+0x0010` |
| VTable | `0x004D8E10` (`Mesh_DeletingDtor`) |
| Pointer in Ball | `Ball + 0x1A4` stores `CollisionMesh*` |

---

## The Pointer Chain Explained

```cpp
// The jump mod follows this exact chain:
// 1. Ball + 0x1A4  →  CollisionMesh* (pointer field inside Ball)
// 2. *(Ball + 0x1A4) + 0xCA8  →  physics velocity Y (float)

DWORD* physicsObjPtr = (DWORD*)((DWORD)playerObject + 0x1a4);  // CollisionMesh**
DWORD physicsObj = *physicsObjPtr;                              // CollisionMesh*
float* trueVelY = (float*)(physicsObj + 0xca8);                 // &collisionMesh->vel_y
```

The Ball struct ends at `~0xC98`. The `CollisionMesh` object it points to extends **beyond** the Ball's own size with its own fields up to `+0xCA8` and beyond. **These are NOT Ball fields** — they belong to the nested object.

---

## How CollisionMesh is Created

From `Ball_ctor2` (`0x004039E0`) decompilation:

```c
pvVar2 = operator_new(0xcb0);               // Allocate 3,248 bytes
pvVar2 = CollisionMesh_ctor(pvVar2, this);    // Initialize, pass Ball as owner
*(void **)((int)this + 0x1a4) = pvVar2;      // Store pointer in Ball struct
```

`CollisionMesh` is a **completely separate object** with its own vtable (`0x4D8E10`), its own constructor (`0x405680`), and its own destructor. It lives on the heap and is pointed to by `Ball + 0x1A4`.

---

## Why Modders Target CollisionMesh Instead of Ball Velocity

### System A: Ball Input Velocity (NOT the target)

| Field | Address | Type | Behaviour |
|-------|---------|------|-----------|
| `vel_x` | `Ball + 0x0170` | `float` | Input force accumulator |
| `vel_y` | `Ball + 0x0174` | `float` | Input force accumulator |
| `vel_z` | `Ball + 0x0178` | `float` | Input force accumulator |

- Written by `Ball_ApplyForceWithMultipliers` (`0x402650`)
- Read by `Ball_Update` (`0x405E00`) then **cleared to zero every frame**
- **Writing here does nothing** — values are erased before physics integration

### System B: CollisionMesh Physics Velocity (THE target)

| Field | Address | Type | Behaviour |
|-------|---------|------|-----------|
| `vel_x` | `CollisionMesh + 0x0CA4` | `float` | Persistent physics state |
| `vel_y` | `CollisionMesh + 0x0CA8` | `float` | Persistent physics state |
| `vel_z` | `CollisionMesh + 0x0CAC` | `float` | Persistent physics state |

- Read/written by `Ball_Update` and `Ball_AdvancePositionOrCollision`
- **Survives across frames** — integrated by the physics engine
- The jump mod writes `20.0f` to `+0xCA8` to inject an upward impulse

---

## Complete CollisionMesh Field Map

### Identity / Header

| Offset | Hex | Type | Name | Initial | Notes |
|--------|-----|------|------|---------|-------|
| `+0x0000` | `0x0000` | `uint32_t` | `vtable` | `0x4D8E10` | `Mesh_DeletingDtor` |
| `+0x0004` | `0x0004` | `uint32_t` | `field_04` | `0` | — |
| `+0x0008` | `0x0008` | `uint32_t` | `field_08` | `0` | — |
| `+0x000C` | `0x000C` | `uint32_t` | `field_0C` | `0` | — |
| `+0x0010` | `0x0010` | `Ball*` | `owner_ball` | `this` | Back-pointer to parent Ball |

### AthenaList #1 — Collision face list

| Offset | Hex | Type | Name | Notes |
|--------|-----|------|------|-------|
| `+0x0018` | `0x0018` | `uint32_t` | `list1_count` | `0` (from `AthenaList_Init`) |
| `+0x001C` | `0x001C` | `uint32_t` | `list1_capacity` | `0` |
| `+0x0020` | `0x0020` | `void**` | `list1_data` | Array of collision face pointers |
| `+0x0424` | `0x0424` | `void**` | `list1_end_ptr` | End of list data |

- **Total span:** `0x0018` to `~0x042F` (~1,044 bytes)
- Populated during collision detection with level geometry

### AthenaList #2 — Secondary collision data

| Offset | Hex | Type | Name | Notes |
|--------|-----|------|------|-------|
| `+0x0430` | `0x0430` | `uint32_t` | `list2_count` | `0` |
| `+0x0434` | `0x0434` | `uint32_t` | `list2_capacity` | `0` |
| `+0x0438` | `0x0438` | `void**` | `list2_data` | — |
| `+0x083C` | `0x083C` | `void**` | `list2_end_ptr` | — |

- **Total span:** `0x0430` to `~0x0847` (~1,047 bytes)

### AthenaList #3 — Tertiary collision data

| Offset | Hex | Type | Name | Notes |
|--------|-----|------|------|-------|
| `+0x0848` | `0x0848` | `uint32_t` | `list3_count` | `0` |
| `+0x084C` | `0x084C` | `uint32_t` | `list3_capacity` | `0` |
| `+0x0850` | `0x0850` | `void**` | `list3_data` | — |
| `+0x0C54` | `0x0C54` | `void**` | `list3_end_ptr` | — |

- **Total span:** `0x0848` to `~0x0C57` (~1,047 bytes)
- Freed and re-allocated during `Ball_AdvancePositionOrCollision`

### Physics Parameters

| Offset | Hex | Type | Name | Initial Value | Source | Modding Use |
|--------|-----|------|------|---------------|--------|-------------|
| `+0x0C60` | `0x0C60` | `int32_t` | `battle_mode` | `3` | `Ball_InitBattleMode` | Change game mode physics |
| `+0x0C64` | `0x0C64` | `float` | `speed_scalar` | `0.0f` | `Ball_SetSpeed` | Direct speed override |
| `+0x0C68` | `0x0C68` | `float` | `friction_damping` | `~0.56f` | `Ball_InitBattleMode` | Lower = more slippery |
| `+0x0C6C` | `0x0C6C` | `float` | `field_c6c` | `1.0f` | `Ball_InitBattleMode` | Unknown physics param |
| `+0x0C70` | `0x0C70` | `float` | `max_speed_limit` | `1000.0f` | `Ball_InitBattleMode` | Speed cap (raise for turbo) |
| `+0x0C74` | `0x0C74` | `uint32_t` | `field_c74` | `0` | `CollisionMesh_ctor` | — |
| `+0x0C78` | `0x0C78` | `float` | `gravity_strength` | `25.0f` | `Ball_InitBattleMode` | Lower = moon gravity |
| `+0x0C7C` | `0x0C7C` | `uint8_t` | `use_gravity` | `1` | `Ball_InitBattleMode` | Set to 0 = zero gravity |
| `+0x0C7D` | `0x0C7D` | `uint8_t` | `field_c7d` | — | — | Padding/unknown |
| `+0x0C7E` | `0x0C7E` | `uint16_t` | `field_c7e` | — | — | Padding/unknown |

### Direction Vector (Normalized Heading)

| Offset | Hex | Type | Name | Initial Value | Notes |
|--------|-----|------|------|---------------|-------|
| `+0x0C80` | `0x0C80` | `float` | `field_c80` | `0` | Unused / unknown |
| `+0x0C84` | `0x0C84` | `float` | `field_c84` | `0` | Unused / unknown |
| `+0x0C88` | `0x0C88` | `float` | `field_c88` | `0` | Unused / unknown |
| `+0x0C8C` | `0x0C8C` | `float` | `dir_x` | `0` | Normalized heading X |
| `+0x0C90` | `0x0C90` | `float` | `dir_y` | `-1.0f` | Default = straight down |
| `+0x0C94` | `0x0C94` | `float` | `dir_z` | `0` | Normalized heading Z |

- Direction is reset to `(0, -1, 0)` by `Ball_ResetCollisionMesh` (`0x4030B0`)
- `Ball_SetSpeed` multiplies `speed_scalar` by `dir` to produce `scaled_dir`

### Scaled Direction (speed * heading)

| Offset | Hex | Type | Name | Initial Value | Notes |
|--------|-----|------|------|---------------|-------|
| `+0x0C98` | `0x0C98` | `float` | `scaled_dir_x` | `0` | `speed_scalar * dir_x` |
| `+0x0C9C` | `0x0C9C` | `float` | `scaled_dir_y` | `0` | `speed_scalar * dir_y` |
| `+0x0CA0` | `0x0CA0` | `float` | `scaled_dir_z` | `0` | `speed_scalar * dir_z` |

- Written by `Ball_SetSpeed` (`0x4029C0`)
- Used for trajectory/heading calculations

### Physics Velocity (THE JUMP MOD TARGET)

| Offset | Hex | Type | Name | Initial Value | Used By |
|--------|-----|------|------|---------------|---------|
| `+0x0CA4` | `0x0CA4` | `float` | **`vel_x`** | `0` | `Ball_AdvancePositionOrCollision`, `Ball_Update` |
| `+0x0CA8` | `0x0CA8` | `float` | **`vel_y`** | `0` | ← **Jump mod writes here** |
| `+0x0CAC` | `0x0CAC` | `float` | **`vel_z`** | `0` | `Ball_AdvancePositionOrCollision`, `Ball_Update` |

- These are **the actual physics velocities** used for movement integration
- Not cleared each frame — they persist and accumulate gravity/collisions
- `Ball_Update` reads these, copies to local variables, runs physics, then writes back

### Post-Velocity Fields

| Offset | Hex | Type | Name | Notes |
|--------|-----|------|------|-------|
| `+0x0CB0` | `0x0CB0` | — | `struct_end` | End of CollisionMesh struct |

---

## Address Cheat Sheet

For quick reference when writing mods:

```cpp
// === Accessing the CollisionMesh from a Ball pointer ===
void* ball = playerObject;                          // Ball* (player object)
void** cmPtr = (void**)((DWORD)ball + 0x1A4);       // CollisionMesh** (pointer field)
void* cm = *cmPtr;                                   // CollisionMesh* (dereferenced)

// === Identity ===
uint32_t* cmVtable     = (uint32_t*)((DWORD)cm + 0x0000);
Ball**    cmOwner      = (Ball**)((DWORD)cm + 0x0010);

// === Physics Parameters ===
float* cmSpeedScalar   = (float*)((DWORD)cm + 0x0C64);
float* cmFriction      = (float*)((DWORD)cm + 0x0C68);
float* cmMaxSpeed      = (float*)((DWORD)cm + 0x0C70);
float* cmGravity       = (float*)((DWORD)cm + 0x0C78);
uint8_t* cmUseGravity  = (uint8_t*)((DWORD)cm + 0x0C7C);

// === Direction ===
float* cmDirX          = (float*)((DWORD)cm + 0x0C8C);
float* cmDirY          = (float*)((DWORD)cm + 0x0C90);
float* cmDirZ          = (float*)((DWORD)cm + 0x0C94);

// === Scaled Direction ===
float* cmScaledDirX    = (float*)((DWORD)cm + 0x0C98);
float* cmScaledDirY    = (float*)((DWORD)cm + 0x0C9C);
float* cmScaledDirZ    = (float*)((DWORD)cm + 0x0CA0);

// === PHYSICS VELOCITY (persistent) ===
float* cmVelX          = (float*)((DWORD)cm + 0x0CA4);
float* cmVelY          = (float*)((DWORD)cm + 0x0CA8);  // ← Jump mod target
float* cmVelZ          = (float*)((DWORD)cm + 0x0CAC);
```

---

## Key Functions

### CollisionMesh Lifecycle

| Function | Address | Description |
|----------|---------|-------------|
| `CollisionMesh_ctor` | `0x00405680` | Allocates `0xCB0` bytes, initializes lists, zeros velocity |
| `Mesh_DeletingDtor` | `0x004D8E10` (vtable) | Destructor — frees collision data |

### Ball Functions That Touch CollisionMesh

| Function | Address | What it does to CollisionMesh |
|----------|---------|------------------------------|
| `Ball_ctor2` | `0x004039E0` | Calls `operator_new(0xCB0)` → `CollisionMesh_ctor`, stores ptr at `+0x1A4` |
| `Ball_Update` | `0x00405E00` | Reads `cm.vel` into locals, integrates physics, writes back |
| `Ball_AdvancePositionOrCollision` | `0x00405640` | **Main physics integrator** — adds external velocity to `cm.vel`, applies gravity, collision response, friction |
| `Ball_InitBattleMode` | `0x004056D0` | Sets `cm.gravity_strength`, `cm.max_speed_limit`, `cm.friction`, `cm.battle_mode`, resets `cm.dir` to down |
| `Ball_SetSpeed` | `0x004029C0` | Sets `cm.speed_scalar`, updates `cm.scaled_dir` = speed * dir |
| `Ball_SetVec3AtOffset` | `0x00402A20` | Directly overwrites `cm.vel_x/y/z` with a Vec3 |
| `Ball_ResetCollisionMesh` | `0x004030B0` | Resets `cm.dir` to `(0, -1, 0)`, clears impact counter |
| `Ball_FindMeshCollision` | `0x00403980` | Passes `this` (CollisionMesh*) to `Mesh_FindClosestCollision` |

---

## The Ground Check Explained

```cpp
float tolerance = 0.5f;
if (*trueVelY > -tolerance && *trueVelY < tolerance) {
    *trueVelY = 20.0f;
}
```

The mod checks if `CollisionMesh.vel_y` is between `-0.5` and `+0.5`. This is a **grounded check**:

- **When resting on a surface:** Collision response from `Ball_AdvancePositionOrCollision` exactly cancels gravity → `vel_y ≈ 0`
- **When airborne:** `vel_y` is typically negative (falling due to gravity) or has significant magnitude
- **The tolerance `0.5f`** accounts for minor numerical jitter when the ball is "settled"

When the condition is met, the mod injects `20.0f` into `cm.vel_y`, giving the ball an immediate upward velocity that the physics integrator will process over subsequent frames.

---

## Modding Recipes

### 1. Super Jump

```cpp
void* ball = playerObject;
void* cm = *(void**)((DWORD)ball + 0x1A4);
*(float*)((DWORD)cm + 0x0CA8) = 50.0f;  // Double the original mod's force
```

### 2. Horizontal Launch (dash)

```cpp
void* ball = playerObject;
void* cm = *(void**)((DWORD)ball + 0x1A4);
*(float*)((DWORD)cm + 0x0CA4) = 100.0f;  // Blast forward in X
*(float*)((DWORD)cm + 0x0CAC) = 0.0f;   // No Z velocity
```

### 3. Velocity Freeze (stop instantly)

```cpp
void* ball = playerObject;
void* cm = *(void**)((DWORD)ball + 0x1A4);
*(float*)((DWORD)cm + 0x0CA4) = 0.0f;
*(float*)((DWORD)cm + 0x0CA8) = 0.0f;
*(float*)((DWORD)cm + 0x0CAC) = 0.0f;
```

### 4. Moon Gravity

```cpp
void* ball = playerObject;
void* cm = *(void**)((DWORD)ball + 0x1A4);
*(float*)((DWORD)cm + 0x0C78) = 5.0f;   // Default is 25.0f
```

### 5. Disable Gravity Entirely

```cpp
void* ball = playerObject;
void* cm = *(void**)((DWORD)ball + 0x1A4);
*(uint8_t*)((DWORD)cm + 0x0C7C) = 0;   // Set use_gravity = false
```

### 6. Turbo Speed (raise speed cap)

```cpp
void* ball = playerObject;
void* cm = *(void**)((DWORD)ball + 0x1A4);
*(float*)((DWORD)cm + 0x0C70) = 5000.0f;   // Default is 1000.0f
```

### 7. Slippery Mode (reduce friction)

```cpp
void* ball = playerObject;
void* cm = *(void**)((DWORD)ball + 0x1A4);
*(float*)((DWORD)cm + 0x0C68) = 0.1f;   // Default is ~0.56
```

### 8. Infinite Speed Scalar

```cpp
void* ball = playerObject;
void* cm = *(void**)((DWORD)ball + 0x1A4);
*(float*)((DWORD)cm + 0x0C64) = 9999.0f;   // Override speed scalar directly
```

### 9. Direction Override (face a specific way)

```cpp
void* ball = playerObject;
void* cm = *(void**)((DWORD)ball + 0x1A4);
*(float*)((DWORD)cm + 0x0C8C) = 0.0f;   // dir_x = 0
*(float*)((DWORD)cm + 0x0C90) = 1.0f;   // dir_y = up
*(float*)((DWORD)cm + 0x0C94) = 0.0f;   // dir_z = 0
// Call Ball_SetSpeed to update scaled_dir
```

---

## The Two Velocity Systems Side by Side

| Property | Ball Input Velocity | CollisionMesh Physics Velocity |
|----------|---------------------|-------------------------------|
| **Address** | `Ball + 0x170/174/178` | `CollisionMesh + 0xCA4/CA8/CAC` |
| **Type** | `float` accumulator | `float` persistent state |
| **Written by** | `Ball_ApplyForceWithMultipliers` | `Ball_Update`, `Ball_AdvancePositionOrCollision` |
| **Read by** | `Ball_Update` (each frame) | `Ball_Update`, `Ball_AdvancePositionOrCollision` |
| **Cleared?** | **YES** — set to 0 after reading | **NO** — persists across frames |
| **Use case** | Input forces from keys/gamepad | Final movement + gravity + collisions |
| **Mod effect** | Nothing (erased) | Actual movement change |

---

## Verified via Ghidra MCP

All offsets cross-referenced against live decompiled code:

- **`Ball_ctor2` (0x4039E0):** `operator_new(0xCB0)` → `CollisionMesh_ctor` → stored at `this + 0x1A4`
- **`Ball_ApplyForceWithMultipliers` (0x402650):** Writes to `Ball + 0x170/174/178`
- **`Ball_Update` (0x405E00):** Reads Ball.vel at `param_1[0x5c/5d/5e]` then clears to zero. Reads/writes `CollisionMesh + 0xCA4/CA8/CAC` via `param_1[0x69]`.
- **`Ball_AdvancePositionOrCollision` (0x405640):** Integrates `CollisionMesh + 0xCA4/CA8/CAC` with gravity, friction, collision response.
- **`CollisionMesh_ctor` (0x405680):** Initializes `+0xCA4/CA8/CAC` to `0,0,0`. Sets `+0x10` = owner Ball.
- **`Ball_InitBattleMode` (0x4056D0):** Initializes `+0xC60` through `+0xC94`.
- **`Ball_SetVec3AtOffset` (0x402A20):** Directly overwrites `+0xCA4/CA8/CAC`.
- **`Ball_SetSpeed` (0x4029C0):** Sets `+0xC64` = speed, updates `+0xC98/C9C/CA0` = scaled_dir.
- **`Ball_ResetCollisionMesh` (0x4030B0):** Resets `+0xC8C/C90/C94` to `(0, -1, 0)`.

---

## Summary

The jump mod is **correct and sophisticated**. The author understood the engine's architecture:

1. `Ball + 0x1A4` is a **pointer** to a nested `CollisionMesh` object (size `0xCB0`)
2. `CollisionMesh + 0xCA8` is the **persistent physics velocity Y**, not the ephemeral input accumulator
3. A near-zero `cm.vel_y` means the ball is grounded (collision response canceling gravity)
4. Injecting `20.0f` creates an immediate upward impulse that the physics integrator processes
5. Writing to `Ball + 0x174` would do **nothing** because that field is cleared every frame

The mod **bypasses** the normal `Ball_ApplyForceWithMultipliers` → `Ball_Update` flow and writes directly to the physics state. This is the correct low-level approach for instant-response cheats.
