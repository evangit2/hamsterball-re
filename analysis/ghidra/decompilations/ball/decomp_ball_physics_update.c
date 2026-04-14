// =============================================================================
// Ball_Update — Ball Physics Core (0x405E00 / 0x405190)
// =============================================================================
//
// MAIN GAME PHYSICS TICK — Called every frame via Ball vtable[1] = 0x405100
// This is the primary physics simulation loop for the hamster ball. It is a
// massive ~600-line function with complex state management across 23+ phases.
//
// Address: 0x405E00 (the "thiscall" wrapper at 0x405190 dispatches to it)
// Called by: Scene_Update via Ball vtable[1] (0x405100 thunk -> 0x405190 -> 0x405E00)
//
// =============================================================================
// PHASE BREAKDOWN
// =============================================================================
//
// PHASE 0: SETUP & EARLY EXIT
// -----------------------------------
// - Exception handler setup (SEH frame)
// - Timer decay: param_1[0x314] *= 0.999 (auto-forward decay, 0.999f at 0x4CF540)
//   If result < 0.02 (0x4CF538), clamp to 0
// - param_1[0x317]-- (freeze frame counter, min 1)
// - param_1[200]-- (freeze frame counter #2, min 1) [note: these are COUNTER decay]
// - param_1[0xbc]-- (??? timer, min 1)
// - param_1[0xbd]-- (??? timer, min 1)
// - If ambient sound ID (param_1[0x315]) != 0: Sound_Play3DAtPosition
//
// PHASE 1: TRAIL PARTICLES
// -----------------------------------
// - If param_1[200] != 0: create RumbleScore trail particle
//   - Alloc 0x28 bytes, RumbleScore_ctor
//   - Random XYZ offset from ball position (RNG_Rand range 0x19=25)
//   - Set velocity=(0,0,0), timer=0, append to trail list
//   - Particle type ID stored at param_1[0x3B00] (AthenaList)
//
// PHASE 2: GRAVITY PLANE ACCUMULATION
// -----------------------------------
// Gravity plane modes (param_1[0x1D2]):
//   0 = flat (XZ plane): param_1[0x1D3] += 0.1 (0x4CF310), clamp > 6.0 (0x4CF368)
//   1 = tilted: param_1[0x1D3] -= 0.1, clamp < 0.0 (0x4CF530)
//   2 = vertical (XY plane): param_1[0x1D4] += 0.1, clamp > 6.0
// - param_1[0x1D1]-- (general decay counter, min 1)
// - param_1[0xC0]-- (freeze counter, min 0)
// - param_1[0xA8]-- (??? counter, reset speed_scale to 1.0 when < 1)
// - param_1[3]-- (lifetime counter, free display string when hits 0)
//
// PHASE 3: EARLY OUT (NO COLLISION DATA)
// -------------------------------------
// If param_1[0x53] (collision_loaded flag) == 0:
//   Skip entire physics pipeline and jump to PHASE 20
//
// PHASE 4: SECRET TRACKING
// -----------------------------------
// - If level_index != -1 AND in_multiplayer flag (0x1DA):
//   - If collision tracking flag (0xC4) set: record current ball pos as last_secret_pos
//   - Compute distance from last_secret_pos to current pos
//   - If distance > 6.0 AND secret zone flag not set: award time bonus via
//     Difficulty_GetTimeModifier, mark secret triggered
//
// PHASE 5: TIMER RESETS
// -----------------------------------
// - param_1[0x1DE..0x1E1] = 0 (clear 4 state flags)
// - param_1[0x1F2..0x201] -= 1 each, min 0 (16 collision type timers)
// - param_1[0x202]-- (collision type timer, min 1)
// - param_1[0x203]-- (collision type timer, min 1)
// - If dizzy modifier flag (0xC9) set: param_1[0xBF] -= 0.8, clamp >= 0
// - param_1[0xB6]-- (??? timer, reset flag at 0x2D5 when expires)
//
// PHASE 6: MECHANICAL OBJECT UPDATE (TIMER-BASED)
// -----------------------------------
// - If mechanical_objects list (param_1[0x204]) non-empty:
//   - Iterate each object (Timer positions via RotateY)
//   - Compute height vs ball position
//   - If ball BELOW object: mark object timer active (decrement)
//   - If ball ABOVE object AND timer expired: trigger object, move to pending list
//   - Cleanup: free pending objects
//   - If any mechanical events fired AND RNG==0: check floor collision for sparks
//     - Mesh_FindClosestCollision at ball position - ball_radius
//     - If close enough: create BONKPOPUP particle effect
//
// PHASE 7: TARPIT STATE MACHINE
// -----------------------------------
// - If tar_state flag (0x2F9) set:
//   - param_1[0xBF] += 0.5, if > 1.0 then reset flag and clamp to 1.0
//
// PHASE 8: POSITION BACKUP (prev_pos = current)
// -----------------------------------
// - Save display position: param_1[0x58..0x5C] = ball XYZ (prev frame)
// - Clear accumulators: param_1[0x5C..0x5E] = 0 (velocity XYZ)
//
// PHASE 9: COLLISION TREE BUILD
// -----------------------------------
// - Init AthenaList with collision mesh (param_1[5] + 0x8B0)
// - Set collision_mesh[0xC70] = ball_radius
// - Get gravity vector from collision mesh (0xC9C..0xCA4): gravity XYZ
// - Compute gravity projection: -dot(velocity, gravity_normal)
//   if aligned with gravity: zero velocity components perpendicular to gravity
//   This is the "gravity reflection" — zeroing velocity INTO the gravity plane
// - Build SpatialTree (param_1[0x69] + 0xC78 = ball_radius)
// - Create CollisionNode (0x14 bytes) for this ball
//
// PHASE 10: GRADIENT EVALUATION (if not in freeze state)
// -----------------------------------
// - If param_1[0x203] < 1: call collision_mesh[0x14]() (gradient eval callback)
//   This is the GravityCallback — evaluates gradient at current position
//   Used for non-uniform gravity fields (tilted/curved surfaces)
//
// PHASE 11: SPEED COMPUTATION FROM VELOCITY
// -----------------------------------
// - Save ball position to local vars
// - Get "external velocity" from param_1[0x6A..0x6C] (from collision system)
// - Compute speed from external velocity (magnitude = sqrt(vx^2 + vy^2 + vz^2))
// - Clamp: if speed < 0.001, speed = same_dir (avoid division by zero)
// - Compute speed factor: speed * 0.01 (0x4CF374) clamped to [0.001, 0.1] (0x4CF428, 0x4CF420)
// - Add ball's base speed (param_1[0x54])
// - Clamp total: if > 0.7 (0x4CF41C), subtract 0.02 (0x4CF418) — speed limit
// - Store result in param_1[0x54] (display_speed?)
//
// PHASE 12: COLLISION ITERATION — BALL-BALL & BALL-WALL
// -----------------------------------
// Iterate collision node list (param_1[0x69] + 0x18):
//   NODE TYPE 1 = ball-ball collision:
//     - If same ball ID and in multiplayer: record collision pos as last_ball_collision
//     - If counter > 1 AND no trajectory_override: Ball_ApplyTrajectory
//     - Set flag at param_1[0x1D7] bit 0 (collision occurred)
//     - If trajectory flag set AND gravity conditions met: call ball_vtable[0x20] (BOUNCE)
//     - If collision with OTHER ball (param_1):
//         - Compute relative velocity between balls
//         - Apply equal/opposite impulse: both balls get pushed apart
//         - Speed check: if (other_radius < my_radius * 0.5) AND other_ball collision callback:
//             Award 500-point bonus via Difficulty_GetTimeModifier
//             Store formatted score string (e.g., "+500") in param_1[0x30A]
//             Set counter to 200 (score popup lifetime)
//         - Symmetric check: if (my_radius < other_radius * 0.5) AND my collision callback:
//             Same bonus awarded to OTHER ball
//
//   NODE TYPE 2 = ball-wall collision:
//     - Compute dot(velocity, wall_normal)
//     - If dot > 0.004 (0x4CF4E0): ball moving INTO wall
//       - Compute reflection: V_new = V_old - 2*(V_old·N)*N
//       - Apply friction: V_new *= friction_coefficient
//       - Update ball velocity to reflected direction
//       - Compute distance to previous position
//       - Store in param_1[0x97] (display_distance?)
//
// PHASE 13: FLOOR COLLISION (NODE TYPE 5)
// -----------------------------------
// - If ball_speed > 0.1 (0x4CF420): zero XZ velocity (landing)
// - If ball_speed > 0.1 AND not in dizzy: set trajectory flag (0x2E9)
//   Camera focuses on ball via Scene_SetCamera
// - If in_tube flag (0x313) NOT set:
//     - If ball_wall_hits >= 1 AND floor_count >= 3 AND ball_speed > 0.2:
//       Increment wall_hit counter
//     - If wall_hit counter > 0 AND not dizzy AND ball_speed > 0.1:
//       Increment wall_hit counter more
//
// PHASE 14: ROLL PHYSICS (EXTERNAL VELOCITY APPLICATION)
// -----------------------------------
// - Compute roll friction: base_speed * scale_factors / ball_radius
//   Formula: (((1.0 / 60.0) / (1.0 / (radius / 0.98))) / 0.016) * friction * speed_scale
//   = (radius * 0.98 * friction * speed_scale) / 0.96
// - Clamp by max_speed (param_1[0xA9]):
//   If computed > max_speed: use max_speed, clear wall_contact flag
//   If in_special_mode (0x313): apply 0.25x multiplier
// - Apply: collision_mesh[0xC64] = roll_friction
// - Set velocity direction from roll: vel_X = roll * nx, vel_Y = roll * ny, vel_Z = roll * nz
//
// PHASE 15: ROLLING SOUND & FINAL CALLBACK
// -----------------------------------
// - Call ball_vtable[0x00] (Ball_ApplyForce — accumulate force)
// - If velocity ptr != 0: call velocity callback (vtable[0x00] of velocity obj)
// - Sound_CalculateDistanceAttenuation at ball position
// - If sound ID (param_1[0x96]) != 0: play sound at position
// - RumbleBoard_TickTimer for this ball's rumble channel
//
// PHASE 16: DISPLAY POSITION LERP
// -----------------------------------
// - param_1[0x5F..0x61] = frame_delta_XYZ (current - previous position)
// - Update display position: param_1[0x59..0x5B] = position from physics
// - param_1[0x5B] += speed * 0.02 (lerp toward authoritative position)
// - Clear lerp accumulator
//
// PHASE 17: MULTIPLAYER POSITION SYNC
// -----------------------------------
// - If ball_list_index (param_1[6]) >= 0:
//   Store display_pos in listener_array[ball_index] at (App + 0x854 + index*0xC)
//   Used for network synchronization in multiplayer
//
// PHASE 18: BALL FACING ANGLE
// -----------------------------------
// - Compute facing direction from velocity via Math_Atan2Angle
//   param_1[100] = atan2(-vel_X, vel_Z) [sine]
//   param_1[0x65] = atan2 cos  [cosine]
//   (Wave_Sin/Wave_Cos applied for visual wobble)
// - For gravity planes:
//   plane 0 (flat): atan2(disp_X, disp_Z)
//   plane 1 (tilted): atan2(-vel_Y, vel_Z)
//   plane 2 (vertical): atan2(vel_X, vel_Y)
//
// PHASE 19: GRADIENT FOLLOWING
// -----------------------------------
// - Collision_GradientEval_Stub() — gradient descent step along surface
// - Convert result to int and store in param_1[2]
//
// PHASE 20: ICE/SPIN FRICTION (3 iterations)
// -----------------------------------
// - Only runs if ice_flag (0xC9) == 0
// - For 3 iterations, compute surface speed magnitude
// - If speed <= ball_radius * 0.3 (0x4CF468):
//     - Decay ice counter (0xC6) by 0.03 (0x4CF3D0) per frame
//     - Clamp ice counter to >= 0
//     - Apply ice friction: speed *= 0.95 (0x4CF374)
//     - Set rolling flag (0x31D = 0)
//     - Increase ice counter (0xC5) by speed + 0.8 (0x4CF454)
//     - If ice counter > 0.8: subtract 0.1, set slip flag
//     - Compute slip: (ice_counter - ice_counter_slow) - 0.8
//     - If slipping AND ice_counter_slow == 0: ice_counter -= 0.1
//     - If ice_counter < 0.001 AND not slipping: ice_counter += 0.01 (minimum)
//       (doubled if speed < 0.01, tripled if speed < 0.003)
// - Else (speed > ball_radius * 0.3):
//     - Apply spin friction: speed *= 0.8 (0x4CF3F0)
//     - Set spinning flag (0x31D = 1)
//     - Decrease ice counter: ice_counter -= speed
//     - If decreased > 0.4: set slip flag
//     - Else: add 0.2 back, set slip flag
//
// PHASE 21: DISPLAY POSITION LERP (NETWORK/TELEPORT)
// -----------------------------------
// - If ball_index >= 0 AND not teleporting:
//     - Lerp display position toward authoritative: disp += (auth - disp) * lerp_factor
//     - lerp_factor (param_1[0x1D9]): default 0.95 (0x4CF4C0), decay 0.99 (0x4CF4B8)
//     - If lerp_factor < 0.02 (0x4CF538): clamp to 0.02
//     - If result * 0.99 > 0.1: clamp to 1.0 (snap to target)
//
// PHASE 22: TELEPORT OVERRIDE
// -----------------------------------
// - If teleport_pending flag (0x769): set display = authoritative (snap)
// - If position override flag (0x30F): copy override position to display pos
//
// =============================================================================
// KEY BALL STRUCT FIELDS USED
// =============================================================================
// param_1[0x314] = auto_forward_timer (decays 0.999/frame)
// param_1[0x317] = freeze_counter_1
// param_1[200]   = trail_particle_timer
// param_1[0xBC]  = generic_timer_bc
// param_1[0xBD]  = generic_timer_bd
// param_1[0x315] = ambient_sound_id
// param_1[0x1D2] = gravity_plane_mode (0=flat, 1=tilted, 2=vertical)
// param_1[0x1D3] = gravity_accumulator_xz
// param_1[0x1D4] = gravity_accumulator_xy
// param_1[0x1D1] = generic_decay_counter
// param_1[0xC0]  = freeze_counter
// param_1[0xA8]  = speed_scale_reset_counter
// param_1[0xA7]  = speed_scale (default 1.0)
// param_1[3]     = display_string_lifetime
// param_1[0x30A] = display_string (score popup text)
// param_1[0x53]  = collision_loaded_flag
// param_1[0x1DA] = multiplayer_flag
// param_1[0xC4]  = secret_track_flag
// param_1[0xC1..0xC3] = last_secret_pos (XYZ)
// param_1[0xBF]  = dizzy_modifier (decays or accumulates)
// param_1[0xB6]  = ??? timer with flag at 0x2D5
// param_1[0x204] = mechanical_objects_list (AthenaList)
// param_1[0x205] = mechanical_objects_count
// param_1[0x307] = mechanical_objects_array (float* array)
// param_1[0x2F9] = tar_state_flag
// param_1[0x58..0x5C] = prev_display_pos (XYZ + ???)
// param_1[0x56..0x5A] = prev_pos (XYZ)
// param_1[0x5C..0x5E] = accumulated_velocity (XYZ)
// param_1[0x6A..0x6C] = external_velocity (XYZ from collision)
// param_1[0x69] = collision_mesh_ptr
// param_1[0x97] = display_distance (render fog/draw distance?)
// param_1[0x54] = display_speed
// param_1[0x9E] = collision_mask (used for SpatialTree ctor)
// param_1[0x9F] = collision_id (ball's unique collision ID)
// param_1[0xA1] = ball_radius
// param_1[0x59..0x5B] = authoritative_position (XYZ)
// param_1[0x1D6..0x1D8] = network_lerp_target (XYZ)
// param_1[0x1D9] = network_lerp_factor (0.95 default)
// param_1[0x100..0x102] = facing_sin_cos (at ball radius height)
// param_1[0x66] = facing_angle (atan2 Y)
// param_1[0x67] = ??? flag
// param_1[2]    = gradient_result (int)
// param_1[0xC5] = ice_counter (accumulates while rolling)
// param_1[0xC6] = ice_counter_slow (decays while rolling)
// param_1[0xC9] = ice_present_flag (0=normal, 1=ice surface)
// param_1[0x98] = wall_contact_flag
// param_1[0xBB] = wall_hit_counter
// param_1[0xBD] = floor_hit_counter
// param_1[0x769] = teleport_pending_flag
// param_1[0x30F] = position_override_flag
// param_1[0x310..0x312] = override_position (XYZ)
//
// =============================================================================
// COLLISION NODE TYPES (CollisionNode at collision_mesh + 0x18)
// =============================================================================
// type == 1: BALL-BALL COLLISION
//   piVar17[0x19] = other ball pointer (as int)
//   piVar17[3]    = other ball's collision node
//   piVar17[8..10] = other ball's collision normal (XYZ)
//
// type == 2: BALL-WALL COLLISION  
//   piVar17[0x19] = wall collision data pointer (as int)
//   piVar17[4]    = material pointer
//   piVar17[0x434] = vtable for material
//
// type == 5: BALL-FLOOR COLLISION
//   piVar17[0x15] = penetration depth
//
// =============================================================================
// PHYSICS CONSTANTS (from data section)
// =============================================================================
// 0x4CF310 = 0.1  (gravity accumulator rate)
// 0x4CF368 = 6.0  (gravity max clamp)
// 0x4CF530 = 0.0  (gravity min clamp for tilted plane)
// 0x4CF380 = 0.25 (first frame force multiplier, or 0.01 for roll physics)
// 0x4CF374 = 0.01 (speed factor multiplier)
// 0x4CF420 = 0.1  (floor speed threshold)
// 0x4CF428 = 0.001 (speed clamp min)
// 0x4CF41C = 0.7  (max speed clamp)
// 0x4CF418 = 0.02 (speed decay per frame)
// 0x4CF540 = 0.999 (auto-forward decay)
// 0x4CF538 = 0.02 (auto-forward minimum)
// 0x4CF4E0 = 0.004 (wall dot product threshold)
// 0x4CF4DC = 5.0  (wall collision distance)
// 0x4CF4D8 = 0.25 (distance scale)
// 0x4CF4C0 = 0.95 (network lerp decay)
// 0x4CF4B8 = 0.99 (lerp factor decay)
// 0x4CF468 = 0.3 * radius (ice friction radius threshold)
// 0x4CF3D0 = 0.03 (ice decay rate)
// 0x4CF454 = 0.8  (ice accumulator rate)
// 0x4CF374 = 0.95 (ice friction multiplier)
// 0x4CF3F0 = 0.8  (spin friction multiplier)
// 0x4CF44C = 0.2  (spin recovery rate)
// 0x4CF4C4 = 0.001 (ice minimum threshold)
// 0x4CF48C = 0.003 (ice minimum threshold 2)
// 0x4CF4C8 = 0.4  (slip threshold)
// 0x4CF520 = 0.98 (ball-ball collision multiplier)
// 0x4CF508 = 0.5  (ball size comparison for bonus)
// 0x4CF518 = 0.2  (sound play minimum speed)
// 0x4CF3E0 = 1.0  (sound play maximum speed)
// 0x4CF500 = "%d" (score format string)
// 0x4CF438 = 0.8  (floor velocity multiplier)
// 0x4CF4F0 = 50.0 (camera bounds check)
// 0x4CF4F8 = 10.0 (trajectory axis tolerance)
// 0x4CF4E8 = 0.05 (ball-tube speed threshold)
// 0x4CF308 = 0.001 (??? speed threshold)
//
// =============================================================================
// RELATED FUNCTIONS
// =============================================================================
// Ball_ApplyForceV2 (0x4016F0): Secondary force application with gravity plane
//   awareness. Different force multipliers for tilted/vertical gravity planes.
// Ball_ApplyTrajectory (0x403750): Apply trajectory impulse from E:TRAJECTORY zones
// Ball_ApplyForce (0x402650): Primary force accumulator with state multipliers
// SpatialTree_ctor (0x463330): Build octree for collision broad phase
// CollisionNode_ctor: Create per-ball collision tracking node
// Mesh_FindClosestCollision (0x403980): DDA ray traversal for closest mesh hit
// Scene_CheckPath (0x419E20): Path validity check for facing angle smoothing
// Math_Atan2Angle (0x454E70): Compute angle from vector components
// Wave_Sin/Wave_Cos (0x457DA0/0x457DC0): Sinusoidal wobble for ball facing
// Collision_GradientEval_Stub (0x458190): Gradient descent step callback
//
// =============================================================================
// IMPLEMENTATION SIGNIFICANCE
// =============================================================================
// This function is the CORE of the game's physics feel. Every frame:
// 1. Decays all temporary state (timers, ice, lerp factors)
// 2. Manages mechanical object animations (spinners, rotators, etc.)
// 3. Computes collision response (reflection + friction)
// 4. Handles ball-ball and ball-wall interactions
// 5. Applies ice/spin friction over 3 iterations
// 6. Lerps display position for smooth multiplayer/network sync
// 7. Computes ball facing angle with path smoothing
//
// The 23-step structure reflects a carefully ordered physics pipeline where
// earlier steps set up state for later steps. The gravity reflection in PHASE 9
// is particularly important — it ensures the ball "slides" along surfaces rather
// than accumulating velocity into the gravity plane.
//
// For a clean-room reimplementation, this function would need to be broken into:
// - BallState::tick() — main orchestrator
// - CollisionSystem::buildTree() — spatial partitioning
// - CollisionSystem::resolve() — collision response
// - SurfaceFriction::apply() — ice/spin physics (3 iterations)
// - DisplayLerp::update() — smooth position interpolation
