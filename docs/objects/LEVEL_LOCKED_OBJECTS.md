# Level-Locked Objects & Spawn Methods

Hamsterball's object factory (`CreateLevelObjects` at `0x4121D0`) is the central dispatcher that reads named objects from the MESHWORLD file and creates game objects. However, several object types are gated behind two independent mechanisms:

1. **Tournament mode gate** (`app+0x23C != 0`) — prevents objects from spawning in non-tournament/non-arena modes
2. **Scene mesh pointer dependency** — object constructors reference pre-loaded mesh pointers that only exist if the specific `BoardLevel*_ctor` loaded the sub-mesh file

This document covers which objects are locked, why, and how to spawn them anyway.

---

## Object Categories

### Category 1: Self-Loading Objects (spawn anywhere)

These objects load their own MeshWorld from a hardcoded file path inside their constructor. They create their own `CollisionLevel` internally. The **only** barrier is the `app+0x23C` tournament flag gate.

| Object | Factory Match | Constructor | Address | Loads File | Struct Size | Gate |
|--------|--------------|-------------|---------|-----------|-------------|------|
| **Bonk** | `"BONK"` | `Bonk_ctor` | `0x438850` | `levels\level5-bonk` | 0x1200 | `app+0x23C != 0` |
| **Bumper** | `"N:BUMPER%d"` | `CreateBumper` | `0x40FA20` | `levels\level8` | 0x10D0 | None |
| **Bumper2** | `"N:BUMPER%d"` | `CreateBumper2` | `0x413CE0` | `levels\arena-beginner` | 0x10D0 | None |
| **Bridge** | `"BRIDGE"` | (inline in factory) | `0x4121D0` | Uses `scene+0x436C`/`0x4370` | — | None |
| **BreakBridge1** | `"BBRIDGE1"` | `BreakBridge_ctor` | `0x436D70` | Uses `scene+0x5410` | 0x1100 | None |
| **BreakBridge2** | `"BBRIDGE2"` | `BreakBridge_ctor` | `0x436D70` | Uses `scene+0x5414` | 0x1100 | None |
| **PopCylinder** | `"POPCYLINDER"` | `PopCylinder_ctor` | `0x436EE0` | Uses `scene+0x5420` | 0x10E8 | None |

### Category 2: Scene-Dependent Objects (level-locked)

These constructors receive a pre-loaded mesh pointer as a parameter. That pointer only exists if the specific level constructor loaded the sub-mesh file. **Without the pointer, calling the constructor will crash.**

| Object | Factory Match | Constructor | Address | Needs Scene Offset | Mesh File | Loaded By | Gate |
|--------|--------------|-------------|---------|-------------------|-----------|-----------|------|
| **Tipper** | `"TIPPER"` | `Tipper_ctor` | `0x437960` | `+0x4394` (mesh), `+0x4398` (visual) | `Levels\Level3-Tipper` | `LevelBoard_Dizzy_ctor` (`0x41D060`) | `app+0x23C != 0` |
| **Gluebie** | `"GLUEBIE"` | `Gluebie_ctor` | `0x437CB0` | `+0x607C` (mesh) | `Levels\Level3-Gluebie` | `LevelBoard_Dizzy_ctor` (`0x41D060`) | `app+0x23C != 0` |
| **BlockDawg1** | `"BLOCKDAWG1"` | `Blockdawg_ctor` | `0x43C310` | `+0x5840` (mesh) + `"DAWGPATH1"` named object | (dawg sub-mesh) | Level ctors | `app+0x23C != 0` |
| **BlockDawg2** | `"BLOCKDAWG2"` | `Blockdawg_ctor` | `0x43C310` | `+0x5844` (mesh) + `"DAWGPATH2"` named object | (dawg sub-mesh) | Level ctors | `app+0x23C != 0` |
| **Catapult** | `"CATAPULT"` | `Catapult_ctor` | `0x437E10` | `+0x5848` (mesh) | (catapult sub-mesh) | Level ctors | None |

---

## Spawning Self-Contained Objects (No MESHWORLD Edit Needed)

