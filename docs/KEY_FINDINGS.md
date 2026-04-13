# Hamsterball RE — Key Findings Summary

## Session Date: April 12, 2026

## 1. Game Loop (App::Run)

**Address:** 0x46BD80

**Function Signature:**
```c
void App::Run(App* this);  // ECX = App* (0x4FD680)
```

**Key Offsets:**
| Offset | Field | Description |
|--------|-------|-------------|
| +0x158 | minimized | 1 if minimized |
| +0x159 | running | 1 to quit |
| +0x15A | active | 1 if game active |
| +0x164 | last_tick | GetTickCount() |
| +0x168 | frame_time | 1000 / target_fps |
| +0x170 | target_fps | e.g., 60 |
| +0x174 | graphics | Graphics* |
| +0x18C | frame_counter | frames since last FPS reset |
| +0x194 | frame_count | FPS display counter |
| +0x198 | fps_buffer | FPS string display |
| +0x1AC | show_fps | 1 to show FPS |

**Loop Logic:**
1. PeekMessage loop (message pump)
2. If elapsed < frame_time - 5ms: skip (count skipped frames)
3. After 10 skips: force update
4. Otherwise: Update() → Draw() → Present()
5. Sleep/yield between frames

---

## 2. Ball Physics

### Ball Vtable (0x4CF3A0)

| Index | Offset | Address | Function | Description |
|-------|--------|---------|----------|-------------|
| 0 | +0x00 | 0x4027F0 | Ball_dtor | Cleanup and free |
| 1 | +0x04 | 0x405100 | Ball_Update | Per-frame physics init |
| 2 | +0x08 | 0x402DE0 | Ball_CollisionCheck | Check collisions |
| 3 | +0x0C | 0x402A70 | OnCollision | Handle collision event |
| 4 | +0x10 | 0x408390 | Ball_Render | Render the ball |
| 5 | +0x14 | 0x401590 | vtable[5] | Calls vtable[6] with extra param |
| 6 | +0x18 | 0x402650 | Ball_ApplyForce | Apply force/velocity |
| 7 | +0x1C | 0x402C10 | ??? | Physics plane check |
| 8 | +0x20 | 0x409480 | ??? | Unknown |

### Ball Object Structure

| Offset | Type | Field | Description |
|--------|------|-------|-------------|
| +0x164 | float | x | Position X |
| +0x168 | float | y | Position Y |
| +0x16C | float | z | Position Z |
| +0x170 | float | vx | Velocity X |
| +0x174 | float | vy | Velocity Y |
| +0x178 | float | vz | Velocity Z |
| +0x17C | float | ang_vel_z | Angular velocity Z |
| +0x180 | float | ang_vel_y | Angular velocity Y |
| +0x184 | float | ang_vel_x | Angular velocity X |
| +0x198 | float | facing_angle | Movement direction |
| +0x19C | char | facing_valid | Angle computed |
| +0x278 | float | ??? | 0.5 (set by Update dispatcher) |
| +0x27C | float | friction | 0.2 (set by Update dispatcher) |
| +0x280 | char | ??? | Collision flag |
| +0x284 | float | radius | **35.0** (ball radius) |
| +0x2CC | char | no_input | No player input |
| +0x2E8 | char | flag1 | Reset to 0 each frame |
| +0x2E9 | char | flag2 | Reset to 0 each frame |
| +0x2EC | uint32 | ??? | Reset to 0 each frame |
| +0x2F0 | uint32 | force_counter | Forces applied this frame |
| +0x2F8 | char | update_in_progress | Set to 1 during update |
| +0x2F9 | char | frozen | Ball frozen (on surface) |
| +0x2FC | uint32 | freeze_timer | Countdown when frozen (150) |
| +0x310 | char | ??? | Set to 1 each frame |
| +0x314 | float | ang_vel_y | Set to 0 on snap |
| +0x318 | float | ang_vel_x | Set to 0 on snap |
| +0x31C | char | ??? | Set to 0 on snap |
| +0x31D | char | ??? | Render flag |
| +0x324 | char | in_tube | Ball is in tube/pipe |
| +0x748 | int | gravity_plane | 0=XY, 1=Y-tilted, 2=XZ |
| +0xC4C | char | is_dizzy | Dizzy effect |
| +0xC5C | int | on_ice | Ice surface |
| +0xC74 | int | collision_count | Reset to 0 each frame |
| +0xC78 | float | ??? | 0.0 (set by Update dispatcher) |
| +0xC7C | float | ??? | 50.0 (set by Update dispatcher) |
| +0xC6C | float | ??? | 600.0 (set by Update dispatcher) |
| +0xC70 | float | ??? | 1200.0 (set by Update dispatcher) |
| +0xC80 | char | ??? | Has special collision |
| +0xC88-+0xC94 | float[4] | special_col | Special collision data |

