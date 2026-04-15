/*
 * Scene_SpawnBallsAndObjects (0x41C5B0)
 * Level startup - creates balls and objects from MESHWORLD data
 */

void Scene_SpawnBallsAndObjects(Scene *scene) {
    // Store player count in App
    int player_count = AthenaList_GetSize(&scene->player_list);
    *(int*)(scene->app + 0x850) = player_count;
    
    // For each player slot, create a ball
    for each player in player_list {
        // Find start position by looking up "START%d-%d" in hash table
        // Format: "START{player_index}-{player_index}"
        Vec3 start_pos = HashTable_Lookup(scene->hashtable, 
                                          Format("START%d-%d", player_idx, player_idx));
        
        // For 2P mouse mode: also check alternate start position
        if (scene->app->is_2player) {
            Vec3 alt_pos = HashTable_Lookup(scene->hashtable,
                                             Format("START%d-%d"));
            // Use alternate if found
        }
        
        // For tournament/difficulty modes with single player:
        // Random start position selection between START2-1 and START2-2
        if (player_count == 1 && (mode == 5 || mode == 11 || mode == 12 || mode == 14)) {
            int rng = RNG_Rand(0, 2);  // 0 or 1
            if (rng == 0) start_pos = HashTable_Lookup("START2-1");
            else start_pos = HashTable_Lookup("START2-2");
        }
        
        // DEBUG start position (cheat code)
        if (player_count == 1) {
            Vec3 debug_pos = HashTable_Lookup("START-DEBUG");
            if (debug_pos != 0.0f) start_pos = debug_pos;
        }
        
        // Create ball (0xC60 bytes)
        Ball *ball = Ball_ctor2(new(0xC60), scene);
        ball->vtable[1]();  // Post-constructor init
        
        // Set trajectory from player data
        Ball_SetTrajectory(ball, 
            scene->trajectory_data[player_data->traj_idx * 5 + 1],
            scene->trajectory_data[player_data->traj_idx * 5 + 2],
            scene->trajectory_data[player_data->traj_idx * 5 + 3],
            scene->trajectory_data[player_data->traj_idx * 5 + 4]);
        
        // Set ball properties
        ball->pos_y = start_pos.y + OFFSET;   // _DAT_004d0434 = vertical offset
        ball->pos_z = start_pos.z;
        ball->player_index = player_data->index;
        ball->gravity_scale = 0.5;
        ball->field_0x27C = 0.1f;  // _DAT (0x3DCCCCCD)
        ball->radius = 26.0f;       // default ball radius
        ball->field_0x1A0 = 1.05f;  // (0x3F866666)
        ball->max_speed = 5.0f;     // (0x40A00000)
        ball->pos_x = start_pos.x;
        ball->is_falling = 0;
        ball->field_0x769 = 1;
        
        AthenaList_Append(&scene->ball_list, ball);
        player_data->ball = ball;
    }
    
    // Second pass: scan for SAFESPOT/SAFEPOS objects
    for each object in hashtable {
        if (strnicmp(name, "SAFESPOT", 8) == 0 || strnicmp(name, "SAFEPOS", 7) == 0) {
            AthenaList_Append(&scene->safespots, object);
        }
    }
    
    // Tournament/demo mode: add hazards
    if (scene->app->is_tournament || scene->app->is_demo) {
        CreateBadBall(scene);    // Enemy ball
        CreateMouseTrap(scene);   // Mouse trap obstacle
    }
    
    // Create level decoration objects
    CreateSecretObjects(scene);     // Hidden collectibles
    Scene_CreateFlags(scene);        // Finish line flags
    Scene_CreateSigns(scene);        // Direction signs
    Scene_CreateDynamicObjects(scene); // Moving platforms, etc.
}

/*
 * Key Ball spawn constants:
 *   radius = 26.0f (not 27f as used in trail particles)
 *   max_speed = 5.0f
 *   gravity_scale = 0.5
 *   default_speed_factor = 1.05f (0x3F866666)
 *   
 * Start position keys in MESHWORLD hash table:
 *   "START0-0" - Player 1 start (single player)
 *   "START1-1" - Player 2 start (2P mode)
 *   "START2-1" / "START2-2" - Tournament alternate starts
 *   "START-DEBUG" - Debug/cheat start position
 *   
 * SAFESPOT/SAFEPOS - Checkpoint/safe zone objects
 */