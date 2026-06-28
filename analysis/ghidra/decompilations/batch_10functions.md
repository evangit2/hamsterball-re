# Batch Decompilation: 10 New Functions (Session 2928)

**Date:** 2026-06-20
**Tool:** GhidraMCP v5.12.0-headless, Hamsterball.exe
**Selection criteria:** Named, undocumented functions spanning diverse game systems (arena hazards, physics, camera, UI, input, rendering).

---

## 1. Catapult_Update @ 0x0043E600

**Category:** Arena Hazard Physics (Catapult + Rotator objects — shared update)
**Called via:** vtable DATA ref at 0x004D5AFC
**Signature:** `__fastcall Catapult_Update(int *this)`

### Summary
Updates the Catapult/Rotator arena hazard — a rotating platform that carries balls and launches them. The catapult has a rotation timer at `this+0x43C` that decreases by `this+0x43D` (rotation speed) each frame. It applies the rotation matrix to all attached balls (via `AthenaList` at `this+0x43E`), transforming both their positions (`ball+0x164/0x168/0x16C` = X/Y/Z) and their velocity vectors (`CollisionMesh+0xCA4/CA8/CAC`).

This function serves double duty:
1. **Catapult launch system** — triggered by `E:CATAPULTBOTTOM`, launches ball via `Catapult_Launch` (0x434290)
2. **Rotator/gear system** — triggered by `N:ONROTATOR`/`N:SPINNY`/`N:SWIRL`, attaches ball via `Rotator_AddBall` (0x43B6F0) or `Catapult_AddObjectConditional` (0x43E9C0)

In both cases, the 8-byte ball tracking entry `[ball_ptr, tick_counter]` is decremented each frame. The tick counter starts at 10 and resets to 10 on every frame of continued collision contact (grace period, not carry limit). When it reaches 0 after the ball leaves the surface, the entry is freed.

### Key Struct Offsets
| Offset | Type | Description |
|--------|------|-------------|
| this+0x436..0x438 | float[3] | Catapult pivot position (X,Y,Z) |
| this+0x439 | float | Z-axis rotation amount |
| this+0x43A | float | X-axis rotation amount |
| this+0x43B | float | Y-axis rotation amount |
| this+0x43C | float | Current rotation angle |
| this+0x43D | float | Rotation speed (delta per frame) |
| this+0x43E | AthenaList | Attached balls list header |
| this+0x43F | int | Attached balls count |
| this+0x541 | void** | Ball pointer array |
| ball+0x164/168/16C | float[3] | Ball position (X,Y,Z) |
| ball+0x1A4 | int* | CollisionMesh pointer |
| CollisionMesh+0xCA4/CA8/CAC | float[3] | Ball velocity vector |

### Mechanism
1. Decrements rotation angle: `this->rotation -= this->rotation_speed`
2. Builds a rotation matrix from X/Y/Z scale components using `Gfx_ScaleX/Y/Z` and `Timer_Init`
3. For each attached ball: computes delta from catapult pivot, applies rotation matrix, updates ball position and velocity
4. Frees expired ball attachments (reference count drops to 0)
5. Calls vtable methods `+0x58` and `+0x54` (likely render and post-update hooks)

---

## 2. BounceBall_Update @ 0x00440840

**Category:** Arena Hazard Physics (BounceBall spawner)
**Called via:** vtable DATA ref at 0x004D5734
**Signature:** `__fastcall BounceBall_Update(int *this)`

### Summary
A bounce-pad hazard that launches balls into the air on a timer. It has two states:
- **State 0 (`this+0x43A == 0`):** Countdown mode — multiplies `this+0x439` (timer) by 1.5 each frame. When timer exceeds 120.0, it spawns a `FollowBall` entity (a ball that follows a predefined path), resets the timer to 120.0, and flips to state 1.
- **State 1 (`this+0x43A != 0`):** Cooldown mode — multiplies timer by 0.95 each frame. When timer drops below 1.0, resets to 0 and returns to state 0.