### Ball_ApplyForce (0x402650)

```c
void Ball_ApplyForce(ball, force_x, force_y, force_z, magnitude);
```

**Logic:**
- If frozen or no input: ignore
- Apply force multiplier based on:
  - force_counter (first frame = 1.0, then *0.25)
  - in_tube (*0.0 — no force in tube!)
  - on_ice (*0.2 + ice friction 6.0)
  - is_dizzy (*0.75)
- Accumulate into velocity at +0x170/0x174/0x178
- Compute facing angle based on gravity plane

### Ball_Update (0x405190)

The main per-frame physics update:
1. If in_tube: skip (return immediately)
2. Reset per-frame state
3. Find closest collision surface based on gravity_plane:
   - Plane 0: XY (standard), check surfaces with [Z] tag
   - Plane 1: Y-tilted, check surfaces with [X] tag
   - Plane 2: XZ, check surfaces with [Z] tag
4. If surface found: snap position to surface, zero velocity, set frozen=1
5. If no surface: ball falls freely

### Global Constants

| Address | Float | Value | Usage |
|---------|-------|-------|-------|
| 0x4CF368 | 0.0 | epsilon | Float comparison epsilon |
| 0x4CF36C | 0.75 | dizzy_mult | Force multiplier when dizzy |
| 0x4CF374 | 0.2 | ice_mult | Force multiplier on ice |
| 0x4CF378 | 0.0 | tube_mult | Force multiplier in tube |
| 0x4CF380 | 0.25 | force_mult | Force multiplier after 1st frame |
| 0x4CF39C | 0.037 | radius_mult | Collision radius multiplier |
| 0x4CF3E8 | 6.0 | ice_friction | Ice friction factor |
| 0x4CF484 | 40.0 | col_mesh_dist | Collision mesh distance threshold |
| 0x4CF48C | 2.0 | y_threshold | Y offset threshold (radius + epsilon) |

---

## 3. Binary MESHWORLD Format

**Format:**
```
[uint32: object_count]
[uint32: str_len][type_string][object_data...]
[uint32: str_len][type_string][object_data...]
...
```

**Object Data Sizes:**
- **Simple objects** (START, SAFESPOT): 28 bytes after type string
  - 3 floats: position (x, y, z)
  - 4 uint32: rotation/flags
- **Complex objects** (FLAG, PLATFORM): 76+ bytes
  - Transform matrix, colors, size param
  - Texture reference string
  - Face index data

**Object Types Found:**
- START1-1, START2-1, START2-2 (spawn points)
- FLAG02, FLAG04, FLAG06, FLAG07 (checkpoints)
- SAFESPOT (safe zones)
- PLATFORM, N:SINKPLATFORM (platforms)
- N:BUMPER1-4 (bumpers)
- E:NODIZZY<TIME>N</TIME> (dizzy zones)
- E:LIMIT (boundaries)
- CAMERALOOKAT (camera points)
- BADBALL (enemies)
- BCMESH (ball mesh reference)
- S:Walls(NOSHADOW) (walls)

---

## 4. Binary MESH Format (3D Models)

**Header:**
```
[uint32: version = 1]
[uint32: name_len]
[name_len bytes: model name]
[material data: ambient, diffuse, specular, etc.]
[uint32: texture_name_len]
[texture_name_len bytes: texture filename]
[uint32: vertex_count]
[vertex data: vertex_count * 32 bytes]
[face data: ...]
```

**Vertex Format (32 bytes):**
```
[float: x][float: y][float: z]     // position
[float: nx][float: ny][float: nz]  // normal
[float: u][float: v]               // texture coordinates
```

---

## 5. Key Functions Decompiled

