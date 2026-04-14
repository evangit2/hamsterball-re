# Scene System Deep Documentation

## Overview
The Scene system is the core game state manager for Hamsterball. Each game mode
(menus, arenas, tournament) has its own Scene instance with a vtable at 0x4D0260.
Scene controls the game tick, rendering, camera, physics pipeline, and object lifecycle.

## Key Addresses
| Function | Address | Purpose |
|----------|---------|---------|
| Scene_Update | 0x419C00 | Main game tick (called each frame) |
| Scene_Render | 0x41A2E0 | Render dispatch (1P/2P split) |
| Scene_SetCamera | 0x419FA0 | Camera positioning and tracking |
| Scene_dtor | 0x419770 | Destructor (cleanup order reference) |
| Scene_AddObject | 0x469990 | Add SceneObject to scene |
| Scene_CreateGameOverMenu | 0x40A920 | Pause/quit menu creation |
| Gear_AdvanceAlongPath | 0x418930 | Spline path follower with collision avoidance |

## Scene Struct Layout (Key Offsets)
Offsets in hex (byte addresses), computed from Ghidra int-indexed fields.

### Core State
```
+0x0000  vtable_ptr        (Scene vtable at 0x4D0260)
+0x0004  scene_base        (inherited fields from base class)
+0x0878  scene_manager     (D3D device/container, device at +0x174)
+0x087C  viewport_obj      (D3D viewport interface)
+0x3620  tick_count        (frame counter, incremented in Scene_Update)
```

### Demo Timer
```
+0x4358  demo_timer_active (bool)
+0x435C  demo_countdown    (int, counts down each frame)
+0x4360  demo_accumulator  (float, popup timing)
+0x4364  demo_frame_counter
+0x4368  demo_menu_suppressed (bool)
```

### Camera System
```
+0x434C  camera_offset     (Vec3: X/Y/Z offset from ball)
+0x4350  camera_offset_Y
+0x4354  camera_offset_Z
+0x3F1C  path_follow_mode  (bool, camera rails on spline)
+0x3F20  path_object       (Path* for camera rail)
+0x3F24  path_position     (float, parametric position)
+0x3F2C  camera_snap_frames (int, frames until snap-to-ball)
+0x29BC  camera_orbit_angle (float, Y-axis rotation)
+0x29C0  camera_distance   (float, orbit distance)
```

### Player/Ball Tracking
```
+0x0870  app_ptr           (back-pointer to App singleton)
+0x2190  ball_positions_dirty (bool flag)
+0x2194  ball_list_iterator
+0x2198  ball_list_count
+0x219C  ball_list_current
+0x2DE0  ball_list_array   (Ball** array)
+0x2DC0  ball_count_alt    
```

### Object Lists
```
+0x0884  rumble_timer_1    (RumbleBoard)
+0x0898  rumble_timer_2    (RumbleBoard)
+0x08B8  scene_object_list  (AthenaList<SceneObject*>)
+0x08BC  scene_object_count
+0x0CC4  scene_object_array
+0x362C  player_list        (AthenaList of player viewports)
+0x3630  player_count       (0=none, 1=single, 2=split)
+0x3A38  player_ball_array (Ball** array, indexed by player)
+0x29D0  current_ball_ptr   (Ball being tracked by camera)
```

### Rumble/Haptics
```
+0x3A4C  rumble_active      (bool)
+0x29B8  rumble_intensity   (int, starts -800, decays by -10/frame)
```

### Physics Pipeline vtable Calls
Called from Scene_Update via vtable when not in single-gear mode:
```
vtable[0x4C] = Scene_HandleRaceEnd    (0x41B130) - check race finish conditions
vtable[0x50] = Scene_UpdateBallsAndState (0x41B540) - ball physics + respawn + waypoints
vtable[0x54] = NoOp                   (0x40A040) - stubbed (collision handling is in Ball vtable)
vtable[0x58] = Scene_HandleCountdown   (0x41A540) - race countdown timer
vtable[0x7C] = Scene_LevelObjUpdate   (0x41AC70) - level object tick (traps, moving platforms)
```

## Level vtable Entries (from Scene vtable map)

