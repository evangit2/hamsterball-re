# Level Rendering System Documentation

## Overview
The Level rendering system handles the visual pipeline for each frame. Three vtable
functions handle different stages: background/sky, geometry+dynamic objects, and
transparent overlays. Ball_Update also calls into rendering for score popups.

## Key Addresses
| Function | Address | Purpose |
|----------|---------|---------|
| Level_UpdateAndRender | 0x40B600 | Main level render: opaque pass, waypoint, visible objects, shadows |
| Level_RenderObjects | 0x40B570 | Transparent object pass (vtable[0xC]) |
| Level_RenderDynamicObjects | 0x40B420 | Sky/dome, water ripples, dynamic effects |
| Scene_SpawnBallsAndObjects | 0x41C5B0 | Level startup: create balls, pickups, signs, traps |
| Scene_CheckPath | 0x457EC0 | Ring topology path connectivity check |
| SpatialTree_ctor | 0x463330 | Collision acceleration tree constructor |
| Ball_RenderShadow | (varies) | Shadow rendering per ball |

## Scene Struct Offsets (Level Rendering)

### Ball Lists
```
+0x29D4  ball_list_1 (AthenaList<Ball*>, player 1)
+0x29D8  ball_list_1_count
+0x29DC  ball_list_1_iterator
+0x2DE0  ball_list_1_array (Ball** ptr)
+0x3204  ball_list_2 (AthenaList<Ball*>, player 2)
+0x3208  ball_list_2_count
+0x320C  ball_list_2_iterator
+0x3610  ball_list_2_array (Ball** ptr)
```

### Object/Render Lists
```
+0x3A48  visible_object_list (AthenaList, rebuilt each frame by UpdateAndRender)
+0x878   app_ptr (App* back-pointer)
+0x8AC   scene_manager (contains level mesh, D3D device)
+0x8B0   sky_dome_mesh (rendered when is_skydome_enabled)
+0x2160  ripple_list (AthenaList of water ripples)
+0x2164  ripple_count
+0x2168  ripple_iterator
+0x256C  ripple_array_ptr
+0x3F18  water_ripple_system (WaterRipple renderer)
+0x3A44  is_skydome_enabled (bool: false=level mesh bg, true=sky dome)
+0x3AFC  dynamic_object (vtable[8] callback after ripples)
+0x361C  waypoint_display_obj (arrow/marker for next checkpoint)
+0x070C  alpha_blend_state (0=opaque pass, 1=alpha pass)
+0x07C8  render_pass_counter (incremented each state change)
```

### App Offsets (accessed via app_ptr)
```
app+0x154  = d3d_device_ptr (for SetRenderState)
app+0x178  = sound_system_ptr (contains camera info)
app+0x220  = current_game_mode (mode struct: +0x08=mode_type, +0x11=race_active, etc.)
app+0x234  = race_paused (bool)
app+0x237  = is_demo_version (bool)
app+0x23C  = tournament_active (int, non-zero = tournament)
app+0x850   = player_count (updated from ball list size)
app+0x910   = waypoint_list_ptr
```

## Rendering Pipeline (per frame)

```
Scene_Render (0x41A2E0):
├── For each player (1 or 2):
│   ├── Graphics_SetViewport(player_rect)
│   ├── Scene_SetCamera(scene, ball, true)
│   ├── vtable[0x60] Level_RenderDynamicObjects  ← sky/dome + water ripples
│   ├── vtable[0x64] Level_UpdateAndRender         ← main geometry pass
│   │   ├── Clear visible_object_list
│   │   ├── Append all player balls to visible list
│   │   ├── SetRenderState(ALPHA_BLEND, FALSE)  → opaque pass
│   │   ├── For each ball: ball->vtable[0x1C]()  → RenderOpaque
│   │   ├── SetRenderState(ALPHA_BLEND, TRUE)   → alpha pass
│   │   ├── If race active: WaypointList update + render arrow
│   │   ├── For each visible obj: obj->vtable[0x08]()  → Render
│   │   └── If shadows: Ball_RenderShadow per ball
│   └── vtable[0x68] Level_RenderObjects           ← transparent overlay
│       ├── BeginFrame(device, 0)
│       ├── scene_manager->vtable[0x4C]()  → level geometry
│       └── For each visible obj: obj->vtable[0x0C]()  → RenderTransparent
└── Final:
    ├── Graphics_SetViewport(full_screen)
    ├── vtable[0x6C] Scene_RenderOverlay  ← HUD/score
    └── vtable[0x70] Scene_RenderPostEffects  ← fade/transitions
```