### BadBalls (8-balls / AI opponents)

BadBalls are spawned by `CreateBadBall` which is called from `Scene_SpawnBallsAndObjects`. They do NOT go through `CreateLevelObjects` — they scan ALL MESHWORLD section 3 objects for names starting with "BADBALL".

**`CreateBadBall`** — Address: `0x0040BCA0`
- **Convention:** `__fastcall` (MSVC), first arg = Scene\* (param_1)
- **Parameters:** `param_1` = Scene\*
- **MESHWORLD iteration:** Walks `scene+0x8AC → +0x480 → +0xCA0` object array, checks `__strnicmp(name, "BADBALL", 7)`
- **MESHWORLD tag format (in object name string):**
  ```
  BADBALL<CHASE>25.0</CHASE><HOME>5.0</HOME><SIZE>2.0</SIZE>
  ```
  | Tag | Default | Offset in Ball | Description |
  |-----|---------|----------------|-------------|
  | `<CHASE>val</CHASE>` | 25.0 | `ball+0xC6C` | AI chase activation distance (piVar5[0x31b]) |
  | `<HOME>val</HOME>` | 5.0 | `ball+0xC70` | AI patrol/home radius (piVar5[0x31c]) |
  | `<SIZE>val</SIZE>` | 1.0 | `ball+0x188` | Base radius → 3.0 (piVar5[0x62]=0x40400000=3.0f, piVar5[0xa1]=size_val, piVar5[0x9f]=0, piVar5+0x313 flag=1) |
  | `<SPINDISTANCE>val</SPINDISTANCE>` | (none) | `ball+0xC7C` | AI spin orbit distance (piVar5[799]) |

