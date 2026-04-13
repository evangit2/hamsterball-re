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

*Last updated: April 12, 2026*
*Project: Hamsterball RE (Raptisoft, 2000s)*
*Permission: Original developer granted permission for RE*