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
| **Tipper** | `"TIPPER"` | `Tipper_ctor` | `0x437960` | `+0x4394` (mesh), `+0x4398` (visual) | `Levels\Level3-Tipper` | `BoardLevel3_ctor` (`0x41D060`) | `app+0x23C != 0` |
| **Gluebie** | `"GLUEBIE"` | `Gluebie_ctor` | `0x437CB0` | `+0x607C` (mesh) | `Levels\Level3-Gluebie` | `BoardLevel3_ctor` (`0x41D060`) | `app+0x23C != 0` |
| **BlockDawg1** | `"BLOCKDAWG1"` | `Blockdawg_ctor` | `0x43C310` | `+0x5840` (mesh) + `"DAWGPATH1"` named object | (dawg sub-mesh) | Level ctors | `app+0x23C != 0` |
| **BlockDawg2** | `"BLOCKDAWG2"` | `Blockdawg_ctor` | `0x43C310` | `+0x5844` (mesh) + `"DAWGPATH2"` named object | (dawg sub-mesh) | Level ctors | `app+0x23C != 0` |
| **Catapult** | `"CATAPULT"` | `Catapult_ctor` | `0x437E10` | `+0x5848` (mesh) | (catapult sub-mesh) | Level ctors | None |

---

## Spawning Self-Contained Objects (No MESHWORLD Edit Needed)

### BadBalls (8-balls / AI opponents)

BadBalls are spawned by `CreateBadBall` which is called from `Scene_SpawnBallsAndObjects`. They do NOT go through `CreateLevelObjects` — they are spawned based on MESHWORLD section 1 object names matching the pattern `BADBALL<tag>val</tag>`.

**`CreateBadBall`** — Address: `0x0040BCA0`
- **Convention:** `__thiscall` (ECX = Scene\*)
- **Parameters:** `this` (Scene\*), `ball_index` (int), `meshworld_obj_index` (int)
- **MESHWORLD tag format:**
  ```
  BADBALL<CHASE>25.0</CHASE><HOME>5.0</HOME><SIZE>2.0</SIZE>
  ```
  | Tag | Default | Offset in Ball | Description |
  |-----|---------|----------------|-------------|
  | `<CHASE>val</CHASE>` | 25.0 | `ball+0xC74` | AI chase activation distance |
  | `<HOME>val</HOME>` | 5.0 | `ball+0xC78` | AI patrol/home radius |
  | `<SIZE>val</SIZE>` | 1.0 | `ball+0x188` | Radius multiplier (3.0 = giant 8-ball) |

- **Spawning logic (from `Scene_SpawnBallsAndObjects` at `0x41C5B0`):**
  1. Iterate all objects in MESHWORLD section 3
  2. For each object with `type == 1` (ball): search for `BADBALL` in its name string
  3. Parse `<CHASE>`, `<HOME>`, `<SIZE>` tags from the name
  4. Call `Ball_ctor2` (`0x408D10`, alloc 0xC98 bytes for 8ball vs 0xC60 for player)
  5. Set `ball+0x31D = 1` (is_8ball AI flag)
  6. Set `ball+0xC60 = 5` (split_ball / battle mode type)
  7. Copy collision direction from parent ball at `ball+0xCA4`
  8. Add to `scene+0x29D4` (bad_balls list) and `scene+0x2DEC` (all_balls list)

- **To add BadBalls via MESHWORLD:** Add objects of type 1 in section 3 with names like `BADBALL<CHASE>30</CHASE><SIZE>3</SIZE>`. They will be spawned automatically on level load when `app+0x23C != 0` (tournament mode).

### MouseTraps

MouseTraps are spawned by `CreateMouseTrap` based on MESHWORLD object names starting with `"N:MOUSETRAP"`.

**`CreateMouseTrap`** — Address: `0x0040BF50`
- **Convention:** `__thiscall` (ECX = Scene\*)
- **Parameters:** `this` (Scene\*), `position` (float[3]), `mesh_ref` (int)
- **Called from:** `CreateLevelObjects` at `0x4121D0` (match: `__strnicmp(name, "MOUSETRAP", 9)`)
- **Size:** Allocates 0x1104 bytes
- **Constructor:** `TipperVisual_Level_Ctor` at `0x437880`

- **Internal constructor:** `TipperVisual_Level_Ctor(this, mesh_ref)` where `mesh_ref` comes from the level constructor. The mouse trap is a specialized Tipper variant that uses the same scene mesh dependency — it needs a pre-loaded mesh at the offset provided by the level constructor that created it.

**NOTE:** MouseTraps are NOT gated by `app+0x23C`. However, they still reference a scene mesh pointer. In `CreateLevelObjects`, the match is `__strnicmp(param_1, "MOUSETRAP", 9)` — this checks the MESHWORLD object name. The constructor receives a mesh reference parameter.

### Sawblades

Sawblades are created by Arena-specific event handlers, not directly from MESHWORLD names.

**`CreateSawblade`** — Address: `0x0040E250`
- **Convention:** `__thiscall` (ECX = Scene\*)
- **Called from:** `Arena_HandleCollision` at `0x40E6A0` when event `E:ACTIVATESAW1` or `E:ACTIVATESAW2` fires
- **Storage:** `scene+0x4370` (saw1), `scene+0x4374` (saw2)
- **Size:** 0x10E0 bytes
- **Gate:** `app+0x23C != 0` (tournament mode only)