- **Per-ball spawn sequence (from decompilation):**
  1. `operator_new(0xC98)` — allocate 8-ball (larger than player's 0xC60)
  2. `Ball_ctor(this, scene)` — initialize ball with scene reference
  3. `vtable[1]()` — call Ball_Init (2nd virtual)
  4. Position from MESHWORLD object: `ball+0x164 = obj.x + radius`, `ball+0x168 = obj.y + ball.radius`, `ball+0x16C = obj.z + radius`
  5. Clear `ball+0x281 = 0` (dead flag, never read)
  6. Copy same position to home: `ball+0xC60/0xC64/0xC68 = obj.xyz` (spawn/return position)
  7. Parse `<CHASE>`, `<HOME>`, `<SIZE>`, `<SPINDISTANCE>` tags from name string via `MWParser_ReadTag`
  8. `AthenaList_Append(scene+0x29D4, ball)` — add to bad_balls list
  9. `AthenaList_Append(scene+0x2DEC, ball)` — add to all_balls list

- **⚠️ MESHWORLD dependency:** `CreateBadBall` reads position AND tags from MESHWORLD section 3 object entries. You CANNOT call it directly with a custom position — it iterates the object array internally.

#### Spawning a BadBall without MESHWORLD (manual spawn)

You can bypass `CreateBadBall` entirely and construct a BadBall manually by replicating what it does. Here is a code-level pseudocode for spawning at an arbitrary position:

```c
// Manual BadBall spawn — no MESHWORLD needed
// Call from a hook or patched function with access to Scene* (param_1)

void* scene = (void*)param_1;  // Scene pointer

// 1. Allocate
void* ball = operator_new(0xC98);

// 2. Construct
Ball_ctor(ball, scene);  // 0x4087A0 (base Ball_ctor, NOT Ball_Split_ctor)

// 3. Init (vtable[1])
((void(__thiscall*)(void*))(*(void***)ball)[1])(ball);

// 4. Set position (ball+0x164/168/16C = display position)
*(float*)(ball + 0x164) = spawn_x;
*(float*)(ball + 0x168) = spawn_y;
*(float*)(ball + 0x16C) = spawn_z;

// 5. Set home position (ball+0xC60/C64/C68 = return-to position)
*(float*)(ball + 0xC60) = spawn_x;
*(float*)(ball + 0xC64) = spawn_y;
*(float*)(ball + 0xC68) = spawn_z;

// 6. Set AI flag
*(byte*)(ball + 0x281) = 0;  // dead flag (never read by any function)

// 7. Set chase distance (optional, default 25.0)
*(float*)(ball + 0xC6C) = 25.0f;

// 8. Set home radius (optional, default 5.0)
*(float*)(ball + 0xC70) = 5.0f;

// 9. Set size (optional, default 1.0 = normal)
// To make giant: *(float*)(ball + 0x188) = 3.0f; *(byte*)(ball + 0xC4C) = 1;

// 10. Register in scene lists
AthenaList_Append(scene + 0x29D4, ball);  // bad_balls
AthenaList_Append(scene + 0x2DEC, ball);  // all_balls
```

**Key addresses for manual spawn:**

| Step | Function/Offset | Address |
|------|-----------------|---------|
| Allocate | `operator_new(0xC98)` | runtime |
| Ball_ctor | `Ball_ctor(ball, scene)` | `0x40AFE0` |
| Ball_ctor2 | `Ball_ctor2(ball, scene)` (base init only) | `0x4039E0` |
| Ball_Split_ctor | `Ball_Split_ctor(ball, scene)` (8-ball variant) | `0x408D10` |
| Ball_Init | `vtable[1]()` | via vtable |
| Display pos | `ball+0x164/0x168/0x16C` | — |
| Home pos | `ball+0xC60/0xC64/0xC68` | — |
| AI flag clear | `ball+0x281` | DEAD: never read |
| Chase dist | `ball+0xC6C` | — |
| Home radius | `ball+0xC70` | — |
| Add to bad_balls | `AthenaList_Append(scene+0x29D4, ball)` | — |
| Add to all_balls | `AthenaList_Append(scene+0x2DEC, ball)` | — |

**Alternatively, for split-ball 8-balls (from Ball_Shatter at 0x408D70):**
- Uses `Ball_Split_ctor` at `0x408D10` (which calls `Ball_ctor2` then sets `ball+0xC60=5`)
- Allocates 0xC64 bytes (slightly smaller than CreateBadBall's 0xC98)
- Sets `ball+0x31D = 1` (is_8ball), `ball+0xC60 = 0x41200000` (30.0f, split timer)
- Copies collision direction from parent at `ball+0xCA4/0xCA8/0xCAC`
- Sets split ID: 1, 2, or 4 (`ball+0x324`)

- **To add BadBalls via MESHWORLD:** Add objects of type 1 in section 3 with names like `BADBALL<CHASE>30</CHASE><SIZE>3</SIZE>`. They will be spawned automatically on level load when `app+0x23C != 0` (tournament mode).

### MouseTraps

MouseTraps are spawned by `CreateMouseTrap` based on MESHWORLD object names starting with `"N:MOUSETRAP"`.

**`CreateMouseTrap`** — Address: `0x0040BF50`
- **Convention:** `__fastcall` (param_1 = Scene*)
- **MESHWORLD iteration:** Same pattern as `CreateBadBall` — walks `scene+0x8AC → +0x480 → +0xCA0` object array, checks `__stricmp(name, "MOUSETRAP")` (case-insensitive)
- **Alloc size:** 0x10F8 bytes per MouseTrap
- **Constructor:** `TipperVisual_Level_Ctor(this, scene)` at `0x437880`
- **Position copied from object:** `obj+0x10DC/0x10E0/0x10E4 = obj.x/y/z`
- **Floor offset:** `obj+0x10F4 = _DAT_004CF44C - obj.rot_y` (constant minus Y rotation for floor height)
- **Post-init:** Calls `vtable[0x2C]()` (virtual function at offset 0xB0 in vtable, likely Activate)
- **Scene storage:**
  - `AthenaList_Append(scene+0xCD4, obj)` — mouse_trap_list
  - `AthenaList_Append(scene+0x1930, obj)` — physics_objects
  - `AthenaList_Append(scene+0x8AC→0x480→0x1C, obj)` — level_objects
  - `AthenaList_Append(scene+0x8B0+0x18, obj+0x10D4)` — some sub-component
- **Gate:** None (not gated by `app+0x23C`)
- **Scene mesh dependency:** The constructor calls `TipperVisual_Level_Ctor(scene)` — reads `scene+0x4398` (Tipper CollisionLevel). Must be loaded by BoardLevel3 ctor.

**Manual spawn:**
```c
void* mem = operator_new(0x10F8);
void* trap = TipperVisual_Level_Ctor(mem, scene);
*(float*)(trap + 0x10DC) = obj.x;  // position
*(float*)(trap + 0x10E0) = obj.y;
*(float*)(trap + 0x10E4) = obj.z;
*(float*)(trap + 0x10F4) = _DAT_004CF44C - obj.rot_y;  // floor height
((void(__thiscall*)(void*))(*(void***)(trap))[0x0B])(trap);  // Activate
AthenaList_Append(scene + 0xCD4, trap);   // mouse_trap_list
AthenaList_Append(scene + 0x1930, trap);  // physics_objects
```

### Sawblades (and Arena Object Sub-Dispatcher)

⚠️ **`CreateExpertLevelObjects` is actually a multi-factory**, not just a sawblade creator. It handles 6 different arena objects based on the name prefix. It is called from `CreateLevelObjects` for each section-3 object whose name doesn't match any other prefix.

**`CreateExpertLevelObjects`** — Address: `0x0040E250`
- **Convention:** `__thiscall` (ECX = Scene\*, param_1 = name string, param_2/3 = output ptrs, param_4 = transform)
- **Called from:** `Scene_HandleCollisions` / `CreateLevelObjects`
- **Gate:** All sub-objects with `app+0x23C != 0` check: BONK, TIP, SAWBLADE

**Sub-dispatcher branches:**

| Prefix | Object Type | Alloc Size | Constructor | Notes |
|--------|-------------|-----------|-------------|-------|
| `BONK` | Bonk hammer | 0x1200 | `Bonk_ctor` at `0x438850` | Reads position from `param_4+4/8/C`; gate: `app+0x23C != 0` |
| `TIP` | Tower (swinging arm) | 0x1188 | `TowerLevel_Ctor` | Checks `SLOW`, `SUPER`, `UP` suffixes; gate: `app+0x23C != 0` |
| `SAWBLADE` | Spinning saw | 0x111C | `Sawblade_Level_Ctor` | Suffix `1` → `scene+0x4370`, suffix `2` → `scene+0x4374`; gate: `app+0x23C != 0` |
| `BRIDGE` | Spinner platform | 0x10FC | `Spinner_Level_ctor` | Suffix `1` → `scene+0x4380`, suffix `2` → `scene+0x4798`, suffix `NEG` → `obj+0x10F8 = -1.0f` |
| `JUDGE` | Gear | 0x1100 | `Gear_Level_ctor` | Appended to `scene+0x4BBC` |
| `BELL` | Bell (Tipper variant) | 0x10E8 | `Tipper_Level_Ctor` | Appended to `scene+0x2578`; stored at `scene+0x4FD4` |

All created objects get appended to `scene+0x2578` (active objects list).

**SAWBLADE details:**
- `SAWBLADE1` → stored at `scene+0x4370`, calls `Sawblade_SetBreakSound(1)`
- `SAWBLADE2` → stored at `scene+0x4374`, calls `Sawblade_SetBreakSound(2)`
- Triggered by collision events `E:ALERTSAW1` / `E:ACTIVATESAW1`

**TIP details:**
- `TIPSLOW` → `obj+0x10EC = 1`
- `TIPSUPER` → `obj+0x10ED = 1`
- `TIPUP` → calls `Sound_InitChannels(obj, 1)`

**BRIDGE spinner details:**
- `BRIDGE1` → appended to `scene+0x4380`
- `BRIDGE2` → appended to `scene+0x4798`
- `BRIDGENEG` → sets spin direction to -1.0f at `obj+0x10F8`

### Bonks (Hammer popups)

**`Bonk_ctor`** — Address: `0x00438850`
- **Convention:** `__thiscall` (ECX = this, allocated buffer)
- **Parameters:** `this` (void\*), `scene` (int), `x` (float), `param_3` (undefined4), `param_4` (undefined4)
- **Self-loads:** `MeshWorld_ctor(this, graphics, "levels\\level5-bonk")`
- **Creates own collision:** `CollisionLevel_ctorWithLevel(collision, this)`
- **Size:** 0x1200 bytes
- **Scene storage:** Result stored at `scene+0x540C`
- **Gate:** `app+0x23C != 0`

**This is the easiest level-locked object to unlock.** Since `Bonk_ctor` loads its own mesh file (`levels\level5-bonk`), it has no scene mesh pointer dependency. The ONLY thing preventing Bonks from appearing in any level is the `app+0x23C` check at `0x4121D0`.

To add Bonks:
1. Add object named `"BONK_01"` (or any BONK-prefixed name) in MESHWORLD section 3
2. Ensure `app+0x23C != 0` OR patch the gate

### Bumpers (⚠️ Level-replacement hazard)

**`CreateBumper`** — Address: `0x0040FA20`
- **Convention:** `__fastcall` (param_1 = Scene\* as `int*`)
- **⚠️ CRITICAL GOTCHA:** This does NOT just spawn bumper objects. It loads the **entire** `levels\level8` MESHWORLD file as a new MeshWorld, creates a CollisionLevel from it, calls `Level_InitScene`, then scans for 8 `N:BUMPER%d` named objects. This **replaces the level's scene data** — the current level's meshes get overwritten at `scene+0x22B` (MeshWorld) and `scene+0x22C` (CollisionLevel). **Calling CreateBumper on non-Level8 levels will crash or corrupt the scene** because it overwrites the active level's collision/geometry.
- **Self-loads:** `MeshWorld_ctor(buf, graphics, "levels\\level8")` → stored at `param_1[0x22B]`
- **CollisionLevel:** `CollisionLevel_ctorWithLevel(buf2, meshWorld)` → stored at `param_1[0x22C]`
- **Init:** `Level_InitScene(param_1)` — reinitializes scene with new level data
- **8× bumpers:** `Scene_CollectByNameFilter("N:BUMPER%d", ...)` at stride 0x106 (0x10E3 base)
- **Post-init:** `(*param_1)[0x80]()` — calls vtable offset 0x80 (virtual function, likely level-specific setup)
- **Size:** 0x10D0 bytes per allocation
- **Gate:** None

**`CreateBumper2`** — Address: `0x00413CE0`
- Same pattern but loads `levels\arena-beginner` — also replaces scene data
- **Size:** 0x10D0 bytes
- **Gate:** None

**Bottom line:** Both `CreateBumper` variants are level-scoped constructors, not standalone object spawners. They work correctly only when called from within a BoardLevel8 or Arena board constructor that expects this level-swap behavior. Calling them from another level context will corrupt the scene.

### Ball Splits (8-ball arena mechanic)

**`Ball_Shatter`** — Address: `0x00408D70`
- **Convention:** `__thiscall` (ECX = Ball*)
- **Prologue:** Standard SEH (`PUSH -1; MOV EAX, FS:[0]; PUSH handler`)
- **Parameters:** `this` (Ball* — the parent ball), `param_1` (struct with target position table)
- **Called from:** `FollowBall_Update` (0x43ECC0) at two call sites (0x43F722, 0x43FE36)
- **NOT called from E:JUMP.** The E:JUMP collision handler (in collision_events.c) does something different: plays a 3D sound, applies upward force, and adds +200 score. See docs/collision_events.
- **Behavior:** Marks the parent ball for despawn (+0x2E8=1), then creates 3 new `Ball_Split` objects (0xC64 bytes each) via `Ball_Split_ctor` (0x408D10). Each split ball:
  - `ball+0x31D = 1` (is_8ball flag, activates AI behavior)
  - `ball+0xC6 = 0x41200000` (10.0f — split ball size, NOT 30.0f)
  - Split IDs: 1, 2, 4 (bitmask, assigned by iteration index)
  - Trajectory copied from parent's `+0x2AC–0x2B8`
  - Position from parent or lookup table via `param_1`
  - Added to `scene+0x3204` (ball list)
- **Guard conditions:**
  - `+0x324 == 0` (only non-8ball-type balls can split)
  - `+0x744 == 0` (only split once — split_count must be zero)

### Ball Init Battle Mode

**`Ball_InitBattleMode`** — Address: `0x00456CD0`
- **Convention:** `__thiscall` (ECX = Ball*)
- **Prologue:** `SUB ESP, 0C; PUSH ESI; LEA EAX, [ECX+0xC80]; ...`
- **Sets ball to arena/battle physics:**

| Offset | Value | Description |
|--------|-------|-------------|
| `+0xC60` | 3 | battle_mode = 3 |
| `+0xC68` | 0.55 | friction |
| `+0xC6C` | 1.0 | bounciness |
| `+0xC70` | 1000.0 | max_speed |
| `+0xC74` | 25.0 | chase_distance |
| `+0xC80` | 0.0 | gravity_vec[0] |
| `+0xC84` | -1.0 | gravity_vec[1] |
| `+0xC88` | 0.0 | gravity_vec[2] |

---

## Additional Spawn Functions (Not in CreateLevelObjects)

These functions are called from `Scene_SpawnBallsAndObjects` (0x41C5B0) AFTER the main level objects are created. They don't go through the object factory dispatcher.

### CreateSecretObjects

**Address:** Unknown (not yet decompiled)
- **Called from:** `Scene_SpawnBallsAndObjects` at `0x41C5B0`
- **Purpose:** Creates hidden collectible objects (secret items/bonuses)
- **Likely scans:** MESHWORLD for objects with `SECRET` or similar prefix
- **Gate:** Unknown — may be gated by `app+0x23C`

### Scene_CreateFlags

**Address:** Unknown (not yet decompiled)
- **Called from:** `Scene_SpawnBallsAndObjects` at `0x41C5B0`
- **Purpose:** Creates checkpoint/finish-line flag objects
- **Likely scans:** MESHWORLD for `FLAG` or `FINISH` named objects

### Scene_CreateSigns

**Address:** Unknown (not yet decompiled)
- **Called from:** `Scene_SpawnBallsAndObjects` at `0x41C5B0`
- **Purpose:** Creates directional arrow sign objects
- **Likely scans:** MESHWORLD for `SIGN` or `ARROW` named objects

### Scene_CreateDynamicObjects

**Address:** Unknown (not yet decompiled)
- **Called from:** `Scene_SpawnBallsAndObjects` at `0x41C5B0`
- **Purpose:** Creates moving/animated objects (platforms, elevators, etc.)
- **Likely scans:** MESHWORLD for `DYNAMIC` or `MOVING` named objects

---

## The `app+0x23C` Tournament Gate

The single variable that gates most level-locked objects. Located at `App+0x23C` (App singleton at `0x4FD680`, so absolute address `0x4FD8B4`).

**Objects gated by `app+0x23C != 0`:**
- `TIPPER` (at `0x4121D0+0x3A` — JNZ after CMP)
- `BONK` (at `0x4121D0+0xAA` — JNZ after CMP)
- `BLOCKDAWG1` (at `0x4121D0+0x1E2` — JNZ after CMP)
- `BLOCKDAWG2` (at `0x4121D0+0x235` — JNZ after CMP)
- `GLUEBIE` (inverted: at `0x4121D0+0x2F0` — JZ after CMP, skips if `0x23C == 0`)
- `CreateBadBall` (spawned from `Scene_SpawnBallsAndObjects` `0x41C5B0`)

**How to bypass:**
1. **Memory patch:** Write `1` to address `0x4FD8B4` at runtime
2. **Binary patch:** In `CreateLevelObjects` at `0x4121D0`, patch each conditional jump after the `app+0x23C` comparison:
   - Find the `CMP [addr], 0` + `JNZ`/`JZ` sequences
   - Replace `JNZ` with `JMP` (unconditional) to always take the branch
   - For GLUEBIE's inverted check, replace `JZ` with `JMP` (or NOP out the skip)

---

## Scene Mesh Dependency Reference

For scene-dependent objects, here is exactly what each level constructor loads and where it stores the mesh pointers:

### `LevelBoard_Dizzy_ctor` (0x41D060) — Dizzy Race
Loads the most sub-meshes of any level:

| Scene Offset | Mesh File | Used By |
|-------------|-----------|---------|
| `+0x436C` | `Levels\Level3-Tipper` (MeshWorld) | BRIDGE collision mesh, TIPPER visual |
| `+0x4370` | (CollisionLevel from Tipper) | Collision against tipper geometry |
| `+0x4374` | `Levels\Level3-Gluebie` (MeshWorld) | GLUEBIE visual |
| `+0x4BA8` | `Levels\Level3-WaterWheel` (MeshWorld) | Level3-specific water wheel |
| `+0xBAC` | (CollisionLevel from WaterWheel) | Water wheel collision |
| `+0x4BC4` | `Levels\Level3-Swirl` (MeshWorld) | Level3-specific swirl |
| `+0xBC8` | (CollisionLevel from Swirl) | Swirl collision |

### `LevelBoard_Expert_ctor` (0x41EA40)
| Scene Offset | Mesh File | Used By |
|-------------|-----------|---------|
| `+0x4378` | `Levels\Level5-Bridge` (MeshWorld) | BRIDGE collision mesh |
| `+0x437C` | (CollisionLevel from Bridge) | Bridge collision |

### `LevelBoard_Beginner_ctor` (0x4200E0) — Beginner Race (Arena, internal name: Cascade)
| Scene Offset | Content | Used By |
|-------------|---------|---------|
| `+0x436C` | Vec3List array (0x418 bytes, 8 items) | Bumper creation data |

### `LevelBoard_WarmUp_ctor` (0x41CA40) — Warm-Up
**No sub-meshes loaded.** All scene mesh pointer offsets (`+0x4394`, `+0x5840`, `+0x607C`, etc.) are NULL/zero from the base `Board_ctor`.

---

## How to Add Level-Locked Objects to Any Level

### For self-loading objects (BONK, Bumper, BadBall):

**Step 1:** Patch `app+0x23C` to 1 (for BONK and BadBall) or just add MESHWORLD names (for Bumper — no gate).

**Step 2:** Add named objects to the level's MESHWORLD file section 3:
- For BONK: add objects named `"BONK_01"`, `"BONK_02"`, etc. with their position
- For BadBall: add type-1 objects named `"BADBALL<CHASE>25</CHASE><SIZE>2</SIZE>"`
- For Bumper: add objects named `"N:BUMPER1"`, `"N:BUMPER2"`, etc.

### For scene-dependent objects (TIPPER, GLUEBIE, BLOCKDAWG, CATAPULT):

**Step 1:** Patch `app+0x23C` to 1

**Step 2:** Patch the level's `BoardLevel*_ctor` to load the required sub-mesh files and store the MeshWorld/CollisionLevel pointers at the correct scene offsets. For example, to add TIPPER support to Level 1:
- Patch `LevelBoard_WarmUp_ctor` at `0x41CA40` to add:
  ```
  MeshWorld_ctor(buf, graphics, "Levels\\Level3-Tipper")
  store result → scene+0x4394
  CollisionLevel_ctorWithLevel(buf2, tipper_mesh)
  store result → scene+0x4398
  ```
- This requires injecting code (e.g., at the end of the constructor before the return) that allocates, constructs, and stores the mesh pointers.

**Step 3:** Add TIPPER named objects to Level 1's MESHWORLD section 3 (e.g., `"TIPPER_A"`).

### Alternative: Reimplementation approach

In a reimplementation, bypass both gates entirely:
1. Remove all `app+0x23C != 0` checks from `CreateLevelObjects`
2. Make all constructors self-loading (like `Bonk_ctor` does) — call `MeshWorld_ctor` with the appropriate file path inside each constructor
3. Load sub-meshes on demand rather than only in level constructors

---

## Function Address Quick Reference

| Function | Address | Purpose |
|----------|---------|---------|
| `CreateLevelObjects` | `0x4121D0` | Main factory dispatcher — matches MESHWORLD names to constructors |
| `CreateExpertLevelObjects` | `0x40E250` | Multi-factory: BONK, TIP, SAWBLADE, BRIDGE, JUDGE, BELL sub-dispatcher |
| `Scene_SpawnBallsAndObjects` | `0x41C5B0` | Ball spawning on level load |
| `CreateBadBall` | `0x40BCA0` | Spawn 8-ball AI opponent from MESHWORLD BADBALL tag |
| `CreateMouseTrap` | `0x40BF50` | Spawn mouse trap from MESHWORLD MOUSETRAP objects |
| `Ball_Shatter` | `0x408D70` | Arena 8-ball split: replaces parent ball with 3 AI split balls |
| `Ball_InitBattleMode` | `0x456CD0` | Set ball to battle/arena physics |
| `Bonk_ctor` | `0x438850` | Bonk constructor (self-loads `levels\level5-bonk`) |
| `CreateBumper` | `0x40FA20` | Bumper factory (self-loads `levels\level8`) — ⚠️ REPLACES scene mesh |
| `CreateBumper2` | `0x413CE0` | Arena bumper factory (self-loads `levels\arena-beginner`) |
| `TowerLevel_Ctor` | (in `0x40E250`) | Tower/swinging arm constructor (TIP prefix) |
| `Sawblade_Level_Ctor` | (in `0x40E250`) | Sawblade constructor (SAWBLADE prefix) |
| `Spinner_Level_ctor` | (in `0x40E250`) | Spinner platform constructor (BRIDGE prefix) |
| `Gear_Level_ctor` | (in `0x40E250`) | Gear constructor (JUDGE prefix) |
| `Tipper_Level_Ctor` | (in `0x40E250`) | Bell/Tipper variant constructor (BELL prefix) |
| `TipperVisual_Level_Ctor` | `0x437880` | Mouse trap visual constructor |
| `Tipper_ctor` | `0x437960` | Tipper constructor (needs `scene+0x4394`) |
| `Gluebie_ctor` | `0x437CB0` | Gluebie constructor (needs `scene+0x607C`) |
| `Catapult_ctor` | `0x437E10` | Catapult constructor (needs `scene+0x5848`) |
| `Blockdawg_ctor` | `0x43C310` | BlockDawg constructor (needs `scene+0x5840` + path obj) |
| `BreakBridge_ctor` | `0x436D70` | Breakable bridge constructor |
| `PopCylinder_ctor` | `0x436EE0` | Pop cylinder constructor |
| `CreateSecretObjects` | Unknown | Hidden collectibles (called from `Scene_SpawnBallsAndObjects`) |
| `Scene_CreateFlags` | Unknown | Finish line flags (called from `Scene_SpawnBallsAndObjects`) |
| `Scene_CreateSigns` | Unknown | Directional signs (called from `Scene_SpawnBallsAndObjects`) |
| `Scene_CreateDynamicObjects` | Unknown | Moving platforms, etc. (called from `Scene_SpawnBallsAndObjects`) |
| `Level_FindObjectByName` | `0x460530` | Find MESHWORLD object by name |
| `ExpertCollisionEvents` | `0x40E6A0` | Arena event dispatcher (HAMMERCHASE, ALERTSAW, etc.) |
| `operator_new` | `0x4BA57B` | Game's CRT allocator (`__cdecl`, size on stack) |
| `App singleton` | `0x4FD680` | Global App struct |
| `app+0x23C` | `0x4FD8B4` | Tournament mode flag |

---

*Generated from Ghidra 12.0 decompilation. See `docs/MODDING_FUNCTION_REFERENCE.md` for full function details and `docs/OBJECT_CATALOG.md` for object listing.*