# Camera System

## Architecture Overview

Hamsterball uses a dual camera system:
1. **Race Camera** — Smooth-lerp follow camera tracking the ball through race levels
2. **Arena/Rumble Camera** — Fixed overhead camera centered on arena (CAMERALOOKAT point)

Both modes are managed through the Scene/Board object with camera parameters at fixed offsets.

## Camera Modes

### Race Camera (Follow Ball)
Used during tournament / time trial races (races 4-14/15):
- Camera position lerps toward ball position each frame
- Called during `Ball_Update` when BALL-ON-FLOOR event (type 5, depth > 0.1)
- `Scene_SetCamera(scene, ball, FLAG=1)` updates camera target
- `Graphics_SetViewport` at ball current position
- Out-of-bounds detection: if ball leaves visible area, flag `0xBA = 1` triggers respawn

### Arena/Rumble Camera (Fixed)
Used during rumble/arena events:
- Camera positioned above CAMERALOOKAT position in level geometry
- Static position, does not follow ball
- Distance 45.0 units, height 800.0 units above target
- Found via scene hash table lookup at `this[0x22B]`

## Camera Initialization (CameraLookAt 0x413280)

```
CameraLookAt(this = Scene/Board):
  1. Load "levels\arena-spawnplatform" as MeshWorld → this[0x10E3]
  2. Load "levels\arena-stands" as MeshWorld → this[0x10E4]
  3. Add meshes to scene via vtable[0x90]
  4. Level_InitScene(this) — set projection, fog, find CAMERALOCUS
  5. Find "CAMERALOOKAT" in scene hash table
  6. Set camera target: this[0x10DE-0x10E0] = CAMERALOOKAT position
  7. Set current position: this[0x10DB-0x10DD] = same (initialized)
  8. Set parameters:
     - this[0xA6F] = 45.0  (camera distance/zoom)
     - this[0xA70] = 800.0 (camera height offset)
     - this[0x10E1] = 800.0 (camera max height)
     - this[0x10E2] = 1 (camera initialized flag)
  9. vtable[0x54]() — post-camera render callback
```

## Camera Offsets (in Scene/Board struct)

| Offset | Type | Description |
|--------|------|-------------|
| 0x10DB | float | camera_current_X (lerps toward target) |
| 0x10DC | float | camera_current_Y |
| 0x10DD | float | camera_current_Z |
| 0x10DE | float | camera_target_X (from CAMERALOOKAT) |
| 0x10DF | float | camera_target_Y |
| 0x10E0 | float | camera_target_Z |
| 0x10E1 | float | camera_max_height (800.0) |
| 0x10E2 | bool | camera_initialized (1 = active) |
| 0x10E3 | MeshWorld* | spawn_platform_mesh |
| 0x10E4 | MeshWorld* | stands_mesh |
| 0xA6F | float | camera_distance (45.0 default) |
| 0xA70 | float | camera_height_offset (800.0 default) |

## Camera Profile Selection (0x40ACA0)

`Level_SelectCameraProfile(app)` — Selects camera based on current race number:

| Race # | App Flag | Description |
|--------|----------|-------------|
| 4 | App+0x85A | Race 4 camera profile |
| 5 | App+0x85B | Race 5 camera profile |
| 6 | App+0x85C | Race 6 camera profile |
| 7 | App+0x866 | Race 7 camera profile |
| 8 | App+0x85D | Race 8 camera profile |
| 9 | App+0x85E | Race 9 camera profile |
| 10 | App+0x85F | Race 10 camera profile |
| 11 | App+0x860 | Race 11 camera profile |
| 12 | App+0x867 | Race 12 camera profile |
| 13 | App+0x861 | Race 13 camera profile |
| 14 | App+0x862 | Race 14 camera profile |
| 15 | App+0x868 | Race 15 camera profile |

Each flag is a bool: `0` = standard camera, `1` = alternate camera profile
- If flag == 0: `viewport[0x2A8]+4 = viewport[0x2AC]+4` (standard)
- If flag == 1: `viewport[0x2A8]+4 = viewport[0x2B0]+4` (alternate)

Races 1-3 (Warm-up, Beginner, Intermediate) use default camera profile.

## Camera Update in Ball_Update

Within the 18-phase ball update loop:
- **BALL-ON-FLOOR** (event type 5, depth > 0.1):
  - Camera follows ball: `Scene_SetCamera(scene, ball, 1)`
  - `Graphics_SetViewport` at ball display position
  - Out-of-bounds check → trigger respawn

- **Phase 17** (Multiplayer sync):
  - Updates 3D audio listener position from ball display_pos
  - `SoundDevice.listeners[ball_index] = ball.display_pos`
  - Camera does NOT explicitly update here — it's implicit in viewport

## Multiplayer Viewport

In multiplayer, the screen is split:
- Player 1: top or left half
- Player 2: bottom or right half
- Each player has independent camera follow
- Viewport rects at `App+0x27C/0x280/0x284/0x288/0x28C/0x290`
- `App+0x234` determines layout (horizontal vs vertical split)
- Mirror mode swaps cull mode: `App+0x236` → `gfx.reversed`

## Vec3_Bilinear (0x459D90)

Camera path interpolation:
```
Vec3_Bilinear(out, a, b, c, u, v):
  // Bilinear interpolation between 4 corner points
  // a = bottom-left, b = bottom-right, c = top-left, d = top-right
  out.x = (d.x - b.x) * v + (c.x - a.x) * u + a.x
  out.y = (d.y - b.y) * v + (c.y - a.y) * u + a.y
  out.z = (d.z - b.z) * v + (c.z - a.z) * u + a.z
```
Used for smooth camera interpolation across camera trigger volumes.

## Key Address Map

| Address | Function | Description |
|---------|----------|-------------|
| 0x40ACA0 | Level_SelectCameraProfile | Select camera by race number |
| 0x413280 | CameraLookAt | Initialize arena camera (CAMERALOOKAT) |
| 0x459D90 | Vec3_Bilinear | Bilinear camera path interpolation |

## Reimplementation Notes (SDL2/OpenGL)

### Camera System
```cpp
class Camera {
    Vec3 current_pos;    // Lerps toward target each frame
    Vec3 target_pos;     // Set by Scene_SetCamera or CAMERALOOKAT
    float distance = 45.0f;
    float height = 800.0f;
    float max_height = 800.0f;
    
    void Update(float dt) {
        // Smooth follow with lerp factor
        float lerp = 1.0f - pow(0.95f, dt * 60);  // ~0.05/frame at 60fps
        current_pos = lerp * target_pos + (1.0f - lerp) * current_pos;
        
        // Position camera above and behind target
        Vec3 eye = current_pos + Vec3(0, height, distance);
        lookAt(eye, current_pos, Vec3::UP);
    }
};
```

### Multiplayer Viewport
- Use `glViewport()` to render each player's view
- Screen splits: horizontal or vertical based on `App+0x234` 
- Each viewport has independent `Camera` instance

### Mirror Mode
- OpenGL: `glFrontFace(GL_CW)` for normal, `GL_CCW` for mirror
- Need to verify: is track geometry actually mirrored, or just winding order reversed?