Sawblades are triggered by collision events (`E:ALERTSAW1` → `E:ACTIVATESAW1`). To add sawblades to a level, you need:
1. MESHWORLD geometry named to trigger `E:ALERTSAW1` / `E:ACTIVATESAW1` event tags
2. `app+0x23C != 0` (tournament mode)
3. The arena board constructor to have initialized the saw storage at `scene+0x4370/0x4374`

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

### Bumpers

**`CreateBumper`** — Address: `0x0040FA20`
- **Self-loads:** `levels\level8`
- **Size:** 0x10D0 bytes
- **Gate:** None (no `app+0x23C` check)

**`CreateBumper2`** — Address: `0x00413CE0`
- **Self-loads:** `levels\arena-beginner`
- **Size:** 0x10D0 bytes
- **Gate:** None

Bumpers are the freest objects — no gates at all. Match names are `"N:BUMPER%d"` format. They load their own mesh and collision data.

### Ball Splits (8-ball split into 3)

**`Ball_SplitIntoThree`** — Address: `0x00408D70`
- **Convention:** `__thiscall` (ECX = Scene\*)
- **Parameters:** `this` (Scene\*), `parent_ball` (Ball\*)
- **Creates:** 3 split balls (IDs 1, 2, 4) from parent ball
- **Per split ball:**
  - `ball+0x31D = 1` (AI flag)
  - `ball+0xC60 = 5` (split ball type)
  - `ball+0x2E8 = 1` (splitting flag)
  - `ball+0x318 = 30.0` (split timer countdown)
  - Collision direction copied from parent at `ball+0xCA4/CA8/CAC`
  - Added to `scene+0x29D4` (bad_balls) and `scene+0x2DEC` (all_balls)

### Ball Init Battle Mode

**`Ball_InitBattleMode`** — Address: `0x00456CD0`
- **Convention:** `__thiscall` (ECX = Ball\*)
- **Parameters:** `this` (Ball\*)
- **Sets ball to battle/arena physics:**

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

### `BoardLevel3_ctor` (0x41D060) — Dizzy Race
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

### `BoardLevel8_Expert_ctor` (0x41EA40)
| Scene Offset | Mesh File | Used By |
|-------------|-----------|---------|
| `+0x4378` | `Levels\Level5-Bridge` (MeshWorld) | BRIDGE collision mesh |
| `+0x437C` | (CollisionLevel from Bridge) | Bridge collision |

### `BoardLevel_Beginner_Ctor` (0x4200E0) — Cascade Race (Arena)
| Scene Offset | Content | Used By |
|-------------|---------|---------|
| `+0x436C` | Vec3List array (0x418 bytes, 8 items) | Bumper creation data |

### `BoardLevel1_WarmUp_ctor` (0x41CA40) — Warm-Up
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
- Patch `BoardLevel1_WarmUp_ctor` at `0x41CA40` to add:
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
| `Scene_SpawnBallsAndObjects` | `0x41C5B0` | Ball spawning on level load |
| `CreateBadBall` | `0x40BCA0` | Spawn 8-ball AI opponent from MESHWORLD BADBALL tag |
| `Ball_SplitIntoThree` | `0x408D70` | Split ball into 3 AI balls |
| `Ball_InitBattleMode` | `0x456CD0` | Set ball to battle/arena physics |
| `Bonk_ctor` | `0x438850` | Bonk constructor (self-loads `levels\level5-bonk`) |
| `CreateBumper` | `0x40FA20` | Bumper factory (self-loads `levels\level8`) |
| `CreateBumper2` | `0x413CE0` | Arena bumper factory (self-loads `levels\arena-beginner`) |
| `CreateSawblade` | `0x40E250` | Sawblade factory (arena event-triggered) |
| `CreateMouseTrap` | `0x40BF50` | Mouse trap factory |
| `Tipper_ctor` | `0x437960` | Tipper constructor (needs `scene+0x4394`) |
| `TipperVisual_Level_Ctor` | `0x437880` | Mouse trap visual constructor |
| `Gluebie_ctor` | `0x437CB0` | Gluebie constructor (needs `scene+0x607C`) |
| `Catapult_ctor` | `0x437E10` | Catapult constructor (needs `scene+0x5848`) |
| `Blockdawg_ctor` | `0x43C310` | BlockDawg constructor (needs `scene+0x5840` + path obj) |
| `BreakBridge_ctor` | `0x436D70` | Breakable bridge constructor |
| `PopCylinder_ctor` | `0x436EE0` | Pop cylinder constructor |
| `Level_FindObjectByName` | `0x460530` | Find MESHWORLD object by name |
| `Arena_HandleCollision` | `0x40E6A0` | Arena event dispatcher (HAMMERCHASE, ALERTSAW, etc.) |
| `App singleton` | `0x4FD680` | Global App struct |
| `app+0x23C` | `0x4FD8B4` | Tournament mode flag |

---

*Generated from Ghidra 12.0 decompilation. See `docs/MODDING_FUNCTION_REFERENCE.md` for full function details and `docs/OBJECT_CATALOG.md` for object listing.*