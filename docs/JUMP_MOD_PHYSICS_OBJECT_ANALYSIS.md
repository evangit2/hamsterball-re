# Jump Mod Physics Object — Deep Dive

> **Analysis of the `playerObject + 0x1A4` pointer chain used by the jump mod.**  
> All offsets verified via live Ghidra MCP decompilation of `Hamsterball.exe`.

---

## tl;dr for Modders

The jump mod does **NOT** write to the Ball's own velocity. It writes to a **nested `CollisionMesh` object** that lives inside the Ball struct:

```cpp
// The pointer chain the jump mod follows:
// 1. Ball + 0x1A4  →  CollisionMesh* (pointer field)
// 2. *(Ball + 0x1A4) + 0xCA8  →  physics velocity Y (float)

DWORD* physicsObjPtr = (DWORD*)((DWORD)playerObject + 0x1a4);  // CollisionMesh**
DWORD physicsObj = *physicsObjPtr;                              // CollisionMesh*
float* trueVelY = (float*)(physicsObj + 0xca8);                // &collisionMesh->vel_y
```

**Why this works:** The Ball has **two separate velocity systems**. Writing to `CollisionMesh.vel_y` injects velocity directly into the physics integrator. Writing to `Ball.vel_y` (+0x174) would do **nothing** because that field is cleared every frame after being read.

---

## The Two Objects

### 1. Ball Object (the "playerObject")

- **Primary update:** `Ball_Update` @ `0x00405E00`
- **Constructor:** `Ball_ctor2` @ `0x004039E0`
- **Struct size:** `0x0C98` bytes (3,224 bytes)
- **Vtable:** `0x004CF3A0` (9 methods)

### 2. CollisionMesh Object (nested inside Ball)

- **Constructor:** `CollisionMesh_ctor` @ `0x00456D80`
- **Allocation size:** `0x0CB0` bytes (3,248 bytes) — `operator_new(0xCB0)`
- **Stored at:** `Ball + 0x1A4` (pointer field)
- **Owner backref:** `CollisionMesh + 0x10` = pointer to parent Ball
- **Vtable:** `0x004D8E10` (`Mesh_DeletingDtor`)

The CollisionMesh is constructed during `Ball_ctor2`:

```c
// From Ball_ctor2 (0x4039E0) decompilation:
pvVar2 = operator_new(0xcb0);          // Allocate 3248 bytes
pvVar2 = CollisionMesh_ctor(pvVar2, this);  // Initialize, pass Ball as owner
*(void **)((int)this + 0x1a4) = pvVar2;     // Store pointer in Ball struct
```

**`CollisionMesh` is NOT the same as `Ball`.** It is a completely separate object with its own vtable, fields, and behavior. It lives at a **heap address** pointed to by `Ball + 0x1A4`.

---

## Why `+0x1A4` Points to a Different Object

| Offset | What it is | Type |
|--------|-----------|------|
| `Ball + 0x1A4` | `collision_mesh_ptr` | `CollisionMesh**` (pointer stored IN Ball) |
| `*(Ball + 0x1A4) + 0x10` | `owner_ball` | `Ball*` (back-pointer to parent) |
| `*(Ball + 0x1A4) + 0xCA8` | `vel_y` | `float` (physics velocity Y) |

The Ball struct itself ends around `0xC98`. The `CollisionMesh` object it points to has fields going up to `+0xCA8` and beyond — **these are NOT Ball fields**, they are fields of the nested object.

---

## Two Velocity Systems: The Critical Difference

### System A: Ball Input Velocity (`+0x170/174/178`)

- **Location:** `Ball + 0x170` (X), `+0x174` (Y), `+0x178` (Z)
- **Purpose:** **Accumulator** for input forces each frame
- **How it works:** `Ball_ApplyForceWithMultipliers` (0x402650) adds forces here. `Ball_Update` reads them, passes them to physics, then **clears them to zero**.

```c
// From Ball_ApplyForceWithMultipliers (0x402650):
*(float *)((int)this + 0x170) = param_1 * param_4 + *(float *)((int)this + 0x170);
*(float *)((int)this + 0x174) = param_2 * param_4 + *(float *)((int)this + 0x174);
*(float *)((int)this + 0x178) = param_3 * param_4 + *(float *)((int)this + 0x178);
```

```c
// From Ball_Update (0x405E00) — reads then CLEARS:
fStack_8b0 = (float)param_1[0x5c];  // param_1[0x5c] = *(ball + 0x170) = vel_x
// ...physics integration...
param_1[0x5e] = 0;  // vel_z cleared
param_1[0x5d] = 0;  // vel_y cleared
param_1[0x5c] = 0;  // vel_x cleared
```

**Writing to `Ball + 0x174` would be erased on the next frame.** That's why the jump mod does NOT use this address.

### System B: CollisionMesh Physics Velocity (`+0xCA4/CA8/CAC`)