### Key Struct Offsets
| Offset | Type | Description |
|--------|------|-------------|
| this+0x434 | int* | Scene pointer |
| this+0x435..0x437 | float[3] | Bounce pad position |
| this+0x438 | float | Scale X (render) |
| this+0x439 | float | Countdown timer |
| this+0x43A | int | State flag (0=counting up, 1=counting down) |
| this+0x43B | byte | Dirty flag (position changed) |

### Spawn Mechanism
When the timer exceeds 120.0:
1. Calls `operator_new(0xC68)` — allocates a Ball struct (3144 bytes)
2. Calls `FollowBall_Ctor(ptr, scene, Level_FindObjectByName(scene, "BallPath"))`
3. Looks up "FOLLOWBALLSPOT" in the level's AthenaHashTable → gets spawn position
4. Sets ball position from hash table lookup
5. Sets ball velocity: `(-3.0, 10.0, 0.0)` (upward launch)
6. Sets `ball+0x80C = 0x0F` (flag/mode = 15)
7. Appends ball to scene's ball list at `scene+0x29D4`
8. Plays 3D sound at spawn position

### Constants
| Address | Value | Description |
|---------|-------|-------------|
| _DAT_004cf458 | 1.5 (f64) | Countdown multiplier (state 0) |
| _DAT_004d5dbc | 120.0 (f32) | Spawn threshold |
| _DAT_004d5d98 | 0.95 (f64) | Cooldown multiplier (state 1) |
| _DAT_004cf310 | 1.0 (f32) | Cooldown minimum |

---

## 3. Lifter_Update @ 0x0043B330

**Category:** Arena Hazard Physics (Lifter platform)
**Called via:** vtable DATA ref at 0x004D5544
**Signature:** `__fastcall Lifter_Update(int *this)`

### Summary
An oscillating elevator platform that moves up and down on the Z-axis, carrying attached balls. The lifter bounces between position bounds: when it exceeds +2.0, it reverses direction; when it drops below -2.0, it reverses again. It applies its vertical displacement to all balls on the platform.

### Key Struct Offsets
| Offset | Type | Description |
|--------|------|-------------|
| this+0x436..0x438 | float[3] | Lifter pivot position |
| this+0x439 | float | Accumulated vertical offset |
| this+0x43A | float | Current oscillation position |
| this+0x43B | float | Direction (-0.5 or +1.0) |
| this+0x43C | AthenaList | Attached balls list |
| this+0x43D | int | Attached balls count |
| this+0x53F | void** | Ball pointer array |

### Oscillation Logic
```
position += direction * 0.004   // _DAT_004d5c88 = 0.004
if position > 2.0: direction = -0.5   // _DAT_004cf48c = 2.0
if position < -2.0: direction = +1.0   // _DAT_004d5c84 = -2.0
```

### Ball Transform
Same matrix transform pattern as Catapult_Update — applies oscillation displacement to each attached ball's position and velocity through `Gfx_ScaleX`, `Matrix_TransformVec3`, and `Timer_Init/Cleanup`.

### Constants
| Address | Value | Description |
|---------|-------|-------------|
| _DAT_004d5c88 | 0.004 (f32) | Oscillation speed |
| _DAT_004cf48c | 2.0 (f32) | Upper position bound |
| _DAT_004d5c84 | -2.0 (f32) | Lower position bound |

---

## 4. ArenaSceneObj_Tick @ 0x0042B660

**Category:** Arena Scoring System (RumbleBoard timer)
**Called via:** vtable DATA ref at 0x004D3A5C
**Signature:** `__fastcall ArenaSceneObj_Tick(int param_1)`

### Summary
Per-frame tick for RumbleBoard (arena mode) decorative timers. Manages two oscillating timer fields used for visual effects on the arena board (likely animated score displays or arena hazard timing):

