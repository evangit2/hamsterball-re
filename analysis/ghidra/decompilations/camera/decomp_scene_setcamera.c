/*
 * Scene_SetCamera decompilation (0x419FA0)
 * 5-mode camera: Default follow, Path rail, Shake, Snap, Orbit
 * Ball vtable offsets: +0x758 cam_target, +0x76C cam_actual, +0x744 shake_flag
 * Scene offsets: +0x434C cam_offset, +0x3F1C path_active, +0x3F20 path_ptr
 *   +0x3F24 path_t, +0x3F2C snap_countdown, +0x29BC orbit_angle, +0x29C0 orbit_dist
 */

void Scene_SetCamera(void *this, void *ball, char use_path) {
    Vec3 desired;  // local_24
    Vec3 cam_target;  // loaded from ball+0x758
    Vec3 scene_offset;  // loaded from this+0x434C
    
    // Load ball's camera target position
    cam_target = *(Vec3*)(ball + 0x758);
    scene_offset = *(Vec3*)(this + 0x434C);
    desired.x = cam_target.x + scene_offset.x;
    desired.y = cam_target.y + scene_offset.y;
    desired.z = cam_target.z + scene_offset.z;
    
    // Mode 2: Path rail (spring-damped follow)
    if (*(int*)(this + 0x3F1C) != 0 && use_path != 0) {
        Vec3 path_pos;
        Path_GetPosition(*(void**)(this + 0x3F20), &path_pos, *(float*)(this + 0x3F24));
        
        float dx = desired.x - path_pos.x;
        float dy = desired.y - path_pos.y;
        float dz = desired.z - path_pos.z;
        float dist_sq = dx*dx + dy*dy + dz*dz;
        
        float offset = 0.0f;
        if (dist_sq <= THRESHOLD_SQ) {  // _DAT_004cf368 = 0.0f
            float dist = sqrtf(dist_sq);
            if (dist > MIN_DIST) {  // _DAT_004cf3ec
                if (dist > MAX_DIST) dist = 700.0f;  // _DAT_004cff78
                dist -= MIN_DIST;
                float wave = Wave_Sin(dist * WAVE_SCALE);  // _DAT_004d03a4 * _DAT_004d03a0
                offset = dist - dist * DAMPING * wave;  // _DAT_004cf3f0 = 0.95
            }
            
            // Normalize and scale delta vector
            float scale = offset / dist;
            desired.x = path_pos.x + dx * scale;
            desired.y = path_pos.y + dy * scale;
            desired.z = path_pos.z + dz * scale;
            
            // Set ball camera target position
            Ball_SetTargetPos(ball, desired.x, desired.y, desired.z);
        }
        
        // Load ball's actual camera position
        Vec3 cam_actual = *(Vec3*)(ball + 0x76C);
    }
    
    // Mode 3: Camera shake
    if (*(int*)(ball + 0x744) != 0) {
        int shake_x = RNG_Rand(-50, 50);
        int shake_y = RNG_Rand(-50, 50);
        int shake_z = RNG_Rand(-50, 50);
        desired.x += (float)shake_x;
        desired.y += (float)shake_y;
        desired.z += (float)shake_z;
    }
    
    // Mode 4: Camera snap (countdown frames)
    if (*(int*)(this + 0x3F2C) != 0) {
        *(int*)(this + 0x3F2C) -= 1;
        // Set camera to ball's forward vector
        desired = *(Vec3*)(ball + 0x60);
        *(Vec3*)(ball + 0x76C) = desired;  // cam_actual
        *(Vec3*)(ball + 0x758) = desired;  // cam_target
    }
    
    // Mode 5: Orbital rotation (every frame)
    float cos_a = Wave_Cos(*(float*)(this + 0x29BC));
    float sin_a = Wave_Sin(*(float*)(this + 0x29BC));
    Vec3 orbit_dir = { cos_a, 0.9f, sin_a };
    Camera_SetView(orbit_dir, *(float*)(this + 0x29C0));
    Graphics_Refresh();
}
