# Camera System

## Overview
Hamsterball uses a 5-mode camera system controlled by `Scene_SetCamera` (0x419FA0).
The camera follows the ball with spring-damped interpolation, and supports path-based
rails, shake effects, snap transitions, and orbital rotation.

## Camera Modes

### 1. Default Follow (no flags)
Camera target = ball position + offset (Scene+0x434C, Scene+0x4350, Scene+0x4354).
Camera smoothly follows ball with spring damping (hardcoded 0.9 factor).

### 2. Path Rail Mode (Scene+0x3F1C != 0)
When a path is active, the camera rides along a spline path:
1. `Path_GetPosition(path, &pos, path_t)` gets current path position
2. Direction from path to ball is computed
3. If distance < threshold → camera offset = 0 (ball on rail)
4. If distance > threshold → offset scales linearly with sine wobble
5. Max camera offset capped at 700.0 units
6. `Ball_SetTargetPos` sets ball's camera target position

Path rail adds a spring-like oscillation using `Wave_Sin` modulation,
creating a rubber-band effect when the ball deviates from the path.

### 3. Camera Shake (Ball+0x744 != 0)
When shake flag is set, random offsets of ±50 units are added to camera position:
```
shake_x = RNG_Rand(-50, 50)
shake_y = RNG_Rand(-50, 50)  
shake_z = RNG_Rand(-50, 50)
camera_pos += (shake_x, shake_y, shake_z)
```

### 4. Camera Snap (Scene+0x3F2C != 0, countdown frames)
When snap countdown > 0:
- Set ball target pos directly to ball's forward vector (Ball+0x60..0x68)
- Set ball actual cam pos to ball's forward vector
- Decrement countdown each frame
Used for race start countdowns and scene transitions.

### 5. Orbital Rotation (always active)
Every frame, regardless of mode:
```
cos_angle = Wave_Cos(Scene+0x29BC)  // orbit angle
sin_angle = Wave_Sin(Scene+0x29BC)  // orbit angle
orbit_dir = (cos_angle, 0.9, sin_angle)  // slightly above horizontal
Camera_SetView(orbit_dir, Scene+0x29C0)  // distance
Graphics_Refresh()  // commit camera
```

## Scene_SetCamera (0x419FA0)

```c
void Scene_SetCamera(Scene* this, Ball* ball, char use_path) {
    // Start with ball's target cam position
    Vec3 cam_target = ball->cam_target;  // +0x758
    Vec3 scene_offset = this->cam_offset; // +0x434C
    Vec3 desired = cam_target + scene_offset;
    
    // Mode 2: Path rail (if path active and use_path enabled)
    if (this->path_active && use_path) {
        Vec3 path_pos = Path_GetPosition(this->path, this->path_t);
        Vec3 delta = desired - path_pos;
        float dist = Vec3_Length(delta);
        
        if (dist < threshold) {
            // On rail - no offset
            offset = 0.0f;
        } else {
            if (dist > 700.0f) dist = 700.0f;
            dist -= MIN_DIST;  // _DAT_004cf3ec
            float wave = Wave_Sin(dist * WAVE_SCALE);  // oscillation
            offset = dist - dist * DAMPING * wave;  // spring
        }
        Vec3 offset_vec = Vec3_NormalizeAndScale(delta, offset);
        desired = path_pos + offset_vec;
        Ball_SetTargetPos(ball, desired.x, desired.y, desired.z);
    }
    
    // Mode 3: Camera shake
    if (ball->shake_flag) {  // +0x744
        desired.x += RNG_Rand(-50, 50);
        desired.y += RNG_Rand(-50, 50);
        desired.z += RNG_Rand(-50, 50);
    }
    
    // Mode 4: Camera snap (countdown)
    if (this->snap_countdown > 0) {  // +0x3F2C
        this->snap_countdown--;
        desired = ball->forward;  // +0x60..0x68
        ball->cam_actual = desired;  // +0x76C..0x774
        ball->cam_target = desired;  // +0x758..0x760
    }
    
    // Mode 5: Orbital rotation (always)
    float cos_a = Wave_Cos(this->orbit_angle);  // +0x29BC
    float sin_a = Wave_Sin(this->orbit_angle);
    Vec3 orbit_dir = {cos_a, 0.9f, sin_a};
    Camera_SetView(orbit_dir, this->orbit_distance);  // +0x29C0
    Graphics_Refresh();
}
```

## CameraLookAt (Arena Camera) (0x413280)

Arena initialization sets up a fixed overhead camera:
```
1. Load "levels\arena-spawnplatform" mesh (MeshWorld_ctor, 0x10D0 bytes)
2. Load "levels\arena-stands" mesh (MeshWorld_ctor, 0x10D0 bytes)
3. Level_InitScene (build scene graph from loaded meshes)
4. Find "CAMERALOOKAT" object in hash table
5. Store target position: Scene+0x43BC..0x43C4 = CAMERALOOKAT pos
6. Store initial position: Scene+0x43AC..0x43B4 = CAMERALOOKAT pos
7. Set camera distance: Scene+0x29BC = 45.0f  (0x42340000)
8. Set camera height:  Scene+0x29C0 = 800.0f  (0x44480000)
9. Set max height:     Scene+0x43C8  = 800.0f  (0x44480000)
10. Set camera mode:   Scene+0x43CC = 1 (orbits)
11. Call vtable[0x54]() to apply camera settings
```

## Level_SelectCameraProfile (0x40ACA0)

Selects camera profile based on level type:
- Different profiles for: Warm Up, Beginner, Intermediate, etc.
- Adjusts orbit distance, height, damping factor

## Key Offsets

| Offset | Type | Description |
|--------|------|-------------|
| Ball+0x60..0x68 | Vec3 | Ball forward vector |
| Ball+0x164..0x16C | Vec3 | Ball position |
| Ball+0x758..0x760 | Vec3 | Camera target position |
| Ball+0x76C..0x774 | Vec3 | Camera actual position |
| Ball+0x744 | int | Camera shake flag |
| Scene+0x29BC | float | Camera orbit angle |
| Scene+0x29C0 | float | Camera orbit distance |
| Scene+0x3F1C | int | Path rail active flag |
| Scene+0x3F20 | Path* | Path object pointer |
| Scene+0x3F24 | float | Path parameter (t) |
| Scene+0x3F2C | int | Camera snap countdown |
| Scene+0x434C..0x4354 | Vec3 | Camera offset from ball |
| Scene+0x87C | Camera* | Camera object pointer |