| Address | Function | Lines | Description |
|---------|----------|-------|-------------|
| 0x46BD80 | App::Run | 91 | Game loop |
| 0x429530 | App::Initialize | ~200 | App init |
| 0x405190 | Ball_Update | 477 | Main physics |
| 0x405100 | Ball_Update_disp | 27 | Physics init dispatcher |
| 0x402650 | Ball_ApplyForce | 47 | Force/velocity |
| 0x402DE0 | Ball_CollisionCheck | 99 | Collision check |
| 0x402A70 | OnCollision | 64 | Collision event |
| 0x402C10 | ??? | 68 | Physics plane check |
| 0x402860 | Ball_Render | 41 | Ball render stub |
| 0x408390 | Ball_Render | 166 | Actual ball render |
| 0x40AFE0 | Ball_ctor | 30 | Ball constructor |
| 0x455380 | Graphics_Initialize | 272 | D3D init |
| 0x470930 | MeshWorld_Parse | 787 | ASE text parser |
| 0x4629E0 | Binary_mesh_loader | 289 | Binary loader |

---

## 6. Virtual Tables

### App vtable (0x4D8FB0)
- +0x00: Destructor
- +0x0C: LoadMesh
- +0x30: LoadMeshWorld  
- +0x38: Binary mesh loader

### Ball vtable (0x4CF3A0)
- +0x00: dtor (0x4027F0)
- +0x04: Update dispatcher (0x405100)
- +0x08: CollisionCheck (0x402DE0)
- +0x0C: OnCollision (0x402A70)
- +0x10: Render (0x408390)
- +0x14: vtable[5] (0x401590)
- +0x18: ApplyForce (0x402650)
- +0x1C: vtable[7] (0x402C10)
- +0x20: vtable[8] (0x409480)

---

## 7. Files Generated

- `analysis/ghidra/decomp_*.c` — Full decompilations (30+ files)
- `docs/GAME_LOOP_DECOMP.md` — App::Run details
- `docs/BALL_UPDATE_DECOMP.md` — Ball physics details
- `docs/MESHWORLD_FORMAT.md` — Format spec
- `docs/KEY_FINDINGS.md` — This summary
- `reimpl/src/level/meshworld_parser.c` — Working parser
- `reimpl/build/level_viewer` — SDL2+OpenGL viewer

---

## 8. Statistics

- **Total decompiled C code:** ~2500 lines
- **Functions decompiled:** 15+
- **Object types found:** 20+
- **Level files parsed:** 86

---

*Last updated: April 13, 2026*
*Project: Hamsterball RE (Raptisoft, 2000s)*
*Permission: Original developer granted permission for RE*

---

## 9. Scene System (April 13, 2026)

### Scene Architecture

The game uses a virtual-dispatch scene system. Each race level creates a Scene object with a vtable that provides:
- **dtor** (+0x00): Master cleanup (Scene_dtor at 0x419770)
- **vmethods +0x4C-0x58**: Update render passes (PreRender, RenderLevel, RenderDynamic, PostRender)
- **vmethod +0x60/0x64/0x68**: Render3D, RenderObjects, RenderOverlay
- **vmethod +0x6C/0x70**: RenderMenu, RenderHUD
- **vmethod +0x7C/0x80**: Per-dynamic-object update, per-scene init

Scene_Update (0x419C00) is the main tick:
1. Increment frame counter (+0xD88)
2. Demo timer check (50-frame countdown, shows buy dialog)
3. Unpause check (Input_CheckKeyCombo mode 2 for ESC)
4. Ball position sync from +0xA75 list
5. Camera tracking (ball at App+0x5DC)
6. Screen offset animation (+0xA6E, -10/frame toward 0 or -800)
7. Static object update + removal of dead objects (+0x22E list)
8. Render pass vmethods (+0x4C, +0x50, +0x54, +0x58)
9. Dynamic object iteration (+0xD8B list)

Scene_Render (0x41A2E0) dispatches by player count:
- **1P**: Single viewport, all render passes
- **2P**: Set camera from player list at +0x3A38
- **3-4P**: Split-screen per viewport, per-camera

### SceneObject Class (0x4D934C)

Common renderable game object (0xD4 bytes):
- 3 matrix transforms: base scale (+0x94), rotation (+0xA8), world (+0xBC)
- Visible flag (+0x88), zOrder/object ID (+0x8C)
- Bounding radius (+0xCC), type (+0xD0)
- Scene registration via Scene_RegisterObject stores at scene+0x710+id*4

### Level Setup Pattern

All level setups follow: Level_ctor → Level_Clone → Level_InitScene → vmethod +0x80, then level-specific object creation (flags, signs, bumpers, vacuum tubes, etc).

