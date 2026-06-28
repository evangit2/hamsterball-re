# Hamsterball Modding Function Reference

A comprehensive reference of every useful function for modders, extracted from Ghidra decompilation of `Hamsterball.exe` (PE32, i386, Athena engine).  
**Last updated:** 2026-06-13  
**Total functions in binary:** ~3,800

---

## Table of Contents

1. [Game Loop & Frame Control](#1-game-loop--frame-control)
2. [App & System Initialization](#2-app--system-initialization)
3. [Ball Physics & Movement](#3-ball-physics--movement)
4. [Input System](#4-input-system)
5. [Camera System](#5-camera-system)
6. [Scene & Object Management](#6-scene--object-management)
7. [Object Spawning & Creation](#7-object-spawning--creation)
8. [Collision Event System](#8-collision-event-system)
9. [Rendering & Graphics Pipeline](#9-rendering--graphics-pipeline)
10. [Audio & Music System](#10-audio--music-system)
11. [UI & Menu System](#11-ui--menu-system)
12. [Save, Registry & Progression](#12-save-registry--progression)
13. [Arena Scoring & Timer](#13-arena-scoring--timer)
14. [Level-Specific Functions](#14-level-specific-functions)
15. [Utility & Math Functions](#15-utility--math-functions)
16. [Vtables & Calling Conventions](#16-vtables--calling-conventions)
17. [Quick Reference: Most-Used Offsets](#quick-reference-most-used-offsets)

---

## 1. Game Loop & Frame Control

### `App_Run`
- **Address:** `0x0046BD80`
- **Convention:** `__fastcall` (ECX = `param_1`)
- **Parameters:** `param_1` (int*): Pointer to `App` instance (`g_App` at `0x4FD680`)
- **Description:** Main game loop. Runs until `App+0x159` (quit flag) is set. Every frame: Win32 message pump → `App.Update` (vtable `0x20`) → `App.Render` (vtable `0x28`) → `Graphics_PresentOrEnd`.
- **Modding use:** Hook vtable slot `0x20` (Update) or `0x28` (Render) to inject custom per-frame logic.
- **Key code pattern:**
  ```c
  while (*(char*)(app + 0x159) == 0) {
      PeekMessageA(...);
      app->vtable[0x20]();  // Update
      app->vtable[0x24]();  // PreRender
      app->vtable[0x28]();  // Render
      app->vtable[0x2C]();  // PostRender / HUD
  }
  ```

### `Scene_Update`
- **Address:** `0x00419C00`
- **Convention:** `__thiscall` (ECX = `this`)
- **Parameters:** `this` (void*): `Scene*` instance
- **Description:** Central game tick. Execution order:
  1. Increment `frame_counter` (`this+0x0D88` / `this+0x3620`)
  2. Demo timer check (if `demo_timer_active`)
  3. ESC key → spawn `GameOverMenu` if not suppressed
  4. Ball position propagation (if `ball_positions_dirty`)
  5. Gear path following (single-gear camera mode)
  6. Rumble board timer ticks
  7. Camera shake decay (`shake_magnitude` += 10/frame toward 0)
  8. SceneObject update+render loop
  9. Physics pipeline (4 vtable calls, see below)
  10. Level object update (`Scene_LevelObjUpdate`)
- **Key offsets:**
  - `this+0x022E` (int index `0x8B`) = `scene_objects` (AthenaList)
  - `this+0x0A6C` (int index `0x29B`) = `ball_positions_dirty`
  - `this+0x0A75` (int index `0x29D`) = `ball_list` (AthenaList\<Ball*\>)
  - `this+0x0D88` (int index `0x362`) = `frame_counter`
- **Modding use:** Hook object vtable slot `4` (Update) for per-object behavior.

---

## 2. App & System Initialization

### `App_Initialize_Full`
- **Address:** `0x00429530`
- **Convention:** `__thiscall` (ECX = `this`)
- **Parameters:** `this` (App*), `param_1`, `param_2` (both unused — WinMain params)
- **Description:** Full 26-step game init sequence. Each step writes `"Initialize(N)"` to `App+0x208`:
  1. `App_Initialize` (base init)
  2. Set `Graphics+0x7D1 = 1`
  3. Load `"BLANKCURSOR"` → `App+0x240`
  4. `vtable[0x8C](800, 600)` — set display mode
  5. Configure D3D render states (16 vs 32-bit depth)
  6. Load `shadow.png` → `App+0x278`
  7. Load `music\music.mo3` → `App+0x534`
  8–11. Parse `jukebox.xml`, create music channels
  12. `Registry_ReadPlayCount`
  13–14. Create 4 `InputDevice` instances (players 1–4)
  15–22. Configure input devices with mode flags
  23. `RegKey_Close`
  24. `vtable[0xA0]()` — show title screen / main menu
  25. Done
- **Modding use:** Hook after step 24 to inject custom startup code.

### `WinMain`
- **Address:** `0x004278E0`
- **Description:** Entry point. Calls `App_Initialize_Full` → `App_Run` → `App_Shutdown`.

### `App_ShowMainMenu`
- **Address:** `0x004280E0`
- **Description:** Allocates `0xCDC` bytes, calls `MainMenu_ctor`, stores at `App+0x224`, adds to scene via `Scene_AddObject`.

### `App_SaveAllConfig`
- **Address:** `0x004284C0`
- **Description:** Writes all game settings to Windows registry:
  - Display settings (`App_WriteDisplaySettings`)
  - Mouse sensitivity (`App+0x84C`)
  - Mirror tournament (`App+0x850`)
  - All race/arena unlock flags (`0x851`–`0x868`)
  - Best times binary blob (`0x86C`, 80 bytes)
  - Medal status (`0x8BC`, 80 bytes)
  - 2P controller mappings (`0xB28`–`0xB34`)

### `App_LoadOrSaveConfig`
- **Address:** `0x004279F0`
- **Description:** Called on game exit. Decides whether to save or load config based on internal state.

---

## 3. Ball Physics & Movement

### `Ball_Update`
- **Address:** `0x00405E00`
- **Convention:** `__fastcall` (ECX = `param_1`)
- **Parameters:** `param_1` (int*): `Ball*` instance
- **Description:** Per-frame ball update. Returns early if `ball+0x324` (dead/eliminated). Clears per-frame flags, runs AI if `ball+0x31D` (is_8ball) or `scene+0x237` (battle mode), then calls physics (`Ball_AdvancePositionOrCollision`). After physics: proximity checks, trail recording, respawn state machine.
- **Key offsets:**
  - `ball+0x324` = dead/eliminated flag
  - `ball+0x31D` = `is_8ball` / AI enable flag
  - `ball+0xC60..0xC94` = AI params (home_pos, chase_distance, npc_flag)

### `Ball_ctor` / `Ball_ctor2`
- **Addresses:** `Ball_ctor` ≈ `0x40AFE0`, `Ball_ctor2` = `0x4039E0`
- **Description:**
  - `Ball_ctor`: allocate `0xC98` bytes (8ball) or `0xC60` (player), call `GameObject_ctor`, set vtable to `0x4CF3A0`
  - `Ball_ctor2`: set physics defaults — gravity `0.5`, radius `26.0`, max_speed `5.0`
  - Player ball: `operator_new(0xC60)` → `Ball_ctor2(this, scene)` → `vtable[4]()` → `Ball_SetTrajectory`

### `Ball_GetInputForce`
- **Address:** `0x46EC30`
- **Parameters:** `this` (InputDevice*), `output` (float*) → `[force_x, force_y]`
- **Description:** Converts input device state to 2D force vector. Reads DIK codes at `InputDevice+0x50C/0x510/0x514/0x518`.
- **Key offsets:** `this+0x08` = input mode

### `Ball_AdvancePositionOrCollision`
- **Address:** `0x4564C0`
- **Parameters:** `this` (Ball*), `out_pos`, `cur_pos`, `input_vel`, `collision_flags`, `dt`
- **Description:** Core physics step. Velocity integration → damp → gravity → `TestSphereVsLevel` collision → bounce → trail.
- **Key physics globals:**
  - `0x4CF3F0` = damping constant (`0.95`)
  - `0x4CF3E8` = ice friction factor (`6.0`)
  - `0x4CF374` = force multiplier on ice (`0.2`)
  - `0x4CF380` = force multiplier after first frame (`0.25`)

### `Ball_SetTargetPos`
- **Address:** `0x00402030`
- **Parameters:** `this` (Ball*), `x`, `y`, `z` (floats)
- **Description:** Sets `ball+0x758/75C/760` — camera orbit center point (smoothed display position).

### `Ball_Shrink`
- **Address:** `0x402200`
- **Description:** Odd Race E:SHRINK collision handler. Sets `ball+0xC4C = 1` (is_shrunk), `ball+0x284 = 13.0` (shrunk radius), plays fall sound.

### `Ball_FallUpdate`
- **Address:** `0x408830`
- **Description:** Physics update while falling. Decrements `alpha` (`ball+0x80C`). When timer expires, calls `Ball_FindClosestRespawnPoint`.

### `Ball_FindClosestRespawnPoint`
- **Address:** `0x405190`
- **Description:** Scans `scene+0x546` (SAFESPOT/SAFEPOS list) for nearest valid respawn position. Writes new position to `ball+0x164..16C`.

### `Ball_Shatter`
- **Address:** `0x408D70`
- **Convention:** `__thiscall` (ECX = Ball* — the parent ball being replaced)
- **Called from:** `FollowBall_Update` (0x43ECC0) — NOT from E:JUMP or any collision handler
- **Description:** Arena mechanic that replaces the parent ball with 3 AI-controlled split balls. The parent ball is marked for despawn (`+0x2E8 = 1`), then 3 new `Ball_Split` objects are created via `Ball_Split_ctor` (0x408D10). Each split ball gets:
  - Allocation: `0xC64` bytes
  - `ball+0x31D = 1` (is_8ball flag, activates AI)
  - `ball+0xC6 = 10.0` (0x41200000 — split ball size)
  - Split IDs: 1, 2, 4 (bitmask, one per iteration)
  - Trajectory from parent's `+0x2AC–0x2B8`
  - Added to `scene+0x3204` (ball list)
- **Guard conditions:** `+0x324 == 0` (not already an 8-ball) AND `+0x744 == 0` (hasn't split yet)

### `Ball_InitBattleMode`
- **Address:** `0x456CD0`
- **Description:** Converts ball to battle mode. Sets:
  - `ball+0xC60 = 3` (battle flag)
  - `ball+0xC68 = 0.55` (high friction)
  - `ball+0xC6C = 1.0` (bouncy)
  - `ball+0xC70 = 1000.0` (max speed — 200x normal!)

---

## 4. Input System

### `Input_IsKeyDown`
- **Address:** `0x46E0B0`
- **Parameters:** `key` (int) — DIK scancode
- **Returns:** 1 if held, 0 if not

### `App_CreateInputDevice`
- **Address:** `0x0046C050`
- **Returns:** `InputDevice*` (0x14 bytes)
- **Key offsets:** `+0x00` vtable, `+0x04` App*, `+0x08` input mode, `+0x10` joystick ptr

### `App_CreateInputHandler`
- **Address:** `0x0046C110`
- **Returns:** `InputHandler*` — aggregates 4 `InputDevice` instances

### `InputDevice_SetType`
- **Address:** `0x46DFC0`
- **Parameters:** `this` (InputDevice*), `mode` (int) — 1=keyboard, 2=mouse, 4-7=joystick

---

## 5. Camera System

### `Scene_SetCamera`
- **Address:** `0x00419FA0`
- **Convention:** `__thiscall` (ECX = `this`)
- **Parameters:** `this` (Scene*), `param_1` (Ball*), `param_2` (char) — if true, apply path-following spring
- **Description:** Camera positioning every frame. 5 modes:
  1. **Default Follow:** Orbit around `ball+0x758` + `scene+0x434C` offset
  2. **Path Rail:** If `scene+0x3F1C != 0` AND `param_2 != 0` → call `Path_GetPosition(scene+0x3F20, &out, scene+0x3F24)`
  3. **Camera Shake:** If `ball+0x744`: random ±50 per axis
  4. **Snap:** If `scene+0x3F2C > 0`: force camera to ball physics pos, decrement
  5. **Orbit:** Apply `scene+0x29BC` (angle) and `scene+0x29C0` (distance)
- **Key offsets:**
  - `scene+0x3F1C` = `path_follow_mode`
  - `scene+0x3F20` = `path_object`
  - `scene+0x3F24` = `path_position`
  - `scene+0x3F2C` = `camera_snap_frames`
  - `scene+0x29BC` = `camera_orbit_angle`
  - `scene+0x29C0` = `camera_distance` (max 700)
  - `scene+0x434C` = `camera_offset` (Vec3)

---

## 6. Scene & Object Management

### `Scene_AddObject`
- **Address:** `0x00469990`
- **Parameters:** `this` (Scene*), `obj` (Gadget*)
- **Description:** Appends to `scene+0x2578` (active object list). Objects receive `Update` (vtable[4]) and `Render` (vtable[0]) per frame.

### `Scene_SpawnBallsAndObjects`
- **Address:** `0x0041C5B0`
- **Description:** Level startup factory. Creates all game objects:
  1. Ball creation loop — lookup `"START%d-%d"` in hash table, `Ball_ctor2`, set properties
  2. Safe spot scan — `"SAFESPOT"`/`"SAFEPOS"` → `scene+0x546`
  3. Tournament/demo: `CreateBadBall`, `CreateMouseTrap`
  4. Decoration: `CreateSecretObjects`, `Scene_CreateFlags`, `Scene_CreateSigns`, `Scene_CreateDynamicObjects`
- **Ball defaults:** radius=26.0, max_speed=5.0, gravity_scale=0.5

### `Board_ctor`
- **Address:** `0x00419030`
- **Description:** Creates Board (~0x5400 bytes, inherits Gadget). Sets up scene lists, ball list, D3D device, level slots.

### `Scene_CleanupScene`
- **Address:** `0x419740`
- **Description:** Destructor helper. Cleans all AthenaLists in reverse order: collision_list → ripple_list → ball_list → scene_object_list → etc.

---

## 7. Object Spawning & Creation

### `CreateLevelObjects` (Object Factory)
- **Address:** `0x004121D0`
- **Convention:** `__thiscall` (ECX = `this`)
- **Parameters:** `this` (Scene*), `name` (char*), `param_2` (matrix), `param_3` (matrix), `param_4` (index)
- **Description:** Main object factory. Matches prefixes via `__strnicmp`:
  - `BRIDGE` → `scene+0x436C`
  - `TIPPER` → `Tipper_ctor(0x1104)` + visual
  - `BONK` → `Bonk_ctor(0x1200)` → `scene+0x540C`
  - `BBRIDGE1`/`BBRIDGE2` → `BreakBridge_ctor(0x1100)`
  - `POPCYLINDER` → `PopCylinder_ctor(0x10E8)`
  - `BLOCKDAWG1`/`BLOCKDAWG2` → `Blockdawg_ctor(0x1154)`
  - `CATAPULT` → `Catapult_ctor(0x1108)` → `scene+0x584C`
  - `GLUEBIE` → `Gluebie_ctor(0x110C)` → `scene+0x6080`
  - `SAFESPOT`/`SAFEPOS` → spawn positions
  - `BADBALL` → 8-ball AI parameters

### `CreateBadBall`
- **Address:** `0x0040BCA0`
- **Description:** Scans meshworld for `"BADBALL"`. For each:
  - `Ball_ctor(new(0xC98), scene)` — note: larger than player ball!
  - Parse tags: `<CHASE>`, `<HOME>`, `<SIZE>`, `<SPINDISTANCE>`
  - Append to `scene+0x29D4` (8ball list) and `scene+0x2DEC` (active ball list)

### `CreateBumper`
- **Address:** `0x0040FA20`
- **Description:** Creates 8 bumpers for Level 8. Loads `levels\level8`, calls `Scene_CollectByNameFilter("N:BUMPER%d")`.

### `CreateBumper2`
- **Address:** `0x00413CE0`
- **Description:** Arena variant — 4 bumpers for beginner arena.

### `CreateMouseTrap`
- **Address:** `0x0040BF50`
- **Description:** Scans meshworld for `"MOUSETRAP"`. Creates `TipperVisual` objects appended to collision and render lists.

### `CreateBonkPopup`
- **Address:** `0x00438B30`
- **Description:** Hammer hit visual feedback. Plays 3D sound, calls `vtable[0x88]("BONKPOPUP")`.

### `Hammer_ChaseStart`
- **Address:** `0x00438BB0`
- **Description:** Starts hammer AI chase. Sets chase flags, copies position/target, chase speed = 0.5.

---

## 8. Collision Event System

### `DispatchCollisionEvents` / `DispatchCollisionEvents`
- **Address:** `0x0040C5D0`
- **Convention:** `__thiscall` (ECX = this)
- **Parameters:** `this` (Level*), `ball` (Ball*), `collider` (CollisionObject*)
- **Description:** Universal collision event dispatcher. Event name at `collider[1]+0x864`.

| Event | Condition | Action |
|-------|-----------|--------|
| `N:SECRET` | — | `Rotator_MarkTriggered` |
| `N:UNLOCKSECRET` | — | `CheckArenaUnlock(scene)` |
| `E:NODIZZY<TIME>N</TIME>` | — | `Ball_RecordBest(ball, duration)` |
| `E:SAFESWITCH(data)` | — | Copy data to `ball+0xC2C` |
| `E:LIMIT` | — | Clear `ball+0x1DA`, set `ball+0x2E9=1` (dizzy_lock — prevents Ball_ApplyTrajectory re-firing) |
| `E:BREAK` | — | `ball->vtable[0x20]()` bounce callback |
| `E:JUMP` | `impactCounter < 1` | Sound, force 0.025, freeze 10 frames, `Ball_RecordBest(+200)` |
| `E:ACTION<ONCE>TRUE</ONCE><SCORE>N</SCORE>` | — | Check duplicate at `ball+0xCB`, award score |
| `E:TRAJECTORY<X>..</X>` | — | Set collision direction `ball+0xCA4/CA8/CAC` |
| `N:NOCONTROL` | — | `ball+0x202 = 10` (disable input 10 frames) |
| `N:WATER` | — | `ball+0x2D5 = 1`, `ball+0xB6 = 10` |
| `N:TARPIT` | first time | Play tar sound, `ball+0xB3 = 1` |
| `DROPIN` | `dist > threshold` | Sound, `dropinCounter = 50`, `Ball_RecordBest(+200)` |
| `PIPEBONK` | `counter < 1` | Random sound, `counter = 10`, `Ball_RecordBest(+100)` |
| `POPOUT` | `counter < 1` | Sound, `counter = 50`, `Ball_RecordBest(+100)` |
| `N:GOAL` | `!finished && active` | Set `goalReached=1`, play music, mark finished |
| `N:MOUSETRAP` | — | Randomize RNG, deflect direction × trap speed |

### `TowerCollisionEvents`
- **Address:** `0x40DCD0`
- **Description:** Level-specific events, then delegates to `DispatchCollisionEvents`:
  - `E:CATAPULTBOTTOM` → launch catapult
  - `E:OPENSESAME` → open all trapdoors
  - `N:TRAPDOOR` → activate trapdoor
  - `E:BITE` → `scene+0x43A0 = 25.0`
  - `E:MACETRIGGER` → activate maces

### `ExpertCollisionEvents`
- **Address:** `0x40E6A0`
- **Description:** Arena-specific events (see `docs/ARENA_SCORING.md` for full table):
  - `E:CALLHAMMER` → `CreateBonkPopup`
  - `E:HAMMERCHASE` → `Hammer_ChaseStart`
  - `E:ALERTSAW1/2` → `Saw_AlertActivate`
  - `E:ACTIVATESAW1/2` → `Saw_Activate`
  - `E:ALERTJUDGES` → reset all judges
  - `E:SCORE<number>` → parse time, `ScoreDisplay_SetTime`
  - `E:BELL` → `Bell_Activate`, +500 time, "EXTRA TIME:" popup
  - `E:JUMP` → jump pad

---

## 9. Rendering & Graphics Pipeline

### `Scene_Render`
- **Address:** `0x0041A2E0`
- **Convention:** `__thiscall`
- **Description:** Main render dispatch. Per viewport:
  1. `Graphics_SetViewport`
  2. `Scene_SetCamera`
  3. `vtable[0x60]` — Background (sky/dome)
  4. `vtable[0x64]` — Opaque geometry
  5. `vtable[0x68]` — Transparent objects
  6. `vtable[0x70]` — Overlay (HUD)
  7. `vtable[0x6C]` — PostEffects (fade/transition)

### `Scene_RenderAllObjects`
- **Address:** `0x0045E0E0`
- **Description:** Iterates `scene+0x22E` (AthenaList), calls `obj->vtable[0]()` (Render) for each.

### `Ball_Render`
- **Address:** `0x402DE0`
- **Description:** D3D8 ball rendering. Sets up render state, applies world transform from `ball+0xC88` (4×4 matrix), draws sphere mesh with texture.

### `Ball_RenderShadow`
- **Address:** `0x401920`
- **Description:** Draws shadow quad beneath ball. Uses `shadowTexture` (`App+0x278`), alpha blend at ball position with Y offset.

### `Ball_CreateTrailParticles`
- **Address:** `0x401DD0`
- **Description:** Creates trail of particles behind moving ball. Appends to `scene+0x3B00` (trail_particles_ptr).

### `Level_UpdateAndRender`
- **Address:** `0x40B600` (vtable[0x64])
- **Description:** Two-pass rendering: opaque then alpha. Updates waypoint arrow, renders visible objects.

### `Level_RenderObjects`
- **Address:** `0x40B570` (vtable[0x68])
- **Description:** Transparent pass. Glass, water, effects.

### `Level_RenderDynamicObjects`
- **Address:** `0x40B420` (vtable[0x60])
- **Description:** Sky/dome + water ripples + dynamic object callback.

### `Graphics_Initialize`
- **Address:** `0x455380`
- **Description:** Creates D3D8 device, enumerates display modes, sets up render targets.

### `Gfx_SetAlphaBlendState`
- **Address:** `0x425FE0`
- **Description:** Toggles `D3DRS_ALPHABLENDENABLE`.

### `Gfx_SetCullMode`
- **Address:** `0x427940`
- **Description:** Sets `D3DRS_CULLMODE`.

---

## 10. Audio & Music System

### `Audio_PlayMusic`
- **Address:** inferred from `MusicPlayer_ctor` usage
- **Description:** Plays BASS music stream. Music handle at `App+0x534`.

### `Audio_PlayMusicAtSpeed`
- **Address:** inferred
- **Parameters:** `musicHandle`, `trackName` (char*), `speed` (float)
- **Description:** Plays music at modified tempo. Used by `MusicPlayer_ctor` for "Main Theme - No Intro" at 2.0× speed.

### `MusicPlayer_ctor`
- **Address:** `0x426030`
- **Parameters:** `this`, `App*`, `bool skipIntro`
- **Description:** Creates music player gadget. If `skipIntro==false`: plays `"Main Theme"`. If `true`: plays `"Main Theme - No Intro"` at 2.0×.

### `Sound_Play3D`
- **Address:** inferred from multiple call sites
- **Parameters:** `soundHandle`, `x`, `y`, `z` (floats)
- **Description:** Plays positional 3D audio effect at world coordinates.

### `Level_ReadSoundVolume`
- **Address:** `0x466570`
- **Description:** Reads `"Sound Volume"` from registry, defaults to `1.0`.

### `SoundDevice_dtor`
- **Address:** `0x4668A0`
- **Description:** Writes current volume back to registry before cleanup.

---

## 11. UI & Menu System

### `MainMenu_ctor`
- **Address:** `0x42DE50`
- **Description:** Creates main menu gadget (~0xCDC bytes). Buttons: Play, Practice, Options, Quit.

### `PauseMenu_Ctor`
- **Address:** `0x42E4B0`
- **Description:** Creates pause menu overlay. Triggered by ESC during gameplay.

### `OptionsMenu_RenderControls`
- **Address:** `0x42E840` / `0x42E910`
- **Description:** Renders control remapping UI. Reads/writes `App+0xB28`–`0xB34` (2P controller mappings).

### `PracticeMenu_ctor`
- **Address:** `0x42EA30`
- **Description:** Practice mode menu — level select without tournament constraints.

### `TimeTrialMenu_ctor`
- **Address:** `0x42F810`
- **Description:** Time trial menu — race against ghost data.

### `ArenaMenu_ctor`
- **Address:** `0x42FC40`
- **Description:** Arena mode menu — multiplayer level select.

### `PartyMenu_ctor`
- **Address:** `0x42FC10`
- **Description:** Party mode menu.

### `DifficultyMenu_ctor`
- **Address:** `0x42E220`
- **Description:** Difficulty selection menu (affects AI speed/timing).

### `TourneyMenu_GetRaceName`
- **Address:** `0x4264A0`
- **Returns:** `char*` — current race level name string

### `UI_DrawTextCentered`
- **Address:** `0x409C60`
- **Description:** Draws centered text at screen position.

### `UI_DrawTextCenteredAbsolute`
- **Address:** `0x4013A0`
- **Description:** Draws centered text with absolute pixel coordinates.

### `UI_DrawTextShadow`
- **Address:** `0x4012C0`
- **Description:** Draws text with drop shadow effect.

### `UI_DrawTextShadow_Wrapper`
- **Address:** `0x409B90`
- **Description:** Wrapper for shadow text with additional parameters.

### `Font_DrawCentered`
- **Address:** `0x42C870`
- **Description:** Renders centered string using game font.

---

## 12. Save, Registry & Progression

### `TourneyMenu_WriteSave`
- **Address:** `0x4264B0`
- **Description:** Writes `DATA\TOURNAMENT.SAV` (~151 bytes):
  - `Profile+0x08` (4 bytes) — current_race
  - `Profile+0x14` (4 bytes)
  - `Profile+0x18` (60 bytes) — race_time_array[15]
  - `Profile+0x54` (60 bytes) — race_time_array_2[15]
  - `Profile+0x90` (4 bytes) — accumulated_time
  - `Profile+0x94` (1 byte) — difficulty
  - `Profile+0x95` (1 byte) — rollback flag
  - `App+0x236` (1 byte) — mirror mode
  - `App+0x23C` (4 bytes) — race active
  - `App+0x5E8` (4 bytes) — total time
  - `App+0x5E4` (4 bytes) — ranking time
  - `App+0x5F4` (4 bytes)

### `TourneyMenu_LoadSaveAndShow`
- **Address:** `0x4265A0`
- **Description:** Reads `DATA\TOURNAMENT.SAV` with same field order, then creates `TourneyMenu`.

### `LoadConfig`
- **Address:** `0x42AE80`
- **Description:** Loads display settings from registry on startup.

### `SaveConfig`
- **Address:** `0x42B6E0`
- **Description:** Saves display settings to registry.

### `CheckPurchaseOrHighScore`
- **Address:** `0x40A420`
- **Description:** Shareware nag screen. If not registered:
  - Creates `ConfirmMenu` with "BUY HAMSTERBALL..." text
  - Or `HighScoreEntry` if game in progress
  - Registered version: skips dialog, allows save

### `CheckArenaUnlock`
- **Address:** `0x40ABA0`
- **Description:** Checks if arena levels should be unlocked based on race completion progress.

### `LoadRaceData`
- **Address:** `0x40A120`
- **Description:** Parses `racedata.xml` for medal thresholds:
  - `TIME` → target time
  - `GOLD`/`SILVER`/`BRONZE` → medal cutoffs (stored as `9 - value`)
  - `WEASEL` → weasel time threshold
  - Reads per-level data based on level name parameter

---

## 13. Arena Scoring & Timer

> **Full documentation:** See `docs/ARENA_SCORING.md`

### `ArenaBoard_Update`
- **Address:** `0x421FE0`
- **Description:** Per-frame arena update. Checks timer expiration, computes winner, handles tie-breaker.

### `ArenaBoard_Render`
- **Address:** `0x421910`
- **Description:** Draws countdown timer, 4-player HUD, tie-breaker overlay.

### `Rotator_AddBall`
- **Address:** `0x43B6F0`
- **Signature:** `__thiscall Rotator_AddBall(Scene* scene, Ball* ball)`
- **Description:** Registers ball on rotator's ball-tracking AthenaList (at `scene+0x10F0`). Allocates 8-byte entry `[ball_ptr, tick_counter=10]`. If ball already in list, resets tick to 10. Called from collision handlers on `N:ONROTATOR` (Impossible), `N:SPINNY` (Toob), `N:SWIRL` (Dizzy arena). Tick counter decremented each frame by `Catapult_Update` (0x43E600); resets to 10 on every frame of continued contact (10-frame grace period after leaving). Formerly misnamed `ScoreObject_SetScore` — has nothing to do with scoring.

### `ScoreObject_ctor`
- **Address:** `0x44BE80`
- **Parameters:** `this`, `App*`, `playerData*`, `label` (char*)
- **Size:** `0x30` bytes
- **Description:** Creates a SceneObject with vtable `PTR_RaceGoalReached_Render` (0x4D6C70). Used for race goal rendering and as the container type for rotator ball tracking.

### `ScoreDisplay_SetTime`
- **Address:** `0x434C80`
- **Description:** Sets displayed timer string with randomized decimal variation.

---

## 14. Level-Specific Functions

Each level has a custom `BoardLevel` subclass with constructor and destructor:

### WarmUp (Level 1)
- **Ctor:** `BoardLevel1_WarmUp_ctor` @ `0x41CA40`
- **Dtor:** `BoardLevel1_WarmUp_dtor` @ `0x41CB10`

### Intermediate (Level 2)
- **Ctor:** `BoardLevel2_Intermediate_ctor` @ `0x41CB20`
- **Dtor:** `BoardLevel2_Intermediate_dtor` @ `0x41CC80`

### Dizzy (Level 4)
- **Ctor:** `BoardLevel3_Dizzy_ctor` @ `0x41D060` (note: named Level3 in code)
- **Dtor:** `BoardLevel3_Dizzy_dtor` @ `0x41D450`

### Tower (Level 5)
- **Ctor:** `BoardLevel5_Tower_ctor` @ `0x41E340`
- **Dtor:** `BoardLevel5_Tower_dtor` @ `0x41E640`

### Expert (Level 8)
- **Ctor:** `BoardLevel8_Expert_ctor` @ `0x41EA40`
- **Dtor:** `BoardLevel8_Expert_dtor` @ `0x41EC90`

### Odd (Level 9)
- **Ctor:** `BoardLevel9_Odd_ctor` @ `0x41ED80`
- **Dtor:** `BoardLevel9_Odd_dtor` @ `0x41EE70`

### Wobbly (Level 12)
- **Ctor:** `BoardLevel12_Wobbly_ctor` @ `0x41F110`
- **Dtor:** `BoardLevel12_Wobbly_dtor` @ `0x41F3C0`

### Toob
- **Ctor:** `BoardLevel_Toob_Ctor` @ `0x41F4B0`
- **Dtor:** `BoardLevel_Toob_dtor` @ `0x41F720`

### Sky
- **Ctor:** `BoardLevel_Sky_Ctor` @ `0x41F930`
- **Dtor:** `BoardLevel_Sky_Dtor` @ `0x41FBC0`

### Beginner
- **Ctor:** `BoardLevel_Beginner_Ctor` @ `0x4200E0`
- **Dtor:** `BoardLevel_Beginner_Dtor` @ `0x4201D0`
- **HandleRaceEnd:** `Board_Beginner_HandleRaceEnd` @ `0x420240`

### Up
- **Ctor:** `BoardLevel_Up_Ctor` @ `0x420390`
- **Dtor:** `BoardLevel_Up_Dtor` @ `0x420550`

### Arena Level Constructors (via `TourneyMenu_CreateBoard`)
| ID | Arena | Ctor Name | Size |
|----|-------|-----------|------|
| 1 | WarmUp | `ArenaBoard_Warmup_Ctor` | `0x47E0` |
| 2 | Beginner | `ArenaBoard_Beginner_Ctor` | `0x5850` |
| 3 | Intermediate | `ArenaBoard_Intermediate_Ctor` | `0x47E0` |
| 4 | Dizzy | `ArenaBoard_Dizzy_Ctor` | `0x47E4` |
| 5 | Tower | `ArenaBoard_Tower_Ctor` | `0x501C` |
| 6 | UpArena | `ArenaBoard_Up_Ctor` | `0x47E4` |
| 7 | NeonArena | `ArenaBoard_Neon_Ctor` | `0x47E8` |
| 8 | ExpertArena | `ArenaBoard_Expert_Ctor` | `0x4BFC` |
| 9 | OddArena | `ArenaBoard_Odd_Ctor` | `0x47E0` |
| 10 | ToobArena | `ArenaBoard_Toob_Ctor` | `0x5C6C` |
| 11 | WobblyArena | `ArenaBoard_Wobbly_Ctor` | `0x47E4` |
| 12 | Glass | `BoardLevel_Glass_ctor` | `0x47E0` |
| 13 | SkyArena | `ArenaBoard_Sky_Ctor` | `0x4CFC` |
| 14 | WarmupArena | `ArenaBoard_WarmupArena_Ctor` | `0x47E0` |
| 15 | Impossible | `ArenaBoard_Impossible_Ctor` | `0x47E4` |

### `TourneyMenu_CreateBoard`
- **Address:** `0x426780`
- **Description:** Giant switch statement (cases 1–15) that allocates and constructs the correct `ArenaBoard` subclass for the selected arena level.

---

## 15. Utility & Math Functions

### Vec3 Operations
| Function | Address | Description |
|----------|---------|-------------|
| `Vec3_Copy` | `0x401010` | Copy vector |
| `Vec3_Init` | `0x401040` | Initialize to zero |
| `Vec3_dtor` | `0x401070` | Destructor (no-op) |
| `Vec3_Scale` | `0x4016C0` | Multiply by scalar |
| `Vec3_DivideByScalar` | `0x401890` | Component-wise divide |
| `Vec3_AddTwo` | `0x4018C0` | Add two vectors |
| `Vec3_AddInPlace` | `0x4018F0` | `a += b` |
| `Vec3_Length` | `0x401A60` | Euclidean length |
| `Vec3_NormalizeAndScale` | `0x401AA0` | Normalize then scale |
| `Vec3_Distance` | `0x401D20` | Distance between two vectors |

### Matrix Operations
| Function | Address | Description |
|----------|---------|-------------|
| `Matrix_TransformVec3` | `0x401D60` | 4×4 matrix × vector |
| `Matrix_TransformPoint2D` | `0x45C273` | 2D point transform |

### Math Utilities
| Function | Address | Description |
|----------|---------|-------------|
| `Gfx_PackColorRGB` | `0x401100` | Pack R,G,B into DWORD |
| `RNG_Rand` | inferred | Random number generator |
| `Wave_Sin` / `Wave_Cos` | inferred | Sine/cosine wave functions |
| `SQRT` | inferred | Square root |
| `ABS` | inferred | Absolute value |

---

## 16. Vtables & Calling Conventions

### Ball Vtable (`0x4CF3A0`)
| Slot | Offset | Function | Purpose |
|------|--------|----------|---------|
| 0 | `0x00` | `Ball_dtor` | Destructor |
| 1 | `0x04` | `Ball_Init` | Post-constructor setup |
| 4 | `0x10` | `Ball_Update` | Per-frame physics tick |
| 5 | `0x14` | `ApplyForceWithMultipliers` | Force with scale |
| 7 | `0x1C` | `CollisionHandler` | Custom collision response |
| 8 | `0x20` | `BounceCallback` | Called on `E:BREAK` events |
| 30 | `0x78` | `PreSplitCallback` | Before `Ball_Shatter` |
| 32 | `0x80` | `PostFactoryInit` | After bumper/sawblade creation |

### Scene/Board Vtable (`0x4D0260`)
| Slot | Offset | Function | Purpose |
|------|--------|----------|---------|
| 0 | `0x00` | `Scene_ctor` | Constructor |
| 4 | `0x10` | `Scene_Update` | Main game tick |
| 8 | `0x20` | `Scene_RunTick` | Single tick (Update+Render) |
| 10 | `0x28` | `Scene_Render` | Full render dispatch |
| 12 | `0x30` | `Scene_LoadLevel` | Level loading |
| 16 | `0x40` | `Scene_CleanupScene` | Cleanup |
| 18 | `0x48` | `Scene_StartRace` | Race countdown start |
| 19 | `0x4C` | `Scene_HandleRaceEnd` | Check finish |
| 20 | `0x50` | `Scene_UpdateBallsAndState` | Ball physics |
| 22 | `0x58` | `Scene_HandleCountdown` | Countdown timer |
| 24 | `0x60` | `RenderBackground` | Sky/dome |
| 25 | `0x64` | `RenderOpaque` | Opaque geometry |
| 26 | `0x68` | `RenderTransparent` | Glass/effects |
| 28 | `0x70` | `RenderOverlay` | HUD |
| 29 | `0x74` | `RenderPostEffects` | Fade/transition |
| 33 | `0x84` | `CreateDynamicObject` | Level-specific factory override |

### App Vtable (`0x4CE400`)
| Slot | Offset | Function | Purpose |
|------|--------|----------|---------|
| 0 | `0x00` | `App_ScalarDtor` | Destructor |
| 2 | `0x08` | `App_Shutdown` | Cleanup on exit |
| 8 | `0x20` | `Update` | Game logic (Scene_Update) |
| 9 | `0x24` | `PreRender` | Camera setup |
| 10 | `0x28` | `Render` | Draw scene |
| 11 | `0x2C` | `PostRender` | HUD / menus |
| 35 | `0x8C` | `SetDisplayMode` | Resolution change |
| 40 | `0xA0` | `ShowMainMenu` | Title screen |

### Calling Conventions
- `__thiscall`: ECX = `this` pointer. Used for all object methods.
- `__fastcall`: ECX = first arg, EDX = second arg. Used for inner loops.
- Standard cdecl: Stack-based args. Used for utility functions.

---

## Quick Reference: Most-Used Offsets

### Ball Struct (`0xC98` bytes for 8ball, `0xC60` for player, vtable `0x4CF3A0`)
| Offset | Type | Name | Description |
|--------|------|------|-------------|
| `+0x00` | void** | vtable | `0x4CF3A0` |
| `+0x04` | void* | scene | Parent Scene* |
| `+0x14` | int32 | player_index | -1=AI, 0=P1, 1=P2 |
| `+0x18` | char[0x14] | toggle_timer1 | ArenaBoard timer sub-object |
| `+0x60` | float[3] | position | Physics position (X, Y, Z) |
| `+0x150` | float | accumulated_time | Delta-time accumulator |
| `+0x158` | float[3] | prev_pos | Previous frame position |
| `+0x164` | float[3] | display_pos | Smoothed display position |
| `+0x170` | float[3] | velocity | Current velocity |
| `+0x17C` | float[3] | acceleration | Cleared each frame |
| `+0x188` | float | max_speed | Hard velocity cap (default 5.0) |
| `+0x18C` | float | speed_scale | Global speed multiplier (default 1.0) |
| `+0x1A4` | void* | collision_mesh | CollisionMesh ptr |
| `+0x1A8` | float[3] | gravity_vec | Gravity direction vector |
| `+0x1C8` | float | render_alpha | Render context alpha (0.75) |
| `+0x20C` | float[4] | color | RGBA tint (default 1,1,1,1) |
| `+0x254` | uint8 | uses_alpha | True if color_a != 1.0 |
| `+0x260` | uint8 | boost_hit_flag | Set on boost pad contact |
| `+0x278` | float | gravity_scale | Gravity multiplier (default 0.1) |
| `+0x281` | uint8 | unused_init_flag | DEAD: set to 1 in ctor, never read |
| `+0x284` | float | radius | Collision + render size (default 26.0) |
| `+0x2A4` | float | spin_rate | Angular spin factor (5.0) |
| `+0x2BC` | float[3] | force | Accumulated input force |
| `+0x2CC` | uint8 | force_disable | 1 = skip Ball_ApplyForce |
| `+0x2DC` | float[3] | checkpoint | Last safe position |
| `+0x2E8` | uint8 | splitting | Set during Ball_Shatter |
| `+0x2F0` | uint32 | force_count | Forces applied this frame |
| `+0x2F9` | uint8 | frozen | Stuck on surface |
| `+0x2FC` | uint32 | freeze_timer | Countdown while frozen |
| `+0x310` | uint8 | state_active | General active flag |
| `+0x318` | float | split_timer | Countdown for split balls (30.0) |
| `+0x31D` | uint8 | is_8ball | AI enable flag |
| `+0x324` | uint8 | dead | Eliminated / in-tube |
| `+0x748` | int32 | gravity_plane | 0=flat, 1=tilted, 2=vertical |
| `+0x758` | float[3] | camera_target | Camera orbit center |
| `+0x76C` | float[3] | camera_actual | Camera position (written by Scene_SetCamera) |
| `+0xC28` | char** | display_string | Floating text above ball |
| `+0xC3C` | uint8 | teleport_active | Teleport in progress |
| `+0xC40` | float[3] | teleport_dest | Destination coordinates |
| `+0xC4C` | uint8 | is_shrunk | Odd Race shrunk state (E:SHRINK=1, E:GROW=0) |
| `+0xC60` | int32 | battle_mode | 3=battle, 5=split |
| `+0xC68` | float | friction | 0.55 in battle mode |
| `+0xC6C` | float | bounciness | 1.0 in battle mode |
| `+0xC70` | float | max_speed_battle | 1000.0 in battle mode |
| `+0xC74` | float | chase_distance | AI chase radius (25.0 battle) |
| `+0xC80` | float[5] | gravity_vec_battle | Battle gravity (0,-1.0,0,...) |
| `+0xCA0` | float | speed_scale_split | 0.01 for split balls |
| `+0xCA4` | float[3] | collision_dir | Last collision direction |
| `+0xC88` | float[16] | world_matrix | 4×4 transform for rendering |

### Scene Struct (`~0x5400` bytes, vtable `0x4D0260`)
| Offset | Type | Name | Description |
|--------|------|------|-------------|
| `+0x0000` | void** | vtable | `0x4D0260` |
| `+0x0874` | byte | is_skydome | 0=skybox, 1=skydome |
| `+0x0878` | App* | scene_manager | D3D device / App back-pointer |
| `+0x087C` | void* | viewport_obj | D3D viewport |
| `+0x08B8` | AthenaList | scene_object_list | All scene objects |
| `+0x08BC` | int | scene_object_count | Number of objects |
| `+0x0CC4` | SceneObject** | scene_object_array | Direct pointer array |
| `+0x1518` | AthenaList | collision_list | Collision surfaces |
| `+0x2160` | AthenaList | ripple_list | Water ripples |
| `+0x29B0` | byte | ball_positions_dirty | Need propagate this frame |
| `+0x29B8` | int | shake_magnitude | Camera shake (-800→0) |
| `+0x29BC` | float | camera_orbit_angle | Y-axis rotation |
| `+0x29C0` | float | camera_distance | Orbit distance (max 700) |
| `+0x29D0` | Ball* | current_ball_ptr | Camera-tracked ball |
| `+0x29D4` | AthenaList | ball_list_1 | Player 1 balls |
| `+0x29D8` | int | ball_list_1_count | P1 ball count |
| `+0x2DE0` | Ball** | ball_list_1_array | P1 Ball pointer array |
| `+0x3204` | AthenaList | ball_list_2 | Player 2 balls |
| `+0x3208` | int | ball_list_2_count | P2 ball count |
| `+0x3610` | Ball** | ball_list_2_array | P2 Ball pointer array |
| `+0x361C` | SceneObject* | waypoint_arrow | Next checkpoint arrow |
| `+0x3620` | int | frame_counter | Total frames |
| `+0x362C` | AthenaList | player_list | Player viewport list |
| `+0x3630` | int | player_count | 0=none, 1=SP, 2=split |
| `+0x3A38` | Ball** | player_ball_array | Indexed by player (0–3) |
| `+0x3A48` | AthenaList | visible_object_list | Render bucket |
| `+0x3A4C` | byte | shake_active | Camera shake active |
| `+0x3AFC` | void* | dynamic_object | Post-update callback obj |
| `+0x3F1C` | byte | path_follow_mode | 1=camera rides spline rails |
| `+0x3F20` | void* | path_object | Spline Path* |
| `+0x3F24` | float | path_position | Parametric t (0.0–1.0) |
| `+0x3F2C` | int | camera_snap_frames | Snap countdown |
| `+0x434C` | float[3] | camera_offset | Added to ball pos |
| `+0x4358` | byte | demo_timer_active | Demo countdown running |
| `+0x435C` | int | demo_countdown | Frames remaining |
| `+0x436C` | void* | hammer_obj | Arena hammer |
| `+0x4370` | void* | saw1_obj | Saw blade 1 |
| `+0x4374` | void* | saw2_obj | Saw blade 2 |
| `+0x43A0` | float | damage_amount | From E:BITE |
| `+0x43A8` | int | damage_timer | Damage countdown |
| `+0x43B8` | void* | catapult_list | Catapult objects |
| `+0x47D0` | void* | door_list | Trapdoor list |
| `+0x4BBC` | void* | judge_list | Judge/score displays |
| `+0x4FD4` | void* | bell_obj | Bell (extra time) |

### App Struct (singleton at `0x4FD680`, vtable `0x4CE400`)
| Offset | Type | Name | Description |
|--------|------|------|-------------|
| `+0x000` | void** | vtable | `0x4CE400` |
| `+0x004` | HINSTANCE | hInstance | WinMain param |
| `+0x054` | RegKey* | registryKey | Registry handle |
| `+0x05C` | int | targetFPS | Target frame rate (30) |
| `+0x159` | bool | quitFlag | 1 = exit loop |
| `+0x15A` | bool | activeFlag | Window focused |
| `+0x158` | bool | minimizedFlag | Window minimized |
| `+0x156` | bool | updateDisabled | Pause all updates |
| `+0x15C` | int | width | Window width (800) |
| `+0x160` | int | height | Window height (600) |
| `+0x174` | Graphics* | graphics | D3D8 engine |
| `+0x17C` | AudioSystem* | audioSystem | BASS audio |
| `+0x180` | InputHandler* | inputHandler | DirectInput8 |
| `+0x184` | void* | gameUpdateObj | Passed to tick |
| `+0x1B4` | char* | versionString | ProductVersion |
| `+0x1CC` | int | loadedCount | Objects loaded |
| `+0x200` | bool | initialized | 1 after init |
| `+0x208` | char* | initStep | "Initialize(1)".."(26)" |
| `+0x224` | void* | mainMenuObj | MainMenu instance |
| `+0x228` | void* | resultsScreen | Race results |
| `+0x234` | bool | is_demo | Demo mode |
| `+0x237` | bool | is_2player | Multiplayer |
| `+0x238` | bool | rightButtonPause | Right-click pause |
| `+0x23C` | int | is_tournament | Tournament/arena active |
| `+0x278` | Texture* | shadowTexture | shadow.png |
| `+0x534` | HMUSIC | musicHandle | BASS music |
| `+0x538` | HCHANNEL | musicChannel1 | BASS channel 1 |
| `+0x53C` | HCHANNEL | musicChannel2 | BASS channel 2 |
| `+0x550` | void* | gameMode1 | 1-player mode |
| `+0x554` | void* | gameMode2 | 2-player mode |
| `+0x558` | void* | gameMode3 | 4-player mode |
| `+0x55C` | void* | gameMode4 | Tournament mode |
| `+0x5D8` | bool | p1_active | Player 1 active |
| `+0x5E8` | int | p1_current_time | P1 time/score |
| `+0x5EC` | int | p1_extra_time | P1 bonus time |
| `+0x60C` | int | p1_race_index | P1 race slot |
| `+0x678` | bool | p2_active | Player 2 active |
| `+0x688` | int | p2_current_time | P2 time/score |
| `+0x84C` | float | mouseSensitivity | 0.0–1.0 |
| `+0x850` | bool | mirrorMode | Tournament mirror |
| `+0x851`–`+0x868` | bool[24] | unlock_flags | Race/arena unlocks |
| `+0x86C` | uint8[0x50] | bestTimes | Per-level best times |
| `+0x8BC` | uint8[0x50] | medals | Per-level medals |
| `+0x914` | int | playCount | Total launches |
| `+0xB28` | DWORD | p2Controller1 | DI device index |
| `+0xB2C` | DWORD | p2Controller2 | DI device index |
| `+0xB30` | DWORD | p2Controller3 | DI device index |
| `+0xB34` | DWORD | p2Controller4 | DI device index |

---

## Document Info

- **Generated from:** Ghidra 12.0 decompilation + GhidraMCP REST API
- **Game version:** Hamsterball.exe (PE32, i386)
- **Total functions in binary:** ~3,800
- **Functions documented:** 120+
- **Decompilation sources:** `analysis/ghidra/decompilations/`
- **Struct headers:** `analysis/ghidra/structs/*.h`
- **Last updated:** 2026-06-13

### Cross-References
| Topic | Document |
|-------|----------|
| Arena scoring internals | `docs/ARENA_SCORING.md` |
| Arena hazards | `docs/ARENA_HAZARD_SYSTEM.md` |
| Ball object deep dive | `docs/BALL_OBJECT.md` |
| App singleton | `docs/APP_OBJECT.md` |
| Scene system | `docs/SCENE_SYSTEM_DECOMP.md` |
| Scene object modding | `docs/SCENE_OBJECT_MODDING.md` |
| Collision events | `docs/COLLISION_EVENT_SYSTEM.md` |
| Input system | `docs/DIRECTINPUT_SYSTEM.md` |
| Save/registry | `docs/SAVE_REGISTRY_SYSTEM.md` |
| Rendering pipeline | `docs/RENDERING_PIPELINE.md` |
| Camera system | `docs/CAMERA_SYSTEM.md` |
| 8-ball AI | `docs/8BALL_AI_SYSTEM.md` |
| UI menus | `docs/UI_MENU_SYSTEM.md` |
| MESHWORLD format | `docs/MESHWORLD_FORMAT.md` |
| Key decompilations | `docs/KEY_DECOMPILATIONS.md` |
| ArenaBoard system | `docs/ARENA_BOARD_SYSTEM.md` |
| Game state lifecycle | `docs/GAME_STATE_RACE_LIFECYCLE.md` |

---

*All offsets verified against raw Ghidra decompiled C. For questions, check the decompilation files in `analysis/ghidra/decompilations/`.*
