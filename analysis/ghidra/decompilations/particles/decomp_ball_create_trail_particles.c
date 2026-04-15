/*
 * Ball_CreateTrailParticles (0x401DD0)
 * Creates 9 trail particles in a circular pattern around the ball
 * Uses camera right/up vectors to orient the ring
 * Each particle is a RumbleScore object (0x28 bytes)
 */

void Ball_CreateTrailParticles(int ball_ptr) {
    int scene = *(int*)(*(int*)(ball_ptr + 0x14) + 0x878);
    float radius = *(float*)(ball_ptr + 0x284);
    
    int particle_list = *(int*)(*(int*)(ball_ptr + 0x14) + 0x3B00);
    
    // 9 particles (loop: 0 to 0x168 in steps of 0x28)
    for (int angle = 0; angle < 0x168; angle += 0x28) {
        // Camera basis vectors from graphics subsystem
        float cam_right_x = *(float*)(scene + 0x744 + 0x5C);
        float cam_right_y = *(float*)(scene + 0x744 + 0x60);
        float cam_right_z = *(float*)(scene + 0x744 + 0x64);
        float cam_up_x = *(float*)(scene + 0x744 + 0x68);
        float cam_up_y = *(float*)(scene + 0x744 + 0x6C);
        float cam_up_z = *(float*)(scene + 0x744 + 0x70);
        
        // Sin/cos for circular pattern
        float sin_angle = Wave_Sin(0x4F7188, (float)angle);
        float cos_angle = Wave_Cos(0x4F7188, (float)angle);
        
        // Compute right/up offsets
        float offset_right_x = radius * cam_right_x * sin_angle;
        float offset_right_y = radius * cam_right_y * sin_angle;
        float offset_right_z = radius * cam_right_z * sin_angle;
        
        float offset_up_x = radius * cam_up_x * cos_angle;
        float offset_up_y = radius * cam_up_y * cos_angle;
        float offset_up_z = radius * cam_up_z * cos_angle;
        
        // Ball position
        float ball_x = *(float*)(ball_ptr + 0x164);
        float ball_y = *(float*)(ball_ptr + 0x168);
        float ball_z = *(float*)(ball_ptr + 0x16C);
        
        // Create RumbleScore particle
        void* particle = operator_new(0x28);
        RumbleScore_ctor(particle, *(int*)(*(int*)(ball_ptr + 0x14) + 0x878));
        
        // Particle position = ball_pos + offset_right - offset_up
        float pos_x = offset_right_x + ball_x - offset_up_x;
        float pos_y = offset_right_y + ball_y - offset_up_y;
        float pos_z = offset_right_z + ball_z - offset_up_z;
        *(float*)(particle + 8) = pos_x;
        *(float*)(particle + 0xC) = pos_y;
        *(float*)(particle + 0x10) = pos_z;
        
        // Particle velocity = offset_right - offset_up
        float vel_x = offset_right_x - offset_up_x;
        float vel_y = offset_right_y - offset_up_y;
        float vel_z = offset_right_z - offset_up_z;
        
        // Scale velocity by 1/(RNG + 20) 
        // _DAT_004cf310 = 1.0f, RNG range 0x14..0x14+0x14
        int rng = RNG_Rand(0x14, 0x14);
        float scale = 1.0f / (rng + 20);
        vel_x *= scale;
        vel_y *= scale;
        vel_z *= scale;
        *(float*)(particle + 0x14) = vel_x;
        *(float*)(particle + 0x18) = vel_y;
        *(float*)(particle + 0x1C) = vel_z;
        
        // Add to particle list
        AthenaList_Append(particle_list, (int)particle);
    }
}