1. **Timer A (`this+0x880`):** Increments by 0.01 each frame. When it exceeds 0.75, resets to 0.75 (clamps at max).
2. **Timer B (`this+0x884`):** Decrements by 20.0 each frame. When it drops below 0.0, resets to 0.0 (clamps at min).
3. Calls `ToggleTimer_Tick` on two sub-timer fields at `this+0x888` and `this+0x89C`.

### Key Struct Offsets
| Offset | Type | Description |
|--------|------|-------------|
| this+0x880 | float | Timer A (ramps up to 0.75) |
| this+0x884 | float | Timer B (ramps down to 0.0) |
| this+0x888 | Timer | Sub-timer 1 |
| this+0x89C | Timer | Sub-timer 2 |

### Constants
| Address | Value | Description |
|---------|-------|-------------|
| _DAT_004cf524 | 0.01 (f32) | Timer A increment |
| _DAT_004cf438 | 0.75 (f64) | Timer A max |
| _DAT_004cf370 | 20.0 (f32) | Timer B decrement |
| _DAT_004cf368 | 0.0 (f32) | Timer B min |

---

## 5. Scene_UpdateArenaPhysics @ 0x00440390

**Category:** Arena Core Loop (Wave physics)
**Called via:** vtable DATA ref at 0x004D5484
**Signature:** `__fastcall Scene_UpdateArenaPhysics(int *this)`

### Summary
The main per-frame physics update for arena levels. Runs every 3rd frame (frame counter at `this+0x43E`, triggers when `counter > 2`, then resets to 0). This implements the arena's wave/surface physics:

1. **Frame gate:** `this->frame_counter++; if (counter <= 2) return;` — runs at 20fps on a 60fps game
2. **Wave computation:** For each of `this->num_waves` (at `this+0x43B`) wave segments:
   - Computes wave phase: `(frame * 360.0) * (i / num_waves) + base_offset`
   - Applies `Wave_Sin()` to get displacement amplitude × `this+0x43D` (wave height)
   - Updates vertex Z-offsets in mesh data arrays (two separate lists: `piVar5+0x102` and `piVar5+0x342E`)
3. **Ball physics:** Iterates arena balls (list at `this+0x6A99`):
   - For each ball, finds closest collision mesh point via `Mesh_FindClosestCollision`
   - If ball Y-position is within `this+0x43D` (wave height) of the surface, snaps ball Y to surface + ball radius (`ball+0x284`)
   - Ball radius stored at `CollisionMesh+0xC98/0xC9C/0xCA0`
4. **Vertex buffer upload:** Copies updated vertex data from `this+0x43F` to render buffer, calls vtable methods `+0x38` and `+0x3C` (likely `UnlockVertexBuffer` and `DrawPrimitive`)
5. **Wave sound:** Every 360th frame (`frame % 360 == 0`), plays a 3D wave sound at the arena center

### Key Struct Offsets
| Offset | Type | Description |
|--------|------|-------------|
| this+0x434 | int* | App/scene context pointer |
| this+0x435 | void* | Collision mesh |
| this+0x43B | int | Number of wave segments |
| this+0x43C | int | Frame counter (×360 for wave phase) |
| this+0x43D | float | Wave height amplitude |
| this+0x43E | int | Frame gate counter (mod 3) |
| this+0x43F | void* | Vertex buffer source |
| this+0x440 | int | Vertex count |
| this+0x6A99 | AthenaList | Arena ball list |
| this+0x6A9A | int | Arena ball count |
| this+0x6B9C | void** | Arena ball pointer array |
| this+0x120 | int* | Graphics/device context |
| ball+0x164/168/16C | float[3] | Ball position |
| ball+0x284 | float | Ball radius |
| ball+0x1A4 | int* | CollisionMesh |

### Constants
| Address | Value | Description |
|---------|-------|-------------|
| _DAT_004cf454 | 100.0 (f32) | Ball Y surface offset |
| _DAT_004cfecc | 25.0 (f32) | Wave displacement offset |
| _DAT_004d0418 | 180.0 (f32) | Wave frequency (degrees) |
| _DAT_004d03a0 | 90.0 (f32) | Wave phase offset (degrees) |