### Level_UpdateAndRender (0x40B600, vtable[0x64])
Main level render: two-pass system (opaque + alpha).
1. Clear visible_object_list, append player balls
2. SetRenderState(ALPHA_BLEND=FALSE) → opaque pass
3. For each ball: ball->vtable[0x1C]() = RenderOpaque
4. SetRenderState(ALPHA_BLEND=TRUE) → alpha pass
5. If race active: update waypoint arrow + render
6. For each visible object: obj->vtable[0x08]() = Render
7. Ball_RenderShadow for each ball (if shadow data exists)
Scene offsets: +0x29D4=P1 balls, +0x3204=P2 balls, +0x3A48=visible objects, +0x361C=waypoint

### Level_RenderObjects (0x40B570, vtable[0x68])
Transparent pass. Graphics_BeginFrame, scene_manager->vtable[0x4C]
for level geometry, then obj->vtable[0x0C]() = RenderTransparent per visible object.

### Level_RenderDynamicObjects (0x40B420, vtable[0x60])
Sky/dome + water ripples. If is_skydome_enabled (+0x3A44):
sky_dome_mesh->vtable[0x48](1,1), else level_mesh->vtable[0x48]().
Then iterate ripple_list (+0x2160), render each:
Gfx_SetPositionAndRender→ScaleX→ScaleZ→SetPosition→WaterRipple_Render.
Finally dynamic_object->vtable[8]() callback.

### Scene vtable Full Map (0x4D0260)
```
[0x00] 0x425020  Scene_ctor
[0x04] 0x419C00  Scene_Update          (main game tick)
[0x08] 0x41A2E0  Scene_Render           (1P/2P dispatch)
[0x0C] 0x4692F0  Scene_dtor_thunk
[0x10] 0x469220  Scene_Release
[0x14] 0x4130A0  Scene_SetupLevel
[0x18] 0x469280  Scene_SceneObjThunk1
[0x1C] 0x409D90  Scene_Init
[0x20] 0x40B400  Scene_RunTick
[0x24] 0x44B840  Scene_NoOp1
[0x28] 0x44B840  Scene_NoOp2
[0x2C] 0x4692A0  Scene_SceneObjThunk2
[0x30] 0x4692A0  Scene_SceneObjThunk3
[0x34] 0x44B840  Scene_NoOp3
[0x38] 0x409DA0  Scene_LoadLevel
[0x3C] 0x469430  Scene_SceneObjThunk4
[0x40] 0x419740  Scene_CleanupScene
[0x44] 0x4692B0  Scene_SceneObjThunk5
[0x48] 0x40B090  Scene_StartRace
[0x4C] 0x41B130  Scene_HandleRaceEnd
[0x50] 0x41B540  Scene_UpdateBallsAndState
[0x54] 0x40A040  Scene_NoOp_Collision (stub)
[0x58] 0x41A540  Scene_HandleCountdown
[0x5C] 0x409DE0  Scene_?? (0x409DE0)
[0x60] 0x40B420  Level_RenderDynamicObjects
[0x64] 0x40B600  Level_UpdateAndRender
[0x68] 0x40B570  Level_RenderObjects
[0x6C] 0x41B710  Scene_RenderOverlay
[0x70] 0x41BFD0  Scene_RenderPostEffects
[0x74] 0x40C5D0  Scene_?? (0x40C5D0)
[0x78] 0x44B840  Scene_NoOp4
[0x7C] 0x41AC70  Scene_LevelObjUpdate
[0x80] 0x41C5B0  Scene_?? (0x41C5B0)
[0x84] 0x419750  Scene_?? (0x419750)
[0x88] 0x44B840  Scene_NoOp5
[0x8C] 0x41A9A0  Scene_?? (0x41A9A0)
```

### Level Objects
```
+0x3624  level_object_list  (AthenaList of level objects)
+0x362C  level_object_count
+0x3630  level_object_iter
+0x3A38  level_object_array_ptr
+0x3AFC  post_update_callback_obj (vtable[1] called after all updates)
```

## Game Tick Execution Order (Scene_Update)

