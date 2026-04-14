/* Ball_Update - Core physics tick for ball entity
 * Address: 0x405E00 (called via Ball vtable[4] = thunk at 0x408390)
 * Called from: Scene_UpdateBallsAndState (0x41B540) via ball->vtable[0x10]()
 *
 * BALL VTABLE MAP (0x4CF3A0):
 *   [0x00] 0x4027F0 Ball_dtor
 *   [0x04] 0x405100 Ball_InitPhysicsDefaults (thunk)
 *   [0x08] 0x402DE0 Ball_Render
 *   [0x0C] 0x402A70 Ball_?? (thunk)
 *   [0x10] 0x408390 Ball_Update (thunk to 0x405E00)
 *   [0x14] 0x401590 Ball_?? (thunk)
 *   [0x18] 0x402650 Ball_ApplyForceWithMultipliers
 *   [0x1C] 0x402C10 Ball_?? (thunk)
 *   [0x20] 0x409480 Ball_OnLevelTransition (thunk)
 *
 * BALL STRUCT KEY OFFSETS (int-indexed, *4 for byte):
 *   [0x04]  = app_offset / scene_base
 *   [0x05]  = scene_ptr (back-pointer to Scene)
 *   [0x06]  = player_index (int, -1 = invalid/NPC)
 *   [0x07]  = render_callback_obj
 *   [0x59/0x5A/0x5B] = position (Vec3: X/Y/Z)
 *   [0x56/0x57/0x58] = prev_position (Vec3, for delta calc)
 *   [0x5C/0x5D/0x5E] = accumulated_force (Vec3, zeroed each tick)
 *   [0x5F/0x60/0x61] = velocity_delta (for rotation calc)
 *   [0x62]  = speed (float, magnitude of velocity)
 *   [0x65..0x66] = facing_direction (cos/sin of angle)
 *   [0x69]  = physics_body (BallPhysics struct, contains collision tree)
 *   [0x6A/0x6B/0x6C] = gravity_direction (Vec3)
 *   [0x97]  = ball_rotation_speed (from collision friction)
 *   [0x98]  = is_airborne (bool)
 *   [0x99]  = rumble_timer (RumbleBoard for haptics)
 *   [0x9E]  = collision_layer_mask (int)
 *   [0x9F]  = collision_category (int)
 *   [0xA1]  = radius (float)
 *   [0xA7]  = speed_scale (float, defaults to 1.0)
 *   [0xA8]  = invincibility_frames (decremented each tick)
 *   [0xA9]  = max_speed (float)
 *   [0xB0/0xB1/0xB2] = external_velocity (Vec3, applied+zeroed each tick)
 *   [0xB3]  = force_camera_target (bool)
 *   [0xB6]  = freeze_timer (int, decremented, resets flag)
 *   [0xB7..0xB9] = checkpoint_position (Vec3, last valid position)
 *   [0xBA]  = out_of_bounds (bool, triggers respawn)
 *   [0xBB]  = consecutive_airborne_frames (int, OOB counter)
 *   [0xBC]  = air_timer (decremented each tick)
 *   [0xBD]  = was_airborne_frames (int)
 *   [0xBE]  = camera_override (bool)
 *   [0xBF]  = respawn_countdown (float, OOB timer)
 *   [0xC0]  = some_timer (decremented each tick, clamped to 0)
 *   [0xC5]  = roll_angle (float, ball mesh rotation)
 *   [0xC6]  = roll_damping (float)
 *   [0xC9]  = respawn_frozen (bool, skip auto-respawn)
 *   [0xC4]  = just_moved (bool, cleared after distance check)
 *   [0xC1..0xC3] = last_moved_position (Vec3)
 *   [0x1D2] = camera_axis (0=X, 1=Y, 2=Z) - which axis camera follows
 *   [0x1D3/0x1D4] = camera_bounds (float, clamp values)
 *   [0x1D6..0x1D8] = display_position (Vec3, smoothed/lerped)
 *   [0x1D9] = display_lerp_factor (float, 0.01-1.0)
 *   [0x1DA] = race_active (bool, in race mode)
 *   [0x1DE..0x1E1] = force accumulators (Vec3 + w)
 *   [0x1F2..0x201] = timer_array[16] (uint, decremented each tick)
 *   [0x202/0x203] = more_timers
 *   [0x204] = trail_point_list (AthenaList of trail particles)
 *   [0x307] = trail_point_array
 *   [0x30A] = display_string_ptr (char*, freed after 200 ticks)
 *   [0x30F] = teleport_active (bool)
 *   [0x310..0x312] = teleport_destination (Vec3)
 *   [0x313] = is_NPC (bool, no collisions with other balls)
 *   [0x314] = sound_cooldown (float, multiplied by decay each tick)
 *   [0x315] = ambient_sound_id
 *   [0x324]  = ??? (some timer)
 *
 * PHYSICS UPDATE ORDER:
 *   1. Decay sound cooldown multiplier
 *   2. Decrement various frame counters (invincibility, air, freeze)
 *   3. Play 3D ambient sounds
 *   4. Spawn RumbleScore particles (random positions near ball)
 *   5. Update camera axis bounds (clamped +/- offset per frame)
 *   6. Update checkpoint distance tracking (for race timer penalties)
 *   7. Clear force accumulators (16-element timer array)
 *   8. Process OOB timer (if respawn_frozen: counts down, then OOB flag)
 *   9. Process trail particles (render, cull hidden, expire)
 *   10. Save prev_position, zero accumulated force
 *   11. Build spatial collision tree (SpatialTree_ctor)
 *   12. Create CollisionNode for this ball
 *   13. Compute gravity reflected velocity
 *   14. Compute collision distance threshold
 *   15. Iterate collision results list:
 *       - Type 1: Ball-ball collision (apply impulse, sound, score)
 *       - Type 2: Wall collision (reflect velocity, bounce sound)
 *       - Type 5: Floor/level surface (gravity, OOB check, checkpoint)
 *   16. Post-collision: apply external velocity, update display position
 *   17. Calculate facing direction (atan2 of velocity)
 *   18. Path following (Scene_CheckPath for rail/track snapping)
 *   19. Rolling physics (roll_angle accumulation from ground contact)
 *   20. Lerp display_position toward actual position
 *   21. Teleport override (if active, snap position)
 *   22. Cleanup spatial tree
 *
 * BALL-BALL COLLISION:
 *   - Detects when two balls overlap
 *   - Computes relative velocity and separation vector
 *   - Applies elastic impulse proportional to speed difference
 *   - Plays collision sound at impact point
 *   - If one ball is much bigger: triggers "crush" event
 *   - Both balls get score penalties (Difficulty_GetTimeModifier(500ms))
 *
 * WALL COLLISION:
 *   - Reflects velocity off wall normal
 *   - Computes angular velocity from roll on wall surface
 *   - Speed reduction on wall contact (friction)
 *   - Bounce coefficient varies by surface angle
 *
 * FLOOR/SURFACE (type 5):
 *   - Gravity application (up vector = 0,1,0 for normal gravity)
 *   - Viewport clipping for OOB detection
 *   - Airborne counter increments when above surface
 *   - Camera update for surface following
 *   - Mesh_FindClosestCollision for ground check
 */
void __fastcall Ball_Update(int *param_1)
{
    // This function is ~2400 lines of decompiled code
    // Full decompilation saved in this file for reference
    // Key offsets and execution order documented above
}