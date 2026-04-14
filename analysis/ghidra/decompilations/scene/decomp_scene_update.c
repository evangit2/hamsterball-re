/* Scene_Update (0x419C00) - Main Game Tick Deep Documentation
 *
 * This is THE central game loop function. Called every frame from App_Run.
 * Handles demo timer, input, ball propagation, object updates, physics.
 *
 * ═══════════════════════════════════════════════════════════════
 * SCENE_UPDATE EXECUTION ORDER
 * ═══════════════════════════════════════════════════════════════
 *
 * STEP 1: Frame counter increment
 *   this+0xD88 (= 0x3620) = frame counter (incremented each tick)
 *
 * STEP 2: Demo timer check (if this+0x10D6 != 0)
 *   this+0x10D7 (0x438C) = demo countdown timer, decremented
 *   When timer reaches 0:
 *     this+0x10D8 (0x4390) = frame counter reset to 0
 *     this+0x10D9 (0x4394) = secondary counter reset to 0  
 *   If elapsed > DAT_004D039C (demo time limit):
 *     this+0x21D (0x874) = bool = 1 (showing demo popup)
 *     Create ScoreDisplay_CtorC with "You have reached the end of the
 *       demo version..." message
 *     Scene_AddObject to add the popup
 *     this+0x10D6 (0x4388) = 0 (disable demo timer)
 *     Increment this+0x10D8 by DAT_004CF3E0 (demo timer step)
 *
 * STEP 3: ESC key / menu check
 *   If this+0x10DA == 0 OR objects < 2:
 *     If App+0x5FC input mode not 3-4, and this+0x220 paused flag == 0:
 *       Input_CheckKeyCombo(App, 2) → Scene_CreateGameOverMenu(this, 1)
 *
 * STEP 4: Ball position propagation (if this+0xA6C != 0)
 *   Iterate ball list at this+0xA75
 *   For each ball: Ball_SetTargetPos(ball, ball+0x164, ball+0x168, ball+0x16C)
 *   Set this+0xA6C = 0 (propagation done)
 *
 * STEP 5: Gear path following (if objects==1 AND this+0xFC7 != 0)
 *   Read position from App+0x5DC → +0x758/75C/760
 *   Gear_AdvanceAlongPath(this+0xFC8, X, Y, Z)
 *
 * STEP 6: Rumble timer ticks
 *   RumbleBoard_TickTimer(this+0x221)
 *   RumbleBoard_TickTimer(this+0x226)
 *
 * STEP 7: Rumble camera shake decay
 *   this+0xE93 = shake_active flag
 *   this+0xA6E = shake_magnitude (decays by -10 per frame when > -800)
 *   When shake_active: target is -800, when not: target is 0
 *   If crossing zero → stop decay
 *
 * STEP 8: Scene object update + render
 *   Iterate this+0x22E (AthenaList of scene objects)
 *   For each object:
 *     obj->vtable[4]() — Update tick
 *     If obj+0x18 (flag) != 0:
 *       Gfx_SetRenderState(scene, obj) — apply render state
 *       obj->vtable[0]() — Render tick
 *
 * STEP 9: Physics pipeline (if single player OR App+0x5DC[0x14C]==0)
 *   this->vtable[0x4C]() — Physics step 1
 *   this->vtable[0x50]() — Physics step 2
 *   this->vtable[0x54]() — Physics step 3
 *   this->vtable[0x58]() — Physics step 4
 *   Iterate this+0xD8B (physics objects list):
 *     obj->vtable[0x7C]() — Object physics update
 *   this+0xEBF+0x04() — Post-physics callback
 *
 * ═══════════════════════════════════════════════════════════════
 * SCENE OFFSETS (from this pointer)
 * ═══════════════════════════════════════════════════════════════
 *
 * +0x022E: scene_objects (AthenaList) — object list for update/render
 * +0x022F: object_count (int)
 * +0x0331: object_array (void**)
 * +0x0A6C: ball_propagate_flag (byte, 1 = need to propagate)
 * +0x0A6E: camera_shake_magnitude (int, decays to 0 or -800)
 * +0x0A75: ball_list (AthenaList<Ball*>)
 * +0x0A76: ball_count
 * +0x0A77: ball_iterator
 * +0x0B78: ball_array (Ball**)
 * +0x0D88: frame_counter (int, incremented each tick)
 * +0x0D8B: physics_objects (AthenaList)
 * +0x0D8C: physics_count
 * +0x0D8D: physics_iterator
 * +0x0E8E: physics_array
 * +0x0E93: camera_shake_active (bool)
 * +0x0FC7: gear_enabled (bool, for single-gear mode)
 * +0x0FC8: gear_path (Gear path data)
 * +0x10D6: demo_timer_active (int, !=0 = counting down)
 * +0x10D7: demo_timer_countdown (int, frames remaining)
 * +0x10D8: demo_elapsed_frames (int)
 * +0x10D9: demo_frame_counter (int)
 * +0x10DA: demo_ignore_esc (byte, 0 = allow ESC)
 * +0x10DA: demo_shown_popup (byte)
 * +0x21D:  showing_demo_popup (byte)
 * +0x21E:  App* (back-pointer for scene)
 * +0x220:  paused_flag (byte)
 * +0x221:  RumbleBoard timer 1 (0x14 bytes)
 * +0x226:  RumbleBoard timer 2 (0x14 bytes)
 * +0xEBF:  post_physics_callback (function pointer)
 */