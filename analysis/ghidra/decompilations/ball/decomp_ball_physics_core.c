/* Ball Physics Core Functions - Deep Documentation
 * 
 * Ball_ApplyForceWithMultipliers (0x402650) - Force application with speed modifiers
 * Ball_ApplyTrajectory (0x403750) - Boost/launch with trail particles
 *
 * ═══════════════════════════════════════════════════════════════
 * Ball_ApplyForceWithMultipliers
 * ═══════════════════════════════════════════════════════════════
 *
 * Signature: Ball_ApplyForceWithMultipliers(this, dirX, dirY, dirZ, multiplier)
 *
 * GUARD CONDITIONS (all must be true for force to apply):
 *   +0x2F9 = !teleport_active (bool, 0=ok)
 *   +0x2CC = !some_block_flag (bool, 0=ok)
 *   +0x808 = block_timer == 0 (int)
 *   +0x2F0 = impact_counter < 81 (0x51)
 *
 * SPEED MODIFIERS (applied to multiplier param_4):
 *   if +0x2F0 != 0 (ball was recently hit):
 *     multiplier *= _DAT_004CF380 (reduced force after impact, ~0.1?)
 *   if +0x324 != 0 (some flag set):
 *     multiplier *= _DAT_004CF378 (another reduction)
 *   if +0xC5C != 0 (momentum transfer flag):
 *     multiplier *= _DAT_004CF374
 *     Set angular velocity at +0xFC/0x100/0x104 = param_123 * _DAT_004CF3E8
 *   if +0xC4C != 0:
 *     multiplier *= _DAT_004CF36C
 *
 * FORCE ACCUMULATION:
 *   +0x170 += dirX * multiplier
 *   +0x174 += dirY * multiplier
 *   +0x178 += dirZ * multiplier
 *   (These are the velocity components, accumulated per frame)
 *
 * AUTO-ROTATION:
 *   If direction is non-zero:
 *     Compute facing angle via Math_Atan2Angle based on camera_axis:
 *       axis 0: atan2(dirX, dirZ) — Y-axis rotation
 *       axis 1: atan2(-dirY, dirZ) — X-axis rotation
 *       axis 2: atan2(dirX, dirY) — Z-axis rotation
 *     Store at +0x198 (target rotation)
 *     Set +0x19C = 1 (rotation needs update)
 *
 * BALL STRUCT REFERENCES:
 *   +0x748 = camera_axis (0=X, 1=Y, 2=Z)
 *   +0x808 = force_block_timer
 *   +0xC4C = friction modifier flag
 *   +0xC5C = momentum transfer flag
 *   +0xFC/0x100/0x104 = angular velocity (Vec3)
 *   +0x170/0x174/0x178 = accumulated velocity (Vec3)
 *   +0x198 = target_rotation_angle
 *   +0x19C = rotation_dirty (bool)
 *   +0x2CC = some_block_flag
 *   +0x2F0 = impact_counter (0x51 = 81 = max)
 *   +0x2F9 = teleport_active
 *   +0x324 = speed_modifier_flag
 *   +0x3FC/0x400/0x404 = momentum_transfer_velocity (Vec3)
 *
 * ═══════════════════════════════════════════════════════════════
 * Ball_ApplyTrajectory
 * ═══════════════════════════════════════════════════════════════
 *
 * Called when a ball collides with a surface that triggers a launch/boost.
 * Uses the physics body's stored trajectory vector.
 *
 * ALGORITHM:
 *   1. Read trajectory from physics_body+0xCA4/CA8/CAC (Vec3)
 *   2. Set rotation dirty flag (+0x14D = 1)
 *   3. Normalize trajectory and scale by magnitude * _DAT_004CF3F0
 *      (This preserves the speed while zeroing the Y component for launch)
 *   4. Apply damping to Y component: body+0xCA8 *= _DAT_004CF434
 *      (Reduces vertical velocity for flatter launches)
 *   5. Set X/Z components to normalized trajectory
 *   6. Play boost sound: Sound_PlayChannel(scene+0x4DC)
 *   7. Set impact_counter = 100 (prevents ApplyForce for 100 frames)
 *   8. Create trail particles: Ball_CreateTrailParticles
 *   9. Increment player's boost counter: app+0x5F8 + player*0xA0
 *
 * PHYSICS BODY OFFSETS (accessed via ball+0x1A4):
 *   +0xCA4 = trajectory_X (launch direction X)
 *   +0xCA8 = trajectory_Y (launch direction Y, damped)
 *   +0xCAC = trajectory_Z (launch direction Z)
 *
 * BALL STRUCT REFERENCES:
 *   +0x10  = scene_ptr
 *   +0x18  = player_index (-1 = invalid)
 *   +0x14D = rotation_dirty (set to 1)
 *   +0x1A4 = physics_body pointer
 *   +0x2F0 = impact_counter (set to 100 = ~1.67s at 60fps)
 *
 * KEY INSIGHT: Ball_ApplyTrajectory is the "launch pad" function.
 * When a ball hits a boost ramp or launch surface, the stored trajectory
 * vector from the level data is applied to the ball's velocity, with
 * the Y component damped for a more horizontal launch angle.
 * The impact_counter=100 prevents the ball from being pushed by
 * other forces during the launch, ensuring a clean trajectory.
 */