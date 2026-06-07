# Hamsterball Modding Function Reference

A comprehensive reference of the most useful functions for modders, extracted from Ghidra decompilation.

---

## Table of Contents

1. [Game Loop & Frame Control](#1-game-loop--frame-control)
2. [Ball Physics & Movement](#2-ball-physics--movement)
3. [Input System](#3-input-system)
4. [Camera System](#4-camera-system)
5. [Scene & Object Management](#5-scene--object-management)
6. [Object Spawning & Creation](#6-object-spawning--creation)
7. [Collision Event System](#7-collision-event-system)
8. [Rendering Pipeline](#8-rendering-pipeline)
9. [Collision & Pathfinding](#9-collision--pathfinding)
10. [App & System Initialization](#10-app--system-initialization)
11. [Vtables & Calling Conventions](#11-vtables--calling-conventions)
12. [Quick Reference: Most-Used Offsets](#quick-reference-most-used-offsets)

---

## 1. Game Loop & Frame Control

### `App_Run`
- **Address:** `0x0046BD80`
- **Convention:** `__fastcall` (ECX = `param_1`)
- **Parameters:**
  - `param_1` (int\*): Pointer to `App` instance (`g_App` at `0x4FD680`)
- **Description:**
  Main game loop. Runs until `App+0x159` (quit flag) is set. Every frame: Win32 message pump → `App.Update` (vtable `0x20`) → `App.Render` (vtable `0x28`) → `Graphics_PresentOrEnd`.
- **Modding use:** Hook vtable slot `0x20` (Update) or `0x28` (Render) to inject custom per-frame logic.

---

### `Scene_Update`
- **Address:** `0x00419C00`
- **Convention:** `__thiscall` (ECX = `this`)
- **Parameters:**
  - `this` (void\*): `Scene*` instance
- **Description:**
  Central game tick. Order: frame counter++ → demo timer → ESC check → ball propagation → gear path → rumble timers → camera shake decay → object update/render → physics pipeline (4 vtable calls).
- **Key offsets:**
  - `this+0x022E` = `scene_objects` (AthenaList)
  - `this+0x0A6C` = `ball_propagate_flag`
  - `this+0x0A75` = `ball_list` (AthenaList\<Ball*\>)
  - `this+0x0D88` = `frame_counter`
- **Modding use:** Hook object vtable slot `4` (Update) for per-object behavior.

---

## 2. Ball Physics & Movement

### `Ball_Update`
- **Address:** `0x00405E00`
- **Convention:** `__fastcall` (ECX = `param_1`)
- **Parameters:** `param_1` (int\*): `Ball*` instance
- **Description:**
  Per-frame update. Returns early if `ball+0x324` (dead). Clears flags, runs AI if `ball+0x31D` (is_8ball) or `scene+0x237` (battle mode), then calls physics. After physics: proximity checks, trail recording, falling state machine.
- **Key offsets:**
  - `ball+0x324` = dead/eliminated flag
  - `ball+0x31D` = `is_8ball` / AI enable flag
  - `ball+0xC60..0xC94` = AI params (home_pos, chase_distance, npc_flag)

---

### `Ball_GetInputForce`
- **Address:** `0x46EC30`
- **Parameters:** `this` (InputDevice\*), `output` (float\*) → `[force_x, force_y]`
- **Description:** Converts input device state to 2D force vector. 4 modes: keyboard (DIK codes), mouse (screen offset), joystick (axes).
- **Key offsets:** `this+0x8` = input mode, key state at `+0x50C/0x510/0x514/0x518`

---

### `Ball_AdvancePositionOrCollision`
- **Address:** `0x4564C0`
- **Parameters:** `this` (Ball\*), `out_pos`, `cur_pos`, `input_vel`, `collision_flags`, `dt`
- **Description:** Core physics step. Velocity integration → damp → gravity → `TestSphereVsLevel` collision → bounce → trail. Key: `ball+0xC68` = damping, `ball+0xC70` = max velocity, `ball+0xC88..C94` = gravity.

---

### `Ball_SetTargetPos`
- **Address:** `0x00402030`
- **Parameters:** `this` (Ball\*), `x`, `y`, `z` (floats)
- **Description:** Sets `ball+0x758/75C/760` — camera orbit center point. NOT physics position, it's the smoothed display position the camera tracks.

---

## 3. Input System

### `Input_IsKeyDown`
- **Address:** `0x46E0B0`
- **Parameters:** `key` (int) — DIK scancode
- **Returns:** 1 if held, 0 if not
- **Modding use:** Poll any key for custom actions.

### `App_CreateInputDevice`
- **Address:** `0x0046C050`
- **Returns:** `InputDevice*` (0x14 bytes)
- **Key offsets:** `+0x00` vtable, `+0x04` App\*, `+0x08` input mode, `+0x10` joystick ptr

---

## 4. Camera System

### `Scene_SetCamera`
- **Address:** `0x00419FA0`
- **Convention:** `__thiscall` (ECX = `this`)
- **Parameters:**
  - `this` (void\*): `Scene*` instance
  - `param_1` (void\*): `Ball*` being tracked
  - `param_2` (char): Boolean — if true, apply path-following spring
- **Description:**
  Camera positioning every frame. 5 modes:
  
  1. **Default Follow:** Orbit around `ball+0x758` + `scene+0x434C` offset
  2. **Path Rail:** If `scene+0x3F1C != 0` AND `param_2 != 0`:
     - Call `Path_GetPosition(scene+0x3F20, &out, scene+0x3F24)`
     - Compute spring force toward nearest rail point with sine wobble
     - Clamp max distance to 700 units
     - **To DISABLE path follow:** Set `scene+0x3F1C = 0` — camera falls back to default follow
  3. **Camera Shake:** If `ball+0x744`: random ±50 per axis
  4. **Snap:** If `scene+0x3F2C > 0`: force camera to ball physics pos, decrement counter
  5. **Orbit:** Apply `scene+0x29BC` (angle) and `scene+0x29C0` (distance)
  
  After all modes, calls `Ball_SetTargetPos` and updates `ball+0x76C`.
- **Key offsets:**
  - `scene+0x3F1C` = `path_follow_mode` (0=follow ball, 1=follow rail path)
  - `scene+0x3F20` = `path_object` (Path\* for camera rail spline)
  - `scene+0x3F24` = `path_position` (float, 0.0–1.0 parametric)
  - `scene+0x3F2C` = `camera_snap_frames`
  - `scene+0x29BC` = `camera_orbit_angle`
  - `scene+0x29C0` = `camera_distance` (max 700)
  - `scene+0x434C` = `camera_offset` (Vec3)

---

## 5. Scene & Object Management

### `Scene_AddObject`
- **Address:** `0x00469990`
- **Parameters:** `this` (Scene\*), `obj` (Gadget\*)
- **Description:** Appends to `scene+0x2578` (active object list). Objects receive `Update` (vtable[4]) and `Render` (vtable[0]) per frame.

### `Board_ctor`
- **Address:** `0x00419030`
- **Description:** Creates Board (~0x5400 bytes, inherits Gadget). Sets up scene lists, ball list, D3D device, level slots.

---

## 6. Object Spawning & Creation

### `Scene_SpawnBallsAndObjects`
- **Address:** `0x0041C5B0`
- **Convention:** `__fastcall` (param_1 = Scene\*)
- **Parameters:** `param_1` (Scene\*) — the board/scene instance
- **Description:**
  Level startup factory. Creates all game objects in order:
  
  **1. Ball creation loop** — For each start entry:
  - Look up `"START%d-%d"` position in hash table
  - Tournament mode: alternate start positions
  - Race types 5, 0xB, 0xC, 0xE: random start between `"START2-1"`/`"START2-2"`
  - Debug override: `"START-DEBUG"` if present
  - `Ball_ctor2(new(0xC60), scene)` → `vtable[4]()` init → `Ball_SetTrajectory()` → set properties
  
  **2. Safe spot scan** — Parse for `"SAFESPOT"`/`"SAFEPOS"` → `scene+0x546`
  
  **3. Tournament/demo objects** (if `app+0x23C` or `app+0x237`):
  - `CreateBadBall(scene)` — AI opponent balls
  - `CreateMouseTrap(scene)` — mouse trap obstacles
  
  **4. Level decoration:**
  - `CreateSecretObjects(scene)` — hidden collectibles
  - `Scene_CreateFlags(scene)` — finish line flags
  - `Scene_CreateSigns(scene)` — direction signs
  - `Scene_CreateDynamicObjects(scene)` — moving/animated objects

- **Ball spawn constants:**
  - Allocation size: `0xC60` bytes
  - Default radius: `26.0` (not 27)
  - Max speed: `5.0` (0x40A00000)
  - Speed scale: `0.1` (0x3DCCCCCD)
  - Gravity mult: `1.00625` (0x3F866666)
  - Gravity scale: `0.5`

---

### `CreateBadBall`
- **Address:** `0x0040BCA0`
- **Convention:** `__fastcall` (param_1 = Scene\*)
- **Parameters:** `param_1` (int): `Scene*` instance
- **Description:**
  Scans the level meshworld string table for entries starting with `"BADBALL"`. For each match:
  1. Allocate `Ball_ctor(new(0xC98), scene)` — note: 0xC98, larger than normal ball (0xC60)!
  2. Call `vtable[4]()` — post-constructor init
  3. Set position from meshworld object data: `ball+0x164` = X + offset, `ball+0x168` = Y + display_pos, `ball+0x16C` = Z + offset
  4. Copy home position: `ball+0xC60` = X, `ball+0xC64` = Y, `ball+0xC68` = Z
  5. Set `ball+0x281 = 0` (clear active flag)
  6. Parse XML-style tags from the BADBALL name:
     - `<CHASE>value</CHASE>` → `ball+0xC6C` (chase distance)
     - `<HOME>value</HOME>` → `ball+0xC70` (home radius)
     - `<SIZE>value</SIZE>` → `ball+0x188 = 3.0` (0x40400000), `ball+0xA0` = value, and set `ball+0xC4C = 1` (dizzy-resistant)
     - `<SPINDISTANCE>value</SPINDISTANCE>` → `ball+0x1B4` = distance for spin tracking
  7. Append to `scene+0x29D4` (8ball list) and `scene+0x2DEC` (active ball list)
  
- **BADBALL MESHWORLD name format:**
  ```
  BADBALL<CHASE>200</CHASE><HOME>500</HOME><SIZE>30</SIZE>
  ```
  All tags are optional. Without SIZE, ball uses default radius (26.0).

---

### `Ball_SplitIntoThree`
- **Address:** `0x00408D70`
- **Convention:** `__thiscall` (this = Ball\* being split)
- **Parameters:**
  - `this` (void\*): The ball being split (must have `ball+0x324 == 0`, i.e. not dead)
  - `param_1` (int): Collision parameter list (positions for split trajectories)
- **Description:**
  Creates 3 split balls from the current ball (used in 8-ball battle mode):
  1. Free collision direction buffer at `ball+0xC28`
  2. Call `vtable[0x78]()` — pre-split callback
  3. Set `ball+0x2E8 = 1` (splitting flag)
  4. If `ball+0x744 == 0` (no gravity plane): play sound based on `ball+0xC4C` (dizzy flag)
  5. Call `Scene_ForEachBall_SetVelocity` — propagate current velocity to all balls
  6. Loop 3 times (for split ball IDs 1, 2, 4):
     - `Ball_Split_ctor(new(0xC64), scene)` — allocate 0xC64 bytes
     - Set `ball+0x324 = 0xC9` (split ball marker, 1 byte)
     - Call `vtable[4]()` — init
     - Copy collision direction from parent (`ball+0xCA4/CA8/CAC`)
     - Set position: either from collision param list or from parent display_pos
     - Call `Ball_SetTrajectory` with parent trajectory data
     - Set `ball+0x18 = -1` (no player index — AI ball)
     - Set `ball+0x274 = 0.01` (speed scale), `ball+0x27C = 0.5` (force multiplier)
     - Copy parent radius to `ball+0xA0`
     - Call `vtable[0x14]()` — post-init
     - Set `ball+0x758` (split ball ID): 1, 2, or 4
     - Append to `scene+0x3204` (split ball list)
     - Set `ball+0x31D = 1` (AI enable flag — split balls ARE AI-controlled!)
     - Set `ball+0x318 = 30.0` (0x41200000) — split timer
     - Add random trajectory variation (`RNG_Rand` 5, 10)
     - Set `ball+0x324 = split_id` (1, 2, or 4)

- **Key offsets for split balls:**
  - `ball+0x31D` = `is_8ball` flag (always 1 for splits)
  - `ball+0x318` = split timer (starts at 30.0)
  - `ball+0x758` = split ball ID (1, 2, or 4)
  - `ball+0x274` = speed scale (0.01)
  - `ball+0x27C` = force multiplier (0.5)

---

### `Ball_InitBattleMode`
- **Address:** `0x00456CD0`
- **Convention:** `__fastcall` (param_1 = Ball\*)
- **Parameters:** `param_1` (void\*): Ball instance to convert to battle mode
- **Description:**
  Converts a normal ball to battle/8ball mode. Sets physics parameters:
  - `ball+0xC60 = 3` (battle mode flag — overrides default 5)
  - `ball+0xC68 = 0.55` (0x3F0E147B — friction higher than normal)
  - `ball+0xC6C = 1.0` (0x3F800000 — bounciness)
  - `ball+0xC70 = 1000.0` (0x447A0000 — max speed, much higher than normal 5.0!)
  - `ball+0xC78 = 25.0` (0x41C80000 — chase distance for AI)
  - `ball+0xC7C = 1` (AI active flag)
  - Clear velocity: `ball+0xC80/84/88 = 0`
  - Set gravity: `ball+0xC8C = 0`, `ball+0xC90 = -1.0`, `ball+0xC94 = 0`
  - Set `ball+0x14 = 1` (trail recording enabled)
- **Modding use:** Call on any ball to turn it into a battle-mode AI opponent. The 1000x speed increase (1000 vs 5) and friction change make battle balls behave very differently.

---

### `CreateLevelObjects` (Object Factory)
- **Address:** `0x004121D0`
- **Convention:** `__thiscall` (ECX = `this`)
- **Parameters:**
  - `this` (void\*): `Scene*` instance
  - `name` (char\*): Object name from MESHWORLD (e.g., `"BONK_01"`, `"TIPPER_A"`)
  - `param_2` (undefined4\*): Transform matrix pointer
  - `param_3` (undefined4\*): Secondary transform
  - `param_4` (int): Object index
- **Description:**
  The main object factory. Uses `__strnicmp` to match object name prefixes and instantiate the appropriate game object:
  - `BRIDGE` (6 chars) → configure bridge collision mesh at `scene+0x436C`, position at `scene+0x437C/80/84`. If name contains `"(NOCOLLIDE)"`, skip collision at `scene+0x4370`
  - `TIPPER` (6 chars, if `app+0x23C != 0`) → `Tipper_ctor(0x1104)` + `TipperVisual_ctor(0x10D0)`, append visual to `scene+0x2578`
  - `BONK` (4 chars, if `app+0x23C != 0`) → `Bonk_ctor(0x1200)`, store at `scene+0x540C`
  - `BBRIDGE1`/`BBRIDGE2` (8 chars) → `BreakBridge_ctor(0x1100)`, store mesh at `scene+0x5410`/`0x5414`, obj at `scene+0x5418`/`0x541C`
  - `POPCYLINDER` (11 chars) → `PopCylinder_ctor(0x10E8)`, append to `scene+0x5428`
  - `BLOCKDAWG1`/`BLOCKDAWG2` (10 chars, if `app+0x23C != 0`) → `Blockdawg_ctor(0x1154)`, locates path via `Level_FindObjectByName("DAWGPATH1"/"DAWGPATH2")`. DWG2 sets `obj+0x1152 = 1` (variant 2)
  - `CATAPULT` (8 chars) → `Catapult_ctor(0x1108)`, set `obj+0x440*4 = 1` (active), append to `scene+0x584C`
  - `GLUEBIE` (7 chars, if `app+0x23C != 0`) → `Gluebie_ctor(0x110C)`, append to `scene+0x6080`
  - `SAFESPOT`/`SAFEPOS` → safety spawn positions (not game objects)
  - `START1-1`, `START2-1` etc. → ball spawn positions
  - `E:LIMIT`, `E:JUMP`, etc. → event triggers (collision volumes)
  - `BADBALL` → 8-ball AI parameters (`CHASE`, `HOME`, `SIZE`, `SPINDISTANCE`)

- **Scene object list offsets (quick lookup by type):**
  - `scene+0x2578` = all game objects (AthenaList)
  - `scene+0x436C` = bridge_mesh
  - `scene+0x540C` = bonk_ref
  - `scene+0x5410/5418` = breakbridge1 mesh/obj
  - `scene+0x5414/541C` = breakbridge2 mesh/obj
  - `scene+0x5420/5428` = popcylinder mesh/list
  - `scene+0x5840/5844` = blockdawg1/2 mesh
  - `scene+0x5848/584C` = catapult mesh/list
  - `scene+0x607C/6080` = gluebie mesh/list

---

### `CreateBumper`
- **Address:** `0x0040FA20`
- **Convention:** `__fastcall` (param_1 = Scene\*)
- **Parameters:** `param_1` (int\*): `Scene*` instance
- **Description:**
  Creates 8 bumper objects for Level 8 (tournament/arena). Loads `"levels\\level8"` as `MeshWorld`, creates `CollisionLevel` clone, calls `Scene_CollectByNameFilter("N:BUMPER%d")` 8 times. Stores at `scene+0x1913` area blocks. Calls `vtable[0x80]()` post-init.

### `CreateBumper2`
- **Address:** `0x00413CE0`
- **Convention:** `__fastcall` (param_1 = Scene\*)
- **Description:**
  Variant for arena levels. Loads `"levels\\arena-beginner"`, creates 4 bumpers instead of 8. Same pattern but offset to `scene+0x1610` block area.

### `CreateSawblade` (inside `CreateLevelObjects`)
- **Address:** `0x0040E250` (within factory dispatcher)
- **Convention:** Part of `CreateLevelObjects` dispatch chain
- **Parameters:** Same as `CreateLevelObjects` — `this`, `name`, `param_2`, `param_3`, `param_4`
- **Description:**
  Matched by `__strnicmp(name, "SAWBLADE", 8)`. If `app+0x23C != 0`:
  - `Sawblade_Level_Ctor(new(0x111C), scene, x, y, z)` — position from `param_4+4/8/C`
  - Append to `scene+0x2578`
  - If name contains `"1"`: store at `scene+0x4370`, call `Sawblade_SetBreakSound(1)`
  - If name contains `"2"`: store at `scene+0x4374`

### `CreateMouseTrap`
- **Address:** `0x0040BF50`
- **Convention:** `__fastcall` (param_1 = Scene\*)
- **Description:**
  Scans meshworld string table for `"MOUSETRAP"` entries. For each:
  - `TipperVisual_Level_Ctor(new(0x10F8), scene)` — 0x10F8 bytes
  - Append to `scene+0xCD4` (collision list), `scene+0x1930` (render list), mesh data list
  - Set position from meshworld object data (`+0x437/438/439`)
  - Set Z offset: `obj+0x43D = _DAT_004CF44C - obj_Z_value`

### `CreateBonkPopup`
- **Address:** `0x00438B30`
- **Convention:** `__fastcall` (param_1 = hammer object ptr)
- **Description:**
  If `obj+0x10FC != 0` (active): clear flag to 0, play 3D sound at stored position, call `vtable[0x88]("BONKPOPUP")`. Simple audio-visual feedback for hammer hits.

### `Hammer_ChaseStart`
- **Address:** `0x00438BB0`
- **Convention:** `__fastcall` (param_1 = hammer object ptr)
- **Description:**
  If `obj+0x10FD == 0` (not already chasing): set `obj+0x10FD = 1`, set `obj+0x1104 = 1` (chase active), copy position from `obj+0x10D4/10D8/10DC` to `obj+0x1108/110C/1110`, copy target from `obj+0x10E0/10E4/10E8` to `obj+0x1120/1124/1128`. Set `obj+0x1138 = 0.5` (0x3F000000 — chase speed).

### `Ball_ctor` / `Ball_ctor2`
- **Address:** `Ball_ctor` = inside `Ball_ctor2` chain, initial allocation at various addresses
- **Description:**
  `Ball_ctor(this, scene)` allocates and initializes a Ball struct:
  - Calls `GameObject_ctor(this, scene)` base constructor
  - Sets vtable to `0x4CF3A0` (Ball vtable)
  - Initializes physics vectors at `ball+0xC84..0xC94` (gravity default: identity quaternion → 0,0,1,0,0,1)
  - `ball+0xC80 = 0` (gravity mode 0 = flat)

  After construction, the spawner calls `vtable[4]()` for post-init and `Ball_SetTrajectory()` with level data.

---

### Additional Object Factories

#### `CreateSecretObjects`
- **Address:** `0x0040BAA0`
- **Convention:** `__fastcall` (param_1 = Scene\*)
- **Description:**
  Scans meshworld string table for objects starting with `"SECRET"` or `"SECRETUNLOCK"`. Creates hidden collectible objects that trigger `Rotator_MarkTriggered` (for `N:SECRET`) or `CheckArenaUnlock` (for `N:UNLOCKSECRET`). Only active if `app+0x23C != 0` and `app+0x234 == 0` and not in demo mode.

#### `Scene_CreateFlags`
- **Address:** `0x0040C0F0`
- **Convention:** `__fastcall` (param_1 = Scene\*)
- **Description:**
  Scans for `"FLAG"` and `"SMALLFLAG"` entries in meshworld. Creates `Flag` objects (0x8C bytes) appended to `scene+0x2160`. `SMALLFLAG` objects get their size scaled down by a constant factor.

#### `Scene_CreateSigns`
- **Address:** `0x0040C270`
- **Convention:** `__fastcall` (param_1 = Scene\*)
- **Description:**
  Scans for `"SIGN"` entries. Creates `Sign` objects (0x10FC bytes). If name contains `"-TARPIT"`, applies tar texture. Appends to `scene+0xCD4` and render lists.

#### `Scene_CreateDynamicObjects`
- **Address:** `0x0040C430`
- **Convention:** `__fastcall` (param_1 = Scene\*)
- **Description:**
  Iterates ALL meshworld entries. For each, calls `scene->vtable[0x84](name, &result, &count, entry)` — the virtual `CreateDynamicObject` method that each board level overrides. Results are appended to `scene+0x335` (dynamic object list) and sub-lists at `scene+0x43B`.

---

## 7. Collision Event System

### `CreateNoDizzy` / `GameObject_HandleCollision`
- **Address:** `0x0040C5D0`
- **Convention:** `__thiscall` (ECX = this, params = Scene\*, Ball\*, CollisionObject\*)
- **Parameters:**
  - `this` (void\*): Level/Board instance
  - `param_1` (int\*): `Ball*` instance
  - `param_2` (int\*): `CollisionObject*` — contains event name at `collObj[1]+0x864`
- **Description:**
  **THE** universal event dispatcher. Every collision in the game ends up here. Dispatches by `__strnicmp` on the event name string:
  
  | Event Name | Condition | Action |
  |------------|-----------|--------|
  | `N:SECRET` | — | `Rotator_MarkTriggered(collObj+0x47C)` |
  | `N:UNLOCKSECRET` | — | `CheckArenaUnlock(scene)` |
  | `E:NODIZZY<TIME>N</TIME>` | — | Parse TIME tag → `Ball_RecordBest(ball, duration)` |
  | `E:SAFESWITCH(data)` | — | Copy `(data)` to `ball+0xC2C`; if no parens: clear `ball+0x30B` |
  | `E:LIMIT` | — | Clear `ball+0x1DA`, set `ball+0x2E9=1`, increment per-player completions at `board+0x47B4..47C0` |
  | `E:BREAK` | — | Call `ball->vtable[0x20]()` (bounce callback) |
  | `E:JUMP` | `ball.impactCounter < 1` | Play 3D jump sound, set impactCounter=10, apply upward force 0.025, freeze 10 frames, `Ball_RecordBest(+200)` |
  | `E:ACTION<ONCE>TRUE</ONCE><SCORE>N</SCORE>` | — | If ONCE: check `ball+0xCB` for duplicate; award score with difficulty modifier |
  | `E:TRAJECTORY<X>val</X><Y>val</Y><Z>val</Z>` | — | Parse X/Y/Z tags → set ball collision direction at `ball+0xCA4/CA8/CAC` |
  | `N:NOCONTROL` | — | Set `ball+0x202 = 10` (disable input for 10 frames) |
  | `N:WATER` | — | Set `ball+0x2D5 = 1` (water flag), `ball+0xB6 = 10` (water timer) |
  | `N:TARPIT` | `ball+0xB3 == 0` (first time) | Play tar sound, set `ball+0xB3 = 1`, clear `ball+0x1DA` |
  | `DROPIN` | `dist > threshold` | Play dropin sound, `ball.dropinCounter = 50`, `Ball_RecordBest(+200)` |
  | `PIPEBONK` | `pipebonkCounter < 1` | Random sound from 3, `counter = 10`, `Ball_RecordBest(+100)` |
  | `POPOUT` | `popoutCounter < 1` | Play popout sound, `counter = 50`, `Ball_RecordBest(+100)` |
  | `N:GOAL` | `!ball.finished && ball.active` | First ball: set `board->goalReached = 1`, play goal music. Mark player finished, record time |
  | `N:MOUSETRAP` | — | Randomize RNG, deflect ball direction × trap speed (0x4CF370), search rotator list for match |

### `Level_HandleCollision` (Arena-Specific Events)
- **Address:** `0x40DCD0`
- **Convention:** `__thiscall` (ECX = this)
- **Description:**
  Level-specific collision dispatch, then delegates to `CreateNoDizzy` for all remaining events. Arena-specific events:
  
  | Event Name | Condition | Action |
  |------------|-----------|--------|
  | `E:CATAPULTBOTTOM` | `impactCounter < 1` | Set `ball+0x202 = 1000`, find matching catapult in `scene+0x47C4`, launch, play sound |
  | `E:OPENSESAME` | — | Open all trapdoors via `Trapdoor_Open(list->first)` |
  | `N:TRAPDOOR` | — | Find matching trapdoor in `scene+0x4BDC`, activate |
  | `E:BITE` | — | Set `scene+0x43A8 = 0`, `scene+0x43A0 = 25.0` |
  | `E:MACETRIGGER` | — | Iterate mace list, set each `obj+0x10F0 = 1` (active) |

### `Arena_HandleCollision` (Rumble-Specific Events)
- **Address:** `0x40E6A0`
- **Convention:** `__thiscall` (ECX = this)
- **Description:**
  Rumble arena collision dispatch, then delegates to `CreateNoDizzy`. Arena-specific events:
  
  | Event Name | Condition | Action |
  |------------|-----------|--------|
  | `E:CALLHAMMER` | `app+0x23C != 0` (multiplayer) | `CreateBonkPopup(scene+0x436C)` |
  | `E:HAMMERCHASE` | `app+0x23C != 0` | `Hammer_ChaseStart(scene+0x436C)` |
  | `E:ALERTSAW1`/`E:ALERTSAW2` | `app+0x23C != 0` | `Saw_AlertActivate(scene+0x4370/0x4374)` |
  | `E:ACTIVATESAW1`/`E:ACTIVATESAW2` | `app+0x23C != 0` | `Saw_Activate(scene+0x4370/0x4374)` |
  | `E:ALERTJUDGES` | — | Reset all judge objects in list at `scene+0x4FC8` |
  | `E:SCORE<number>` | — | Parse number, set score display timer for each display in list |
  | `E:BELL` | — | `Bell_Activate`, add 500 time units in single player, show "EXTRA TIME:" popup |
  | `E:JUMP` | `impactCounter < 1` | Same as base: sound, force, `Ball_RecordBest(+200)` |

---

## 8. Rendering Pipeline

### `Scene_Render`
- **Address:** `0x0041A2E0`
- **Convention:** `__thiscall` (ECX = `this`)
- **Parameters:** `this` (Scene\*), `param_1` (int\*): Graphics context
- **Description:**
  Main rendering dispatch. Per viewport: `Graphics_SetViewport` → `Scene_SetCamera` → `vtable[0x60]` (Background) → `vtable[0x64]` (Opaque) → `vtable[0x68]` (Transparent) → `vtable[0x70]` (Overlay) → `vtable[0x6C]` (PostEffects).

### `Scene_RenderAllObjects`
- **Address:** `0x0045E0E0`
- **Parameters:** `this` (Scene\*)
- **Description:** Iterates `scene+0x22E` (AthenaList), calls `obj->vtable[0]()` (Render) for each visible object.

---

## 9. Collision & Pathfinding

### `Scene_CheckPath`
- **Address:** `0x00457EC0`
- **Parameters:** `start` (int), `target` (int)
- **Returns:** `int` — 1 (clockwise), -1 (counter-clockwise), 0 (unreachable)
- **Description:** Ring-topology pathfinder on 360-cell circular grid. Used for angular collision and track snapping.

### `Path_GetPosition`
- **Address:** `0x00467BF0`
- **Convention:** `__thiscall` (ECX = Path\*)
- **Parameters:**
  - `this` (void\*): Path instance (`scene+0x3F20`)
  - `param_1` (float\*): OUT — position [x, y, z]
  - `param_2` (float): Parametric value (0.0–1.0)
- **Description:**
  Interpolates path position using cubic splines. Y is always 0.0 (paths are 2D XZ curves). Used by camera path-follow system in `Scene_SetCamera`.
- **Modding use:** Call with different `param_2` values to trace the entire path rail. Set `scene+0x3F1C = 0` and `scene+0x3F24` manually to force camera to specific path positions.

### `Level_FindObjectByName`
- **Address:** `0x00460530`
- **Parameters:** `this` (Level\*), `name` (char\*)
- **Returns:** `int` — pointer to found object, or 0 if not found
- **Description:** Linear search of meshworld object list by `__stricmp`. Used by `BLOCKDAWG1`/`BLOCKDAWG2` to find their path objects (`"DAWGPATH1"`/`"DAWGPATH2"`).

---

## 10. App & System Initialization

### `App_Initialize_Full`
- **Address:** `0x00429530`
- **Convention:** `__thiscall` (ECX = `this`)
- **Parameters:** `this` (App\*), `param_1`, `param_2` (both unused)
- **Description:**
  Full game init sequence (15+ steps): base init → cursor → graphics → D3D config → shadow texture → music → jukebox → registry → input devices → players → main menu.

---

## 11. Vtables & Calling Conventions

### Ball Vtable (`0x4CF3A0`)
| Slot | Offset | Function | Purpose |
|------|--------|----------|---------|
| 0 | `0x00` | `Render` | Draw ball mesh |
| 1 | `0x04` | `Init` | Post-constructor setup |
| 4 | `0x10` | `Update` | Per-frame update (calls `Ball_Update` at `0x405E00`) |
| 5 | `0x14` | `ApplyForceWithMultipliers` | Force application with scale |
| 7 | `0x1C` | `CollisionHandler` | Custom collision response |
| 30 | `0x78` | `PreSplitCallback` | Called before `Ball_SplitIntoThree` |
| 32 | `0x80` | `PostFactoryInit` | Called by bumper/sawblade factories after creation |

### Scene/Board Vtable
| Slot | Offset | Function | Purpose |
|------|--------|----------|---------|
| 4 | `0x10` | `Update` | Per-frame scene update |
| 8 | `0x20` | `RunFrame` | Full frame (Update+Render) |
| 10 | `0x28` | `Render` | Full render dispatch |
| 12 | `0x30` | `LoadLevel` | Level loading |
| 33 | `0x84` | `CreateDynamicObject` | Override per level — creates level-specific dynamic objects |
| 24 | `0x60` | `RenderBackground` | Sky/far plane |
| 25 | `0x64` | `RenderOpaque` | Opaque geometry |
| 26 | `0x68` | `RenderTransparent` | Glass/effects |
| 28 | `0x70` | `RenderOverlay` | HUD |

### Calling Conventions
- `__thiscall`: ECX = `this` pointer. Used for all object methods.
- `__fastcall`: ECX = first arg, EDX = second arg. Used for some inner loops.
- Standard cdecl: Stack-based args. Used for utility functions.

---

## Quick Reference: Most-Used Offsets

### Ball Struct (`0xC98` bytes for 8ball, `0xC60` for player, vtable `0x4CF3A0`)
| Offset | Type | Name | Description |
|--------|------|------|-------------|
| `+0x00` | void\* | vtable | `0x4CF3A0` |
| `+0x04` | void\* | scene | Parent Scene\* |
| `+0x14` | char | trail_flag | 1 = record trail points |
| `+0x60` | float[3] | position | Physics position (X, Y, Z) |
| `+0x164` | float[3] | display_pos | Smoothed display position |
| `+0x170` | float[3] | velocity | Current velocity |
| `+0x188` | float | radius_scale | 3.0 for SIZE-tagged 8balls |
| `+0x198` | float | facing_angle | Yaw in radians |
| `+0x284` | float | radius | Ball radius (default 26.0 for player, SIZE-tag for 8ball) |
| `+0x2E8` | char | splitting | Set to 1 during `Ball_SplitIntoThree` |
| `+0x2EC` | int | collision_count | Per-frame collision tally |
| `+0x2F0` | int | frame_counter | Frames since spawn |
| `+0x2F9` | char | tar_state | Tarpit / stuck flag |
| `+0x318` | float | split_timer | Countdown for split balls (starts 30.0) |
| `+0x31D` | char | is_8ball | AI enable flag — set to 1 for split balls |
| `+0x324` | char | dead | Eliminated / in-tube flag |
| `+0x748` | int | gravity_plane | 0=flat, 1=tilted, 2=vertical |
| `+0x758` | float[3] | camera_target | Camera orbit center + set by `Ball_SetTargetPos` |
| `+0x76C` | float[3] | camera_actual | Camera actual position (written by `Scene_SetCamera`) |
| `+0xCA4` | float[3] | collision_dir | Last collision direction (copied to split balls) |
| `+0xC60` | int | battle_mode | 3=battle mode, 5=split ball |
| `+0xC68` | float | friction | 0.55 in battle mode |
| `+0xC6C` | float | bounciness | 1.0 in battle mode |
| `+0xC70` | float | max_speed | 5.0 (player), 1000.0 (battle) |
| `+0xC74-0xC78` | float | chase_distance | AI chase radius (25.0 in battle mode) |
| `+0xC80-0xC94` | float[5] | gravity_vec | Gravity vector (battle: 0, -1.0, 0, ...) |
| `+0xCA0` | float | speed_scale | 0.01 for split balls |

### Scene Struct (`~0x5400` bytes)
| Offset | Type | Name | Description |
|--------|------|------|-------------|
| `+0x022E` | AthenaList | objects | All scene objects |
| `+0x0A6C` | char | ball_propagate_flag | 1 = need to propagate ball positions |
| `+0x0A75` | AthenaList | balls | Ball list |
| `+0x0D88` | int | frame_counter | Game frame count |
| `+0x0D8B` | AthenaList | physics_objects | Physics step object list |
| `+0x237` | char | battle_mode | 8-ball / race mode |
| `+0x3F1C` | int | path_follow | Camera spline follow (0=direct follow, 1=rail follow) |
| `+0x3F20` | void\* | path_obj | Path\* for camera rail |
| `+0x3F24` | float | path_position | Parametric position on camera path (0.0–1.0) |
| `+0x3F2C` | int | camera_snap_frames | Countdown frames for instant camera snap |
| `+0x29BC` | float | orbit_angle | Camera orbit rotation |
| `+0x29C0` | float | cam_distance | Orbit distance (max 700) |
| `+0x434C` | float[3] | cam_offset | Added to ball pos for camera |
| `+0x2578` | AthenaList | active_objs | Factory output list |
| `+0x29D4` | AthenaList | bad_balls | 8-ball / AI ball list |
| `+0x2DEC` | AthenaList | all_balls | Combined ball list |

### App Struct (singleton at `0x4FD680`)
| Offset | Type | Name | Description |
|--------|------|------|-------------|
| `+0x159` | char | quit_flag | Set to 1 to exit game loop |
| `+0x234` | char | is_demo | Demo mode flag |
| `+0x237` | char | is_2player | Multiplayer mode |
| `+0x23C` | int | is_tournament | Tournament/arena mode |
| `+0x5FC` | int | input_mode | 1=keyboard, 2=mouse, 4-7=joy |
| `+0x850` | int | num_balls | Number of player balls |

---

## Document Info

- **Generated from:** Ghidra 12.0 decompilation + REST API queries
- **Game version:** Hamsterball.exe (PE32, i386)
- **Total functions renamed:** 1,200+ (see `analysis/ghidra/renames_backup.json`)
- **Decompilation sources:** `analysis/ghidra/decompilations/`
- **Last updated:** 2026-06-07

For full struct definitions, see `analysis/ghidra/structs/*.h`.
For the object catalog, see `docs/OBJECT_CATALOG.md`.
For AI internals, see `docs/8BALL_AI_SYSTEM.md`.