---

## 6. FollowBall_Update @ 0x0043ECC0

**Category:** Entity AI (FollowBall path follower)
**Called via:** vtable DATA ref at 0x004D4F64
**Signature:** `__fastcall FollowBall_Update(int *this)`

### Summary
Updates a FollowBall — a ball that follows a predefined path through the arena (spawned by BounceBall_Update). The FollowBall has two movement modes controlled by `this+0x441` (a boolean flag):
- **Mode 0 (`this+0x441 == 0`):** Normal speed — moves at 0.45 multiplier (or 0.09 if `this+0x43F > 0`)
- **Mode 1 (`this+0x441 != 0`):** Fast speed — decays spawn velocity by 0.0, clamps at 50.0 max

The FollowBall also manages a spawning cycle:
1. When the cycle timer (`this+0x43E`) reaches its threshold, it spawns a `RegisterDialog` entity (a score popup or registration marker) at a random position near the ball
2. The spawn position uses `RNG_Rand(0, 10)` for X/Z offset, with Z offset of 60.0
3. Spawns into the scene's dialog list at `scene+0x3B00`

### Key Struct Offsets
| Offset | Type | Description |
|--------|------|-------------|
| this+0x434 | int* | Scene pointer |
| this+0x435..0x437 | float[3] | Ball position |
| this+0x438..0x43A | float[3] | Ball position (mirror/offset) |
| this+0x43B | float | Animation timer (increments by 8.0) |
| this+0x43C | float | Current speed |
| this+0x43D | float | Target speed |
| this+0x43E | float | Spawn cycle timer |
| this+0x43F | float | Cycle threshold |
| this+0x440 | float | Cycle reset offset |
| this+0x441 | byte | Mode flag (0=normal, 1=fast) |
| this+0x442 | float | Spawn velocity decay |

### Constants
| Address | Value | Description |
|---------|-------|-------------|
| _DAT_004cf380 | 0.25 (f32) | Base position offset |
| _DAT_004cf4dc | 8.0 (f32) | Animation timer increment |
| _DAT_004cf4d0 | 0.0 (f32) | Spawn decay (mode 1) |
| _DAT_004cf3ec | 50.0 (f32) | Max spawn velocity |
| _DAT_004cf3c8 | 0.0 (f32) | Cycle threshold |
| _DAT_004cf3e0 | 0.0 (f32) | Lower cycle bound |
| _DAT_004d5d78 | 1.12 (f64) | Position base |
| _DAT_004d5d68 | 0.45 (f64) | Normal speed multiplier |
| _DAT_004d5d70 | 0.09 (f64) | Fast speed multiplier |
| _DAT_004d5d60 | 0.9 (f64) | Spawn decay (mode 0) |
| _DAT_004d5d58 | 1.75 (f64) | Cycle max |
| _DAT_004d5d50 | 0.0065 (f64) | Spawn velocity |
| _DAT_004d5d48 | 0.005 (f64) | Spawn velocity alt |
| _DAT_004d0930 | 60.0 (f32) | Z offset |
| _DAT_004d039c | 15.0 (f32) | Scale modifier |

---

## 7. Scene_HandleRaceEnd_ClampZoom @ 0x0041F7E0

**Category:** Race Camera (Finish-line zoom)
**Called via:** vtable DATA ref at 0x004D0EC4
**Signature:** `__fastcall Scene_HandleRaceEnd_ClampZoom(int this)`

### Summary
Called when a race ends. First calls `Scene_HandleRaceEnd(this)` to handle the actual race-finish logic, then clamps 8 zoom-related float values at `this+0x644C` through `this+0x6468` (every 4 bytes = 8 floats). Each value has `0.05` subtracted and is clamped to a minimum of `0.0`.

These 8 floats likely represent camera zoom parameters (field-of-view multipliers or distance values for up to 4 players in split-screen). The zoom-out animation at race end reduces each one by 0.05 per frame until they reach 0.0.