## Level_StartBalls (Scene_SpawnBallsAndObjects, 0x41C5B0)

### Ball Spawn Process
```
1. Set player count in app->sound_system at +0x850
2. For each entry in level_start_list (+0xD8B):
   a. Lookup "START%d-%d" in hash table for spawn position (Vec3)
   b. If demo version: also check "START%d-%d" alternate position
   c. If single player AND special level (types 5,0xB,0xC,0xE):
      Random choose between "START2-1" and "START2-2"
   d. If "START-DEBUG" exists in hash table, use that position
   e. Ball_ctor2(0xC60 bytes) → init physics defaults
   f. Ball_SetTrajectory → set trajectory from level data
   g. Set: player_index, gravity_scale=0.5, radius=26.0,
           max_speed=5.0, is_falling=false, field_0x769=true
   h. AthenaList_Append to ball_list (+0xA75)
   i. Store ball pointer in level_start_entry+0x10
3. Scan object list for "SAFESPOT"/"SAFEPOS" entries → append to safe_list
4. If demo or tournament: CreateBadBall, CreateMouseTrap
5. CreateSecretObjects, Scene_CreateFlags, Scene_CreateSigns
6. Scene_CreateDynamicObjects
```

### Ball Consts at Spawn
- Ball struct size: 0xC60 (3168 bytes)
- gravity_scale: 0.5 (1.0 = normal, 0.5 = half)
- field_0x27C: 0x3DCCCCCD = 0.1f (ball weight/mass?)
- radius: 26.0 (game units)
- field_0x1A0: 0x3F866666 ≈ 1.05f (collision radius scale?)
- max_speed: 5.0 (game units/tick)

## Scene_CheckPath (0x457EC0)

Simple ring topology pathfinder for the game's 360-cell (0x167) circular grid.

```
Scene_CheckPath(from, to):
  iterations = 0
  forward = from, backward = to
  while iterations < 0x167 (359):
    if forward == to: return 1  (reachable forward)
    if backward == to: return -1 (reachable backward)
    forward = (forward + 1) % 0x167
    backward = (backward - 1 + 0x167) % 0x167
    iterations++
  return 0  (unreachable - shouldn't happen)
```

Used in Ball_Update for grid-based collision/track snapping.
The 360-cell grid matches angular positions (0-359 degrees).

## SpatialTree (Collision Acceleration)

| Function | Address | Purpose |
|----------|---------|---------|
| SpatialTree_ctor | 0x463330 | Create tree from object list |
| SpatialTree_SetDefaults | 0x4632F0 | Reset tree parameters |
| SpatialTree_Free | 0x4632E0 | Free tree memory |
| SpatialTree_DeletingDtor | 0x4633B0 | Destructor |
| SpatialTree_ForEach | 0x463880 | Iterate leaves |
| SpatialTree_CloneToLevel | 0x462380 | Clone for collision testing |
| SpatialTree_ComputeOffset | 0x466480 | Compute spatial offset |
| Collision_TraverseSpatialTree | 0x465EF0 | Main collision query |

SpatialTree is an AABB tree used for O(log N) collision detection.
Ball_Update creates one per frame: SpatialTree_ctor(mesh_list), then
Collision_TraverseSpatialTree to find collision candidates.