```
1. Increment tick_count
2. Demo timer check:
   - If demo_timer_active: decrement countdown
   - On countdown=0: spawn "end of demo" popup via ScoreDisplay_CtorC
3. Pause/escape check:
   - If not menu mode (3/4) AND not pause_suppressed:
     - Check ESC key via Input_CheckKeyCombo(app, 2)
     - Create GameOverMenu if pressed
4. Ball position propagation:
   - If ball_positions_dirty flag set:
     - Iterate ball list, call Ball_SetTargetPos for each
     - Clear dirty flag
5. Gear path following:
   - If exactly 1 gear object AND gear_has_active_path:
     - Read camera position from app->camera_obj
     - Call Gear_AdvanceAlongPath(gear, camX, camY, camZ)
6. Rumble board timers:
   - Tick both RumbleBoard instances
7. Rumble intensity decay:
   - If rumble_active: decay intensity from -800 toward 0 by 10/frame
8. SceneObject update+render loop:
   - For each SceneObject in list:
     - Call vtable[1]() = Update
     - If render_flag set: Gfx_SetRenderState, then vtable[0]() = Render
9. Game state pipeline (4 vtable calls):
   - Scene_HandleRaceEnd -> Scene_UpdateBallsAndState -> NoOp -> Scene_HandleCountdown
   - HandleRaceEnd: check if race won/time expired
   - UpdateBallsAndState: per-ball physics tick + OOB respawn + waypoint progression
   - NoOp: collision stub (collision is in Ball vtable update)
   - HandleCountdown: race start countdown timer
   - Skip if in single-gear camera-locked mode
10. Level object update:
    - For each level object: vtable[0x7C]()
11. Post-update callback
```

## Render Pipeline (Scene_Render)

### Single Player (player_count == 1)
```
Graphics_SetViewport(full_screen)
  -> Copy ball->pos to camera target
  -> Scene_SetCamera(scene, ball, apply_path=true)
  -> vtable[0x60] Level_RenderDynamicObjects (sky/far plane)
  -> vtable[0x64] Level_UpdateAndRender (level geometry + dynamics)
  -> vtable[0x68] Level_RenderObjects (transparent/glass objects)
Graphics_SetViewport(full_screen)
  -> vtable[0x70] Scene_RenderPostEffects  (fade, transitions)
  -> vtable[0x6C] Scene_RenderOverlay (HUD, score)
```

### Split Screen (player_count == 2)
```
For each player:
  Graphics_SetViewport(half_screen_for_player)
  -> Scene_SetCamera(scene, player_ball, apply_path=true)
  -> vtable[0x60] RenderBackground
  -> vtable[0x64] RenderOpaqueObjects
  -> vtable[0x68] RenderTransparentObjects

Graphics_SetViewport(full_screen)
  -> vtable[0x70] RenderOverlay (shared HUD)
  -> vtable[0x6C] RenderPostEffects
```

## Camera System (Scene_SetCamera)

### Camera Modes
1. **Default Follow**: Camera positioned at ball->camera_target_pos + camera_offset
2. **Path Following**: Camera rails along spline, springs back if ball wanders
3. **Camera Shake**: Random position jitter when ball->camera_shake_enabled
4. **Camera Snap**: Instant reposition over N frames (teleport transitions)
5. **Orbit Mode**: Rotates around ball using orbit_angle (menu screens)

### Path Following Algorithm
- When path_follow_mode is active:
  - Get current position on spline at path_position
  - Compute distance from camera to path center
  - If distance > threshold: apply spring force
    - Oscillation via sin wave (Wave_Sin)
    - Damping factor prevents infinite oscillation
    - Max pull strength: 700 units
  - Blend camera position toward path center
  - Update ball target position

## App_Run Game Loop (0x46BD80)

```
while (!quit):
  Sleep(0)  // yield to OS
  tick_interval = 1000 / target_fps
  
  // FPS counter (1Hz update)
  if (time > fps_timer + 1000):
    update_fps_display()
    reset frame counter
  
  // Message pump
  while (PeekMessage()):
    TranslateMessage + DispatchMessage
    if (quit) break
  
  // Tick + Render
  do:
    elapsed = GetTickCount() - last_tick
    if (elapsed < tick_interval - 5) || (skip_count > 9):
      // Render only
      if (time > render_deadline):
        Graphics_BeginFrame()
        vtable[0x24] Scene_Render  // render-only
        vtable[0x28] Scene_Present
        vtable[0x2C] Scene_PostFrame
        Graphics_PresentOrEnd()
      break
    
    // Full tick
    frame_count++
    Graphics_BeginFrame()
    vtable[0x20] Scene_Update    // full game tick
    last_tick += tick_interval
    if (accumulated lag > 1000ms):
      last_tick = now - 1000  // prevent spiral of death
  while (updates_this_frame < 1)
```

Key insight: The game uses a fixed-timestep update with variable rendering.
If the game falls behind, it caps accumulated lag at 1000ms to prevent
the "spiral of death" where too many updates cause more lag.