### Key Struct Offsets
| Offset | Type | Description |
|--------|------|-------------|
| this+0x644C | float | Zoom param 1 |
| this+0x6450 | float | Zoom param 2 |
| this+0x6454 | float | Zoom param 3 |
| this+0x6458 | float | Zoom param 4 |
| this+0x645C | float | Zoom param 5 |
| this+0x6460 | float | Zoom param 6 |
| this+0x6464 | float | Zoom param 7 |
| this+0x6468 | float | Zoom param 8 |

### Constants
| Address | Value | Description |
|---------|-------|-------------|
| _DAT_004cf428 | 0.05 (f64) | Zoom decrement per frame |
| _DAT_004cf6a8 | 0.0 (f32) | Minimum zoom clamp |

---

## 8. App_UpdateCullMode @ 0x00429040

**Category:** Graphics/Rendering (Cull mode toggle)
**Called via:** vtable DATA ref at 0x004D26D8
**Signature:** `__fastcall App_UpdateCullMode(int this)`

### Summary
A tiny function that syncs the D3D cull mode (backface culling) with the current graphics settings. Reads a cull-mode byte from `App+0x236` (a settings field — likely the "rendering quality" or "wireframe" toggle) and writes it to the graphics device's cull-mode field at `device+0x7D2`, then calls `Gfx_SetCullMode(device)` to apply it.

### Key Struct Offsets
| Offset | Type | Description |
|--------|------|-------------|
| App+0x174 | int* | Graphics device pointer |
| App+0x236 | byte | Desired cull mode (from settings) |
| device+0x7D2 | byte | Current cull mode field |

### Mechanism
```
device = App->graphics_device;
device->cull_mode = App->cull_setting;
Gfx_SetCullMode(device);
```

---

## 9. CreditsScreen_Render @ 0x00425AC0

**Category:** UI/Screens (Credits display)
**Called via:** vtable DATA ref at 0x004D2548
**Signature:** `__thiscall CreditsScreen_Render(void *this, void *param_1)`

### Summary
Renders the end-game credits screen. Draws a scrolling list of credit text entries with special formatting markers:

- **`'-'` prefix (0x2D):** Renders in a smaller, dimmer font (scale 0.5/0.5/1.0, color tint 0.75 alpha). Used for sub-credits (secondary roles).
- **`'*'` prefix (0x2A):** Renders in a different color/style (scale 1.0/1.0/0.5, color tint 0.5 alpha). Used for section headers. If the entry matches `this+0x864` (currently selected/highlighted entry), applies additional emphasis (scale 0.5/1.0/0.5).
- **No prefix:** Renders at default scale (1.0/1.0/1.0) with full color.

Each entry is drawn with `UI_DrawTextCenteredAbsolute` at X=400 (screen center), Y computed from `__ftol2` (entry index × line height). After rendering all entries, calls `UIList_Render` for the standard list scrollbar/border.

### Key Struct Offsets
| Offset | Type | Description |
|--------|------|-------------|
| this+0x44C | AthenaList | Credit entries list |
| this+0x450 | int | Entry count |
| this+0x858 | void** | Entry pointer array |
| this+0x864 | int | Highlighted entry index |
| this+0xCDc | int* | UI/font context |
| this+0xCE0 | float | Scroll animation value |

### Color/Scale Constants
| Hex Value | Float | Meaning |
|-----------|-------|---------|
| 0x3F800000 | 1.0 | Full scale/alpha |
| 0x3F400000 | 0.75 | Reduced scale/alpha |
| 0x425BA4 | — | String pointer (default color) |

---

## 10. KeyRemapMenu_WaitForKey @ 0x00443430

**Category:** Input System (Key rebinding)
**Called via:** vtable DATA ref at 0x004D5F54
**Signature:** `__fastcall KeyRemapMenu_WaitForKey(int *this)`

### Summary
Handles the "Press a key..." state in the key rebinding options menu. Has three code paths:

1. **Not waiting (`this+0x338 == 0`):** Calls `UIList_ScrollUpdate` to handle normal menu scrolling. Returns immediately.
2. **Debounce mode (`this+0x33A != 0`):** Clears the debounce flag, then scans the DirectInput keyboard state buffer (256 bytes) for any key with the high bit set (`& 0x80`). If any key is still pressed, re-sets the debounce flag and returns. This ensures the user releases the old key before pressing a new one.
3. **Capture mode (`this+0x338 != 0, this+0x33A == 0`):** Scans the 256-byte DirectInput keyboard state buffer. When it finds a key with high bit set (`& 0x80`), it:
   - Stores the DIK (DirectInput Key) scan code at `*this+0x339` (the key binding slot)
   - Clears the waiting flag (`this+0x338 = 0`)
   - Calls `KeyRemapMenu_UpdateKeyLabels` to refresh the UI
   - Sets a timer at `scene+0x560` to 0x32 (50) — likely a UI cooldown

### Key Struct Offsets
| Offset | Type | Description |
|--------|------|-------------|
| this+0x21E | int* | App/scene context |
| this+0x338 | byte | Waiting-for-key flag (1=capturing) |
| this+0x339 | int* | Key binding storage slot pointer |
| this+0x33A | byte | Debounce flag (1=waiting for key release) |
| App+0x180 | int* | Input device context |
| InputDevice+0x434 | int* | DirectInput keyboard state buffer |
| InputDevice+0x434+0xC | byte[256] | DIK key state array (high bit = pressed) |
| scene+0x560 | int | UI cooldown timer |

### DirectInput Scanning
The 256-byte keyboard state buffer is the standard DirectInput `IDirectInputDevice8::GetDeviceData` format. Each byte represents one DIK code; bit 7 (0x80) is the "pressed" flag. The function scans all 256 keys linearly.

---

## Summary

| # | Function | Address | System | Lines |
|---|----------|---------|--------|-------|
| 1 | Catapult_Update | 0x0043E600 | Arena Hazard | 141 |
| 2 | BounceBall_Update | 0x00440840 | Arena Hazard (spawner) | 99 |
| 3 | Lifter_Update | 0x0043B330 | Arena Hazard (elevator) | 149 |
| 4 | ArenaSceneObj_Tick | 0x0042B660 | Arena Scoring | 22 |
| 5 | Scene_UpdateArenaPhysics | 0x00440390 | Arena Wave Physics | 182 |
| 6 | FollowBall_Update | 0x0043ECC0 | Entity AI | 125 |
| 7 | Scene_HandleRaceEnd_ClampZoom | 0x0041F7E0 | Race Camera | 50 |
| 8 | App_UpdateCullMode | 0x00429040 | Graphics | 11 |
| 9 | CreditsScreen_Render | 0x00425AC0 | UI/Credits | 126 |
| 10 | KeyRemapMenu_WaitForKey | 0x00443430 | Input System | 38 |

**Total:** 943 lines of decompiled C documented.

### Architecture Notes
All 10 functions are called exclusively via **vtable DATA references** (virtual dispatch). None are called directly. This confirms the engine's C++-style polymorphism: each game object type (Catapult, BounceBall, Lifter, FollowBall, etc.) has its own vtable with `Update`, `Render`, and `Tick` virtual methods. The Scene's main loop iterates object lists and calls the appropriate vtable slot.

### Common Patterns
- **Timer_Init/Timer_Cleanup:** Structured exception handling (SEH) frame setup/teardown used as scoped profiling/timing wrappers.
- **AthenaList_NextIndex:** Iterator pattern for the engine's custom linked-list/array hybrid container.
- **Gfx_ScaleX/Y/Z:** These are NOT just scale operations — they build transformation matrix rows used for rotation/translation of attached objects.
- **`_DAT_` globals:** Mix of f32 and f64 constants. Ghidra doesn't distinguish — must read 8 bytes and try both interpretations.
