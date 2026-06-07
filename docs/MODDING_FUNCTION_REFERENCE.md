# Hamsterball Modding Function Reference

A comprehensive reference of the most useful functions for modders, extracted from Ghidra decompilation.

---

## Table of Contents

1. [Game Loop & Frame Control](#1-game-loop--frame-control)
2. [Ball Physics & Movement](#2-ball-physics--movement)
3. [Input System](#3-input-system)
4. [Camera System](#4-camera-system)
5. [Scene & Object Management](#5-scene--object-management)
6. [Level Factory & Object Creation](#6-level-factory--object-creation)
7. [Rendering Pipeline](#7-rendering-pipeline)
8. [Collision & Pathfinding](#8-collision--pathfinding)
9. [App & System Initialization](#9-app--system-initialization)
10. [Vtables & Calling Conventions](#10-vtables--calling-conventions)

---

## 1. Game Loop & Frame Control

### `App_Run`
- **Address:** `0x0046BD80`
- **Convention:** `__fastcall` (ECX = `param_1`)
- **Parameters:**
  - `param_1` (int\*): Pointer to `App` instance (`g_App` at `0x4FD680`)
- **Description:**
  The main game loop. Runs until `App+0x159` (quit flag) is set. Every frame:
  1. Calls `PeekMessageA` / `TranslateMessage` / `DispatchMessageA` for Win32 message pump
  2. Calls `App.Update` via vtable offset `0x20`
  3. Calls `App.Render` via vtable offset `0x28`
  4. Calls `Graphics_PresentOrEnd`
- **Key offsets used:**
  - `App+0x5b` = target FPS
  - `App+0x5c` = frame interval (ms)
  - `App+0x5d` = `Graphics*` pointer
  - `App+0x159` = quit flag (set to 1 to exit)
- **Modding use:** Hook vtable slot `0x20` (Update) or `0x28` (Render) to inject custom per-frame logic.

---

### `Scene_Update`
- **Address:** `0x00419C00`
- **Convention:** `__thiscall` (ECX = `this`)
- **Parameters:**
  - `this` (void\*): `Scene*` instance
- **Description:**
  Central game tick called every frame. Execution order:
  1. Increment frame counter (`this+0xD88`)
  2. Demo timer countdown & popup
  3. ESC / menu check (`Input_CheckKeyCombo`)
  4. Ball position propagation (`this+0xA6C` flag)
  5. Gear path following
  6. Rumble timer ticks & camera shake decay
  7. Scene object update + render (calls `obj->vtable[4]()` for each object)
  8. Physics pipeline (4 vtable steps)
- **Key offsets:**
  - `this+0x022E` = `scene_objects` (AthenaList)
  - `this+0x0A6C` = `ball_propagate_flag`
  - `this+0x0A75` = `ball_list` (AthenaList<Ball*>)
  - `this+0x0D88` = `frame_counter`
- **Modding use:** The heart of the game simulation. Hook object vtable slot `4` (Update) to add custom per-object behavior.

---

## 2. Ball Physics & Movement

### `Ball_Update`
- **Address:** `0x00405E00`
- **Convention:** `__fastcall` (ECX = `param_1`)
- **Parameters:**
  - `param_1` (int\*): `Ball*` instance
- **Description:**
  Runs every frame for every ball in the scene. Contains:
  1. Guard: returns early if `ball[0x324]` (dead/eliminated flag) is set
  2. Resets collision mesh if `scene[0x237]` (battle mode flag) is set
  3. Clears per-frame state flags (`0x2f8`, `0x2ec`, `0x150`, etc.)
  4. Frees trail point lists
  5. Frees collision marker lists
  6. **AI block** (at offset ~0x31D): If `ball[0x31D] != 0` OR `scene[0x237] != 0`, runs `Ball_AI_ChaseNearest`
  7. Physics integration via `Ball_AdvancePositionOrCollision`
  8. Proximity checks to other balls
  9. Record trail points if enabled
  10. Ball falling state machine
- **Key offsets:**
  - `ball+0x324` = dead/eliminated flag (returns early if set)
  - `ball+0x31D` = `is_8ball` / AI enable flag
  - `ball+0x2f8` = per-frame active flag
  - `ball+0x2ec` = frame collision count
  - `ball+0x150` = collision state
  - `ball+0xC60..0xC94` = home pos, chase distance, NPC flag (AI params)
- **Modding use:** Hook here to modify ball behavior per-frame. Setting `ball[0x31D] = 1` on a player ball forces it to run the 8-ball AI chase code.

---

### `Ball_GetInputForce`
- **Address:** `0x46EC30`
- **Convention:** Standard (ECX = `this`)
- **Parameters:**
  - `this` (void\*): `InputDevice*` instance
  - `output` (float\*): OUT — writes `[force_x, force_y]`
- **Description:**
  Reads the current input device state and converts it to a 2D force vector. Supports 4 input modes:
  - **Mode 1 (Keyboard):** Reads DIK codes from `InputDevice+0x50C`/`0x510`/`0x514`/`0x518`. Up = -0.5, Down = 1.0, Left/Right = ±1.0
  - **Mode 2 (Mouse):** Computes offset from screen center. Re-centers cursor if mouse capture active.
  - **Mode 4-7 (Joystick):** Reads axes at `joy+0x10C` and `joy+0x110`, normalizes to unit circle.
- **Key offsets:**
  - `this+0x8` = input mode (1=keyboard, 2=mouse, 4-7=joy)
  - `this+4` -> `+0x434` = key state base
  - `key_state+0x50C` = LEFT DIK code
  - `key_state+0x510` = RIGHT DIK code
  - `key_state+0x514` = UP DIK code
  - `key_state+0x518` = DOWN DIK code
  - `this+0xC` = per-ball speed scale
- **Modding use:** Replace entirely for custom control schemes (analog steering, custom keybindings, autopilot).

---

### `Ball_ApplyForceV2`
- **Address:** `0x004016F0`
- **Convention:** `__thiscall` (ECX = `this`)
- **Parameters:**
  - `this` (void\*): `Ball*` instance
  - `dir_x` (float): Force direction X
  - `dir_y` (float): Force direction Y
  - `dir_z` (float): Force direction Z
  - `magnitude` (float): Force multiplier
- **Description:**
  Secondary force application used for collision-derived and system forces (not direct player input). Guard conditions: not in tarpit (`0x2F9`), force not disabled (`0x2CC`), not frozen (`0x808`), frame counter under limit (`0x2F0 < 81`).
  
  Force modifiers (applied in order):
  1. First frame: `magnitude *= 0.25`
  2. In tube (`0x324`): `magnitude *= 0.0` — **NO FORCE**
  3. On ice (`0xC5C`): `magnitude *= 0.01` + weak lateral push
  4. Is dizzy (`0xC4C`): `magnitude *= 0.75`
  
  Accumulates into `ball+0x170` (velocity X), `+0x174` (Y), `+0x178` (Z). Updates facing angle based on gravity plane.
- **Modding use:** Adjust force multipliers at global constants (`0x4CF380`, `0x4CF374`, `0x4CF36C`) to change ball feel.

---

### `Ball_AdvancePositionOrCollision`
- **Address:** `0x4564C0`
- **Convention:** Standard
- **Parameters:**
  - `this` (void\*): `Ball*` instance
  - `out_pos` (float\*): OUT — new position [x, y, z]
  - `cur_pos` (float\*): IN — current position [x, y, z]
  - `input_vel` (float\*): IN — input velocity [x, y, z]
  - `param_4` (int): Collision iteration / flags
  - `dt` (float): Delta time
- **Description:**
  Core physics step. Phases:
  1. Free trail point and collision marker lists
  2. Add input velocity to current velocity, clamp to `max_velocity` (`ball+0xC70`)
  3. Apply damping: `vel *= (1-dt) + (1-damping_factor)*dt` (`ball+0xC68`)
  4. Apply gravity (`ball+0xC88`/`0xC8C`/`0xC90` scaled by `ball+0xC94`)
  5. Integrate position: `pos += vel * dt`
  6. Sphere-vs-level collision (`TestSphereVsLevel`)
  7. Reflect velocity on collision, apply bounce damping
  8. Record trail points if `trail_flag` (`ball+0x14`) is set
  9. Set `ball+0xC7C` = collision occurred flag
- **Key offsets:**
  - `ball+0x14` = trail recording flag
  - `ball+0xC68` = damping factor
  - `ball+0xC70` = max velocity clamp
  - `ball+0xC7C` = collision occurred (0/1)
  - `ball+0xC88..0xC94` = gravity vector + scale
  - `ball+0xCA4..0xCAC` = velocity [x, y, z]
- **Modding use:** Hook to change physics behavior (low gravity, high damping, no-clip). The collision response is the key to ball feel.

---

### `Ball_SetTargetPos`
- **Address:** `0x00402030`
- **Convention:** Standard
- **Parameters:**
  - `this` (void\*): `Ball*` instance
  - `x` (float): Target X position
  - `y` (float): Target Y position
  - `z` (float): Target Z position
- **Description:**
  Sets the ball's target position (`ball+0x758`/`0x75C`/`0x760`), which is used by the camera system as the point to orbit around. This is NOT the physics position — it's the smoothed display position that the camera tracks.
- **Modding use:** Call to force camera to look at arbitrary points (cinematic angles, spectator mode).

---

## 3. Input System

### `Input_IsKeyDown`
- **Address:** `0x46E0B0`
- **Convention:** Standard
- **Parameters:**
  - `key` (int): DirectInput key code (DIK_\* constant)
- **Returns:** `int` — 1 if key is currently held, 0 otherwise
- **Description:**
  Checks the DirectInput8 keyboard state array for a specific key. Uses the standard DIK scancode set (e.g., DIK_SPACE = 0x39, DIK_ESCAPE = 0x01).
- **Modding use:** Poll any key for custom actions. Combine with `Ball_ApplyForceV2` for custom abilities bound to keys.

---

### `App_CreateInputDevice`
- **Address:** `0x0046C050`
- **Convention:** Standard
- **Parameters:**
  - `this` (void\*): `App*` instance
- **Returns:** `void*` — new `InputDevice*` (0x14 bytes)
- **Description:**
  Allocates a 0x14-byte `InputDevice` and calls the constructor. The InputDevice holds input mode, device handle, and key state. Called during `App_Initialize_Full` steps 16-22.
- **Key offsets:**
  - `InputDevice+0x00` = vtable
  - `InputDevice+0x04` = App* back-reference
  - `InputDevice+0x08` = input mode (1=keyboard, 2=mouse, etc.)
  - `InputDevice+0x10` = joystick device pointer
- **Modding use:** Create custom input devices for bot players or replay systems.

---

## 4. Camera System

### `Scene_SetCamera`
- **Address:** `0x00419FA0`
- **Convention:** `__thiscall` (ECX = `this`)
- **Parameters:**
  - `this` (void\*): `Scene*` instance
  - `param_1` (void\*): Ball* being tracked
  - `param_2` (char): Boolean — if true, apply path-following spring
- **Description:**
  Positions the camera every frame before rendering. Algorithm:
  1. Start with `ball->camera_target_pos` (`ball+0x758`)
  2. Add `scene->camera_offset` (`scene+0x434C`)
  3. If `path_follow_mode` (`scene+0x3F1C`) AND `param_2`:
     - Read path position at current param (`scene+0x3F24`)
     - Compute distance from camera to path
     - If distance > threshold: apply spring force back to path
     - Blend camera position toward path
  4. If `camera_shake_enabled` (`ball+0x0744`): add random offsets [-50 to +50]
  5. If `camera_snap_frames` (`scene+0x3F2C`) > 0: force camera to ball position
  6. Apply orbit rotation using `orbit_angle` (`scene+0x29BC`) and `camera_distance` (`scene+0x29C0`)
  7. Set D3D viewport and view/projection matrices
- **Key offsets:**
  - `scene+0x3F1C` = `path_follow_mode` (bool, camera follows spline path)
  - `scene+0x3F20` = `path_object` (Path\* for camera rail)
  - `scene+0x3F24` = `path_position` (float, parametric position on path)
  - `scene+0x3F2C` = `camera_snap_frames` (int, countdown to snap)
  - `scene+0x29BC` = `camera_orbit_angle` (float)
  - `scene+0x29C0` = `camera_distance` (float, max = 700 at `0x4CFF78`)
  - `scene+0x434C` = `camera_offset` (Vec3 added to ball position)
  - `ball+0x758` = `camera_target_pos` (Vec3)
  - `ball+0x76C` = `camera_actual_pos` (Vec3)
- **Modding use:**
  - Disable `path_follow_mode` (`scene+0x3F1C = 0`) for free-follow camera
  - Set `camera_distance` to zoom in/out
  - Set `camera_snap_frames` for instant camera cuts
  - Override `camera_offset` for third-person / overhead angles

---

## 5. Scene & Object Management

### `Scene_AddObject`
- **Address:** `0x00469990`
- **Convention:** Standard
- **Parameters:**
  - `this` (void\*): `Scene*` instance
  - `obj` (void\*): `Gadget*` or `SceneObject*` to add
- **Description:**
  Appends an object to the scene's active object list (`scene+0x2578`). The object will receive `Update` (vtable[4]) and `Render` (vtable[0]) calls every frame via `Scene_Update`.
- **Modding use:** Spawn custom objects mid-game. Must be a valid `Gadget` with a populated vtable.

---

### `Scene_ctor` / `Board_ctor`
- **Addresses:** `0x00419770` (Scene dtor), `0x00419030` (Board ctor)
- **Description:**
  `Board_ctor` creates the main gameplay `Board` object (derived from `Gadget`, size ~0x4368 bytes). It sets up the scene object list, ball list, D3D device reference, and all level-specific slots. `Scene` is essentially `Board` — they share the same object at runtime.
- **Modding use:** Hook `Board_ctor` to inject custom per-level initialization (spawn extra objects, modify lighting).

---

## 6. Level Factory & Object Creation

### `CreateLevelObjects`
- **Address:** `0x004121D0`
- **Convention:** `__thiscall` (ECX = `this`)
- **Parameters:**
  - `this` (void\*): `Scene*` instance
  - `name` (char\*): Object name from MESHWORLD (e.g., "BONK_01", "TIPPER_A")
  - `param_3` (int): Object index
  - `param_4` (void\*): Transform matrix pointer (position at +4/8/C, rotation at +10/14/18)
- **Description:**
  The main object factory. Uses `__strnicmp` to match object name prefixes and instantiate the appropriate game object:
  - `BRIDGE` → configures bridge collision mesh
  - `TIPPER` → `Tipper_ctor(0x1104)` + `TipperVisual_ctor(0x10D0)`
  - `BONK` → `Bonk_ctor(0x1200)`, stored at `Scene+0x540C`
  - `BBRIDGE1/2` → `BreakBridge_ctor(0x1100)`
  - `POPCYLINDER` → `PopCylinder_ctor(0x10E8)`
  - `BLOCKDAWG1/2` → `Blockdawg_ctor(0x1154)` + path lookup
  - `CATAPULT` → `Catapult_ctor(0x1108)`
  - `GLUEBIE` → `Gluebie_ctor(0x110C)`
  - `SAFESPOT` / `SAFEPOS` → safety spawn positions
  - `START1-1`, `START2-1`, etc. → ball spawn positions
  - `E:LIMIT`, `E:JUMP`, etc. → event triggers
  - `BADBALL` → 8-ball AI parameters (`CHASE`, `HOME`, `SIZE`, `SPINDISTANCE`)
- **Modding use:** Add new `__strnicmp` branches to spawn custom objects from level data. Or patch the factory table to replace existing object constructors.

---

## 7. Rendering Pipeline

### `Scene_Render`
- **Address:** `0x0041A2E0`
- **Convention:** `__thiscall` (ECX = `this`)
- **Parameters:**
  - `this` (void\*): `Scene*` instance
  - `param_1` (int\*): Graphics context pointer
- **Description:**
  Main rendering dispatch. Supports 1P and 2P split-screen. Per viewport:
  1. `Graphics_SetViewport` — set clip rect
  2. `Scene_SetCamera` — position camera around ball
  3. `vtable[0x60]` — RenderBackground (sky/far plane)
  4. `vtable[0x64]` — RenderOpaqueObjects (level geometry)
  5. `vtable[0x68]` — RenderTransparentObjects (glass, effects)
  6. `vtable[0x70]` — RenderOverlay (HUD, score)
  7. `vtable[0x6C]` — RenderPostEffects (fade, transitions)
- **Modding use:** Hook `RenderOpaqueObjects` (vtable `0x64`) or `RenderTransparentObjects` (vtable `0x68`) to draw custom geometry.

---

### `Scene_RenderAllObjects`
- **Address:** `0x0045E0E0`
- **Convention:** Standard
- **Parameters:**
  - `this` (void\*): `Scene*` instance
- **Description:**
  Iterates `scene+0x22E` (AthenaList of scene objects) and calls `obj->vtable[0]()` (Render) for each visible object.
- **Modding use:** The place to hook if you want to modify HOW objects render (wireframe mode, custom shaders,剔除).

---

## 8. Collision & Pathfinding

### `Ball_CheckCollisionPlanes`
- **Address:** (within `Ball_AdvancePositionOrCollision` at `0x4564C0`)
- **Convention:** Inline / called
- **Parameters:**
  - `ball` (void\*): Ball instance
  - `pos` (float\*): Position [x, y, z]
  - `vel` (float\*): Velocity [x, y, z]
- **Description:**
  Tests the ball's bounding sphere against the level collision mesh. Returns collision normal and penetration depth. Called from within `Ball_AdvancePositionOrCollision`.
- **Modding use:** Hook to disable collision for noclip, or add custom collision shapes.

---

### `Scene_CheckPath`
- **Address:** `0x00457EC0` (inferred)
- **Convention:** Standard
- **Parameters:**
  - `start` (int): Start cell index (0-358)
  - `target` (int): Target cell index (0-358)
- **Returns:** `int` — 1 (forward/clockwise), -1 (backward/counter-clockwise), 0 (unreachable)
- **Description:**
  Ring-topology pathfinder on a 360-cell circular grid. Two walkers start from `start`: one advances +1, one retreats -1. Whichever reaches `target` first determines direction. Used for angular collision and track snapping in `Ball_Update`.
- **Modding use:** Understand the ring cell layout if modifying tube/tunnel levels with circular paths.

---

## 9. App & System Initialization

### `App_Initialize_Full`
- **Address:** `0x00429530`
- **Convention:** `__thiscall` (ECX = `this`)
- **Parameters:**
  - `this` (void\*): `App*` instance
  - `param_1` (undefined4): Unused / reserved
  - `param_2` (undefined4): Unused / reserved
- **Description:**
  Full game initialization sequence (15+ steps):
  1. Call `App_Initialize` (base init)
  2. Load cursor (`BLANKCURSOR`)
  3. Create graphics device (800x600)
  4. Configure D3D render states
  5. Load `shadow.png` texture
  6. Load `music\music.mo3` (BASS music module)
  7. Load `jukebox.xml`
  8. Read `PlayCount` from registry
  9. Create input devices (keyboard, mouse, joystick)
  10. Load `players.xml`
  11. Create `MainMenu`
- **Modding use:** Hook after step 5 to inject custom textures, or after step 9 to replace input devices.

---

## 10. Vtables & Calling Conventions

### Ball Vtable (`0x4CF3A0`)
| Slot | Offset | Function | Purpose |
|------|--------|----------|---------|
| 0 | `0x00` | `Render` | Draw ball mesh |
| 1 | `0x04` | `Init` | Post-constructor setup |
| 4 | `0x10` | `Update` | Per-frame update (calls `Ball_Update` at `0x405E00`) |
| 5 | `0x14` | `ApplyForceWithMultipliers` | Force application with scale |
| 7 | `0x1C` | `CollisionHandler` | Custom collision response |

### Scene/Board Vtable
| Slot | Offset | Function | Purpose |
|------|--------|----------|---------|
| 4 | `0x10` | `Update` | Per-frame scene update |
| 8 | `0x20` | `RunFrame` | Full frame (Update+Render) |
| 10 | `0x28` | `Render` | Full render dispatch |
| 12 | `0x30` | `LoadLevel` | Level loading |
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

### Ball Struct (`0xC60` bytes, vtable `0x4CF3A0`)
| Offset | Type | Name | Description |
|--------|------|------|-------------|
| `+0x00` | void\* | vtable | `0x4CF3A0` |
| `+0x04` | void\* | scene | Parent Scene\* |
| `+0x60` | float[3] | position | Physics position (X, Y, Z) |
| `+0x164` | float[3] | display_pos | Smoothed display position |
| `+0x170` | float[3] | velocity | Current velocity |
| `+0x198` | float | facing_angle | Yaw in radians |
| `+0x2EC` | int | collision_count | Per-frame collision tally |
| `+0x2F0` | int | frame_counter | Frames since spawn |
| `+0x2F9` | char | tar_state | Tarpit / stuck flag |
| `+0x324` | char | dead | Eliminated / in-tube flag |
| `+0x31D` | char | is_8ball | AI enable flag |
| `+0x748` | int | gravity_plane | 0=flat, 1=tilted, 2=vertical |
| `+0x758` | float[3] | camera_target | Camera orbit center |
| `+0xC60` | float[3] | home_pos | AI home position |
| `+0xC70` | float | chase_distance | AI chase radius |
| `+0xC74` | char | npc_flag | AI active flag |

### Scene Struct (`~0x5400` bytes)
| Offset | Type | Name | Description |
|--------|------|------|-------------|
| `+0x022E` | AthenaList | objects | All scene objects |
| `+0x0A75` | AthenaList | balls | Ball list |
| `+0x0D88` | int | frame_counter | Game frame count |
| `+0x237` | char | battle_mode | 8-ball / race mode |
| `+0x3F1C` | char | path_follow | Camera spline follow |
| `+0x3F20` | void\* | path_obj | Path\* for camera |
| `+0x29BC` | float | orbit_angle | Camera orbit rotation |
| `+0x29C0` | float | cam_distance | Orbit distance (max 700) |
| `+0x434C` | float[3] | cam_offset | Added to ball pos |
| `+0x2578` | AthenaList | active_objs | Factory output list |

---

## Document Info

- **Generated from:** Ghidra 12.0 decompilation
- **Game version:** Hamsterball.exe (PE32, i386)
- **Total functions renamed:** 1,200+ (see `analysis/ghidra/renames_backup.json`)
- **Decompilation sources:** `analysis/ghidra/decompilations/`
- **Last updated:** 2026-06-07

For full struct definitions, see `analysis/ghidra/structs/*.h`.
For the object catalog, see `docs/OBJECT_CATALOG.md`.
For AI internals, see `docs/8BALL_AI_SYSTEM.md`.