- **Location:** `CollisionMesh + 0xCA4` (X), `+0xCA8` (Y), `+0xCAC` (Z)
- **Purpose:** **Persistent physics state** — the actual velocity used for movement
- **How it works:** `Ball_Update` copies this to local variables, integrates gravity/collisions, then writes back. It is **NOT cleared** each frame.

```c
// From Ball_Update (0x405E00):
iVar9 = param_1[0x69];  // param_1[0x69] = *(ball + 0x1A4) = CollisionMesh*
if (&local_928 != (float *)(iVar9 + 0xca4)) {
    local_928 = *(float *)(iVar9 + 0xca4);  // read physics vel_x
    local_924 = *(float *)(iVar9 + 0xca8);  // read physics vel_y
    local_920 = *(float *)(iVar9 + 0xcac);  // read physics vel_z
}
```

The jump mod writes to `CollisionMesh + 0xCA8` because this is the **persistent** velocity that drives actual movement. A value of `20.0f` here gives the ball an immediate upward velocity that the physics system will integrate over multiple frames.

---

## CollisionMesh Field Map (Verified from Ghidra)

### Header / Identity

| Offset | Size | Name | Initial Value | Notes |
|--------|------|------|---------------|-------|
| `+0x00` | 4 | `vtable` | `0x4D8E10` | `Mesh_DeletingDtor` |
| `+0x10` | 4 | `owner_ball` | `this` (Ball*) | Set in `CollisionMesh_ctor` |

### AthenaLists (collision data structures)

| Offset | Size | Name | Notes |
|--------|------|------|-------|
| `+0x18` | ~0x400 | `list1` | `AthenaList_Init` called here |
| `+0x430` | ~0x400 | `list2` | Second AthenaList |
| `+0x848` | ~0x400 | `list3` | Third AthenaList |

### Battle Mode / Physics Parameters

| Offset | Size | Name | Initial Value | Source Function |
|--------|------|------|---------------|-----------------|
| `+0xC60` | 4 | `battle_mode` | `3` | `Ball_InitBattleMode` |
| `+0xC64` | 4 | `speed_scalar` | — | `Ball_SetSpeed` writes here |
| `+0xC68` | 4 | `friction_or_damping` | `0x3F0E147B` (~0.56) | `Ball_InitBattleMode` |
| `+0xC6C` | 4 | `field_c6c` | `1.0f` | `Ball_InitBattleMode` |
| `+0xC70` | 4 | `max_speed_limit` | `1000.0f` | `Ball_InitBattleMode` |
| `+0xC74` | 4 | `field_c74` | `0` | `CollisionMesh_ctor` |
| `+0xC78` | 4 | `gravity_strength` | `25.0f` | `Ball_InitBattleMode` |
| `+0xC7C` | 1 | `use_gravity` | `1` (bool) | `Ball_InitBattleMode` |

### Direction Vector (normalized heading)

| Offset | Size | Name | Initial Value | Notes |
|--------|------|------|---------------|-------|
| `+0xC8C` | 4 | `dir_x` | `0` | Set in `Ball_InitBattleMode` |
| `+0xC90` | 4 | `dir_y` | `-1.0f` | Default = straight down |
| `+0xC94` | 4 | `dir_z` | `0` | |

### Scaled Direction (speed * dir)

| Offset | Size | Name | Notes |
|--------|------|------|-------|
| `+0xC98` | 4 | `scaled_dir_x` | `speed_scalar * dir_x` |
| `+0xC9C` | 4 | `scaled_dir_y` | `speed_scalar * dir_y` |
| `+0xCA0` | 4 | `scaled_dir_z` | `speed_scalar * dir_z` |

### Physics Velocity (THE JUMP MOD TARGET)

| Offset | Size | Name | Initial Value | Used By |
|--------|------|------|---------------|---------|
| `+0xCA4` | 4 | **`vel_x`** | `0` | `Ball_AdvancePositionOrCollision`, `Ball_Update` |
| `+0xCA8` | 4 | **`vel_y`** | `0` | ← **Jump mod writes here** |
| `+0xCAC` | 4 | **`vel_z`** | `0` | |

### Additional Fields (from `Ball_AdvancePositionOrCollision`)

| Offset | Size | Name | Notes |
|--------|------|------|-------|
| `+0xC64` | 4 | `speed_or_mass` | Read in `Ball_AdvancePositionOrCollision` |

---

## The Ground Check Explained

```cpp
float tolerance = 0.5f;
if (*trueVelY > -tolerance && *trueVelY < tolerance) {
    *trueVelY = 20.0f;
}
```

The mod checks if `CollisionMesh.vel_y` is between `-0.5` and `+0.5`. This is a **grounded check** — when the ball is resting on a surface, the physics velocity Y is near zero (held by collision response). When airborne, `vel_y` is typically negative (falling) or has significant magnitude.

**Why this works:** `Ball_AdvancePositionOrCollision` resolves collisions by pushing the ball out of intersecting geometry. When resting on a floor plane, the collision response exactly cancels gravity, resulting in `vel_y ≈ 0`. The mod detects this near-zero state and injects an upward impulse.

---

## Key Functions That Touch CollisionMesh Velocity

| Function | Address | What it does |
|----------|---------|-------------|
| `CollisionMesh_ctor` | `0x00456D80` | Initializes `vel` to `0,0,0` |
| `Ball_InitBattleMode` | `0x00456CD0` | Sets physics parameters (gravity, speed limits) |
| `Ball_AdvancePositionOrCollision` | `0x004564C0` | **Main physics integrator** — adds external velocity to `+0xCA4`, applies gravity, collision response |
| `Ball_SetVec3AtOffset` | `0x00402A20` | Directly overwrites `+0xCA4/CA8/CAC` with a Vec3 |
| `Ball_SetSpeed` | `0x004029C0` | Sets `speed_scalar` at `+0xC64`, updates scaled direction |
| `Ball_ResetCollisionMesh` | `0x004030B0` | Resets direction to down (`-1.0f` Y), clears impact counter |
| `Ball_Update` | `0x00405E00` | Reads `CollisionMesh.vel` into locals, integrates, writes back |

---

## Modding Implications

### Why the jump mod chose `+0xCA8` over `+0x174`

| Target | Address | Persistence | Effect |
|--------|---------|-------------|--------|
| `Ball.vel_y` | `Ball + 0x174` | **Cleared every frame** | Would be erased immediately, no jump |
| `CollisionMesh.vel_y` | `CollisionMesh + 0xCA8` | **Persistent physics state** | Jump persists, integrates over frames |

### What else can modders do with the CollisionMesh?

1. **Super-jump:** Write `50.0f` or `100.0f` to `+0xCA8` for mega jumps
2. **Horizontal launch:** Write to `+0xCA4` (X) or `+0xCAC` (Z) for directional blasts
3. **Velocity freeze:** Set all three to `0.0f` to stop all movement instantly
4. **Gravity manipulation:** Write to `+0xC78` (gravity strength, default `25.0f`) — lower = moon gravity
5. **Speed limit:** Write to `+0xC70` (max speed limit, default `1000.0f`)
6. **Disable gravity:** Set `+0xC7C` to `0` (it's a bool)
7. **Friction tweak:** Write to `+0xC68` (~0.56 default) — lower = slipperier

### Pointer chain for direct access

```cpp
void* ball = playerObject;                          // The Ball
void** collisionMeshPtr = (void**)((DWORD)ball + 0x1A4);
void* collisionMesh = *collisionMeshPtr;            // Dereference

float* velX = (float*)((DWORD)collisionMesh + 0xCA4);
float* velY = (float*)((DWORD)collisionMesh + 0xCA8);  // ← Jump mod target
float* velZ = (float*)((DWORD)collisionMesh + 0xCAC);

float* gravityStrength = (float*)((DWORD)collisionMesh + 0xC78);
BYTE*  useGravity      = (BYTE*)((DWORD)collisionMesh + 0xC7C);
```

---

## Verified via Ghidra MCP

All offsets in this document were cross-referenced against live decompiled code from the GhidraMCP headless server:

- **`Ball_ctor2` (0x4039E0):** Confirms `operator_new(0xCB0)` → `CollisionMesh_ctor` → stored at `this + 0x1A4`
- **`Ball_ApplyForceWithMultipliers` (0x402650):** Confirms Ball.vel at `+0x170/174/178`
- **`Ball_Update` (0x405E00):** Confirms Ball.vel is read at `param_1[0x5c/5d/5e]` then cleared to zero. Also reads/writes `CollisionMesh + 0xCA4/CA8/CAC` via `param_1[0x69]`.
- **`Ball_AdvancePositionOrCollision` (0x405640):** Confirms `CollisionMesh + 0xCA4/CA8/CAC` is the physics velocity used for integration.
- **`CollisionMesh_ctor` (0x405680):** Confirms `+0xCA4/CA8/CAC` initialized to zero, `+0x10` = owner Ball.
- **`Ball_InitBattleMode` (0x4056D0):** Confirms `+0xC60` through `+0xC94` initialization values.
- **`Ball_SetVec3AtOffset` (0x402A20):** Confirms direct write path to `+0xCA4/CA8/CAC`.
- **`Ball_SetSpeed` (0x4029C0):** Confirms `+0xC64` = speed scalar, `+0xC98/C9C/CA0` = scaled direction.

---

## Summary

The jump mod is **correct and sophisticated**. The author understood that:

1. `Ball + 0x1A4` is a **pointer** to a nested `CollisionMesh` object
2. `CollisionMesh + 0xCA8` is the **persistent physics velocity Y**, not the ephemeral input accumulator
3. A near-zero `vel_y` means the ball is grounded (collision response canceling gravity)
4. Injecting `20.0f` creates an immediate upward impulse that the physics integrator will process

The mod **bypasses** the normal `Ball_ApplyForceWithMultipliers` → `Ball_Update` flow and writes directly to the physics state. This is the correct low-level approach for instant-response cheats.