| Level | Address | Special Objects |
|-------|---------|----------------|
| leveldark | 0x416270 | 2-player support |
| level5 | 0x40E190 | (none) |
| level6 | 0x40EA90 | LAUNCH01/02/03, CHROMESHADOW |
| level7 | 0x40F360 | (none) |
| level9 | 0x40830 | PILLAR, MAGNIFYER, CLOUDSCAPE, fog |
| level10 | 0x411F60 | 4 bumpers, TarBubble |
| levelcascade | 0x4110D0 | 8 bumpers |
| levelup | 0x411540 | VAC-IN/VAC-OUT vacuum tubes |

### Scene Vtable (0x4D0260, 36 entries)

Key identified slots:
- [0] Scene_DeletingDtor - destructor + conditional free
- [1] Scene_Update - main 9-step tick
- [2] Scene_Render - 1P/2P/split render dispatch
- [3] Scene_HandleInput - menu item iteration, input check, current item at +0x864
- [4] Scene_ActivateCurrentItem - call vmethod+0x10 on current item
- [6] Scene_SelectCurrentItem - call vmethod+0x0C on current item
- [11/12] Scene_ClearCurrentItem - set current item pointer to NULL
- [17] Scene_SaveAndCleanup - delegates to FUN_469AC0
- [18] Level_InitScene
- [24-26] Level render methods
- [9,10,13,21,30,34] NoOp_return (0x44B840) - default stubs overridden by subclasses

### SceneObject Vtable (0x4D934C, 10 entries)

- [0] SceneObject_dtor (0x46B650)
- [1] SceneObject_SetPosition (0x46B490) - sets +0x08,x,y,z + calls vmethod+0x0C
- [2] SceneObject_SetScale (0x46B4B0) - sets +0x14 scale + calls vmethod+0x0C
- [3] SceneObject_Render (0x46B670) - build world matrix, D3D SetTransform+SetMaterial
- [4] SceneObject_SetVisible (0x46B4D0) - toggle +0x88
- [7] SceneObject_DeletingDtor (0x46B9F0)
- SceneObject_BaseDtor (0x46B860) iterates child list, calls dtor(1) on each child

### Rumble/Arena Board System

14 arena init functions (RumbleBoard_*_Init) follow this pattern:
1. Level_ctor(arena_level_path) → store at scene+0x22B
2. Level_Clone(source) → store at scene+0x22C
3. CameraLookAt(scene)
4. Virtual dispatch vmethod+0x80 (post-setup)

Special arena behaviors:
- Beginner (0x413CE0): 4 bumpers via N:BUMPER%d
- Toob (0x414F00): 5 bumpers via N:BUMPER%d
- Dizzy (0x414240): Extra Level3-Swirl loaded
- Sky (0x4158C0): PILLAR name scanning via __strnicmp
- Neon (0x416F40): Scale matrix setup for dynamic objects, SceneObject decoration at +0x11F9

### Tournament Board Constructors

Board subclasses (0x419030 base) hold sub-level arrays:
- Toob Board (0x41F4B0): "Rodenthood" tournament with 5 sub-levels:
  Level8-Spinny, Level8-Saw, Level8-Fallout, Level8-Blockdawg1, Level8-Blockdawg2

### Scene Object Lists (Scene offsets)

| Offset | Description |
|--------|-------------|
| +0x22E | Static scene objects (persistent) |
| +0x335 | Dynamic objects (moving platforms etc) |
| +0x43B | Sub-objects (linked to dynamic) |
| +0xA75 | Ball list (per-ball position updates) |
| +0xC81 | Additional object list |
| +0xD8B | Render update list |

### Camera System Detail

Scene_SetCamera (0x419FA0) positions camera:
1. Start from ball position +0x758
2. Add scene offset (+0x434C)
3. If boundary check enabled (+0x3F1C): compute distance from center, clamp with distance falloff
4. If countdown timer active (+0x3F2C): snap camera to ball for initial frames
5. Noise randomizer at ball+0x744 (jitters position)
6. Set view/projection matrices

### Difficulty System

Difficulty_GetTimeModifier (0x428ED0) returns multiplier based on +0x23C:
- Mode 0: Easy (time * _DAT_4CF3F0)
- Mode 1: Normal (time * 1.0)
- Mode 2: Hard (time * 2.0)
- Default: 0.0 (no time?)

---

## 10. Documentation Progress

- **Total functions:** 3,797
- **Documented:** 1,758 (46.3%)
- **Session progress:** 44.1% → 46.3% (+41 functions named this session)
- **Key docs updated:** FUNCTION_MAP.md, KEY_FINDINGS.md