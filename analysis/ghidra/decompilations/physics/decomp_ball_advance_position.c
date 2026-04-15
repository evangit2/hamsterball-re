/*
 * Ball_AdvancePositionOrCollision (0x4564C0)
 * Core physics: advances ball position, handles collision response, applies gravity/damping
 * 
 * Ball offsets used:
 *   +0x14    char: trail recording flag (if nonzero, record trail)
 *   +0x18    AthenaList: trail points (Material structs, 0x68 each)
 *   +0x1C    int: trail list size
 *   +0x424   AthenaList*: trail data array
 *   +0x848   AthenaList: collision markers (freed each frame)
 *   +0x84c   int: collision list size
 *   +0x850   int: collision iteration index
 *   +0xC54   AthenaList*: collision data array
 *   +0xC64   float: speed scale factor
 *   +0xC68   float: damping factor (0.0-1.0)
 *   +0xC70   float: max velocity clamp
 *   +0xC74   float: accumulated trail distance
 *   +0xC7C   char: collision occurred flag (0=no collision, 1=collided)
 *   +0xC88   float: gravity X component
 *   +0xC8C   float: gravity Y component
 *   +0xC90   float: gravity Z component
 *   +0xC94   float: gravity scale
 *   +0xCA4   float: velocity X
 *   +0xCA8   float: velocity Y
 *   +0xCAC   float: velocity Z
 *   +0xC604  vtable: collision handler (vtable[0x1C])
 *
 * Constants:
 *   _DAT_004CF310 = 1.0f
 *   _DAT_004CF368 = 0.0f
 *   _DAT_004CF3F0 = 0.95f (damping)
 */

void Ball_AdvancePositionOrCollision(void *this, float *out_pos, float *cur_pos, 
                                      float *input_vel, int param_4, float dt) {
    // ---- Phase 1: Free trail point lists ----
    if (this->trail_flag) {  // +0x14
        // Free old trail points (AthenaList at +0x18)
        int idx = AthenaList_NextIndex(this + 0x18);
        this->trail_iter[idx + 0x20] = 0;
        void *item = (this->trail_count < 1) ? NULL : *this->trail_data[0];
        while (item != NULL) {
            free(item);
            idx = this->trail_iter[idx];
            if (this->trail_count <= idx) break;
            item = this->trail_data[idx];
            this->trail_iter[idx]++;
        }
        AthenaList_Free(this + 0x18);
    }
    
    // Free collision marker list (AthenaList at +0x848)
    int cidx = AthenaList_NextIndex(this + 0x848);
    this->collision_iter[cidx] = 0;
    void *citem = (this->collision_count < 1) ? NULL : *this->collision_data[0];
    while (citem != NULL) {
        free(citem);
        cidx = this->collision_iter[cidx];
        if (this->collision_count <= cidx) break;
        citem = this->collision_data[cidx];
        this->collision_iter[cidx]++;
    }
    AthenaList_Free(this + 0x848);
    
    // ---- Phase 2: Apply input velocity to current velocity ----
    float *vel = &this->vx;  // +0xCA4..0xCAC
    
    if (input_vel[0] != 0.0f || input_vel[1] != 0.0f || input_vel[2] != 0.0f) {
        // Compute current velocity magnitude
        float speed_sq = vel[0]*vel[0] + vel[1]*vel[1] + vel[2]*vel[2];
        float old_speed = (speed_sq > 0.0f) ? sqrtf(speed_sq) : 0.0f;
        
        // Add input to velocity
        vel[0] += input_vel[0];
        vel[1] += input_vel[1];
        vel[2] += input_vel[2];
        
        // Compute new velocity magnitude
        float new_speed_sq = vel[0]*vel[0] + vel[1]*vel[1] + vel[2]*vel[2];
        float new_speed = (new_speed_sq > 0.0f) ? sqrtf(new_speed_sq) : 0.0f;
        
        // Clamp to max velocity if exceeded
        if (this->max_velocity < new_speed) {  // +0xC70
            if (old_speed < this->max_velocity) {
                old_speed = this->max_velocity;
            }
            Vec3_NormalizeAndScale(vel, old_speed);
        }
    }
    
    // ---- Phase 3: Apply damping ----
    char collided = 0;
    float damping = (1.0f - dt) + (1.0f - this->damping_factor) * dt;  // +0xC68
    // Result: damping = (1 - dt) + (1 - damping) * dt
    // At dt=1: damping = 0 + (1 - damping) = 1 - damping
    // At dt=0: damping = 1 + 0 = 1 (no change)
    
    vel[0] *= damping;
    vel[1] *= damping;
    vel[2] *= damping;
    
    // ---- Phase 4: Move position or handle collision ----
    float new_x, new_y, new_z;
    
    if (this->collision_flag == 0) {  // +0xC7C
        // No collision: simple position update
        new_x = vel[0] + cur_pos[0];
        new_y = vel[1] + cur_pos[1];
        new_z = vel[2] + cur_pos[2];
    } else {
        // Collision: call vtable[0x1C] collision handler
        float *result = this->vtable[0x1C](this, cur_pos, vel, param_4, dt, &collided);
        new_x = result[0];
        new_y = result[1];
        new_z = result[2];
    }
    
    // ---- Phase 5: Apply gravity ----
    float grav_dt = dt * this->speed_scale;  // +0xC64
    float grav_scale_damp = dt * 0.95f;     // _DAT_004CF3F0 = damping constant
    
    float grav_y_extra = grav_dt * this->grav_y;    // +0xC94 (Y gravity scale)
    float grav_x_out = grav_scale_damp * this->grav_x_scale;  // +0xC90
    float grav_z_out = grav_scale_damp * this->grav_z_scale;   // +0xC94
    
    // Combined gravity: dt*grav_y + damp*(grav_x + dt*grav_y_base)
    float grav_x = grav_scale_damp * this->grav_y_base + grav_dt * this->grav_y_base;
    // +0xC8C is gravity.y base
    
    vel[0] += (grav_scale_damp * this->grav_y_base + grav_dt * this->grav_y_base);
    vel[1] += grav_dt * this->grav_y_scale + grav_x_out;
    vel[2] += grav_z_out + grav_y_extra;
    
    // ---- Phase 6: Record trail if no collision ----
    if (collided == 0) {
        this->trail_distance += dt;  // +0xC74
        
        if (this->trail_flag) {  // +0x14
            // Create trail point (Material struct, 0x68 bytes)
            Material *trail_pt = operator_new(0x68);
            Material_Init(trail_pt);
            trail_pt->type = 5;  // Trail point type
            trail_pt->pos = this->velocity;  // Copy velocity
            trail_pt->distance = this->trail_distance;
            AthenaList_Append(this + 0x18, trail_pt);
        }
    } else {
        this->trail_distance = 0;  // Reset on collision
    }
    
    // ---- Phase 7: Write output ----
    out_pos[0] = new_x;
    out_pos[1] = new_y;
    out_pos[2] = new_z;
}