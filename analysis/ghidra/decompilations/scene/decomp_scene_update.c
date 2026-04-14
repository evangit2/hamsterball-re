/* Scene_Update - Main game tick function
 * Address: 0x419C00
 * Called from: App_Run game loop (0x46BD80) via vtable offset 0x20
 * 
 * Scene struct offsets (int-indexed, multiply by 4 for byte offset):
 * 
 * TICK COUNTER:
 *   [0xD88] = tick_count (incremented each frame)
 *
 * DEMO TIMER SYSTEM:
 *   [0x10D6] = demo_timer_active (bool, demo version countdown)
 *   [0x10D7] = demo_countdown (decremented each tick)
 *   [0x10D8] = demo_accumulator (float time accumulator for popup)
 *   [0x10D9] = demo_frame_counter
 *   [0x10DA] = demo_menu_suppressed (bool, skip menu if too many objects)
 *
 * When demo_countdown hits 0:
 *   - Resets accumulators
 *   - Spawns ScoreDisplay_CtorC with "You have reached the end of the demo..."
 *   - Adds to object list via Scene_AddObject
 *   - Disables further demo timer
 *
 * ESCAPE/PAUSE CHECK:
 *   [0x21E] = app_ptr (back-pointer to App singleton)
 *     +0x5FC = game_mode (3=menu, 4=countdown - these skip ESC check)
 *   [0x220] = pause_suppressed (bool, disable pause menu)
 *   Input_CheckKeyCombo(app, 2) - checks ESC key
 *   Scene_CreateGameOverMenu(scene, 1) - creates pause/quit menu
 *
 * BALL POSITION UPDATE:
 *   [0xA6C] = ball_positions_dirty (bool flag)
 *   [0xA75] = ball_list_iterator_index
 *   [0xA76] = ball_list_size
 *   [0xA77] = ball_list_current
 *   [0xB78] = ball_list_ptr (pointer to ball pointer array)
 *   For each ball: Ball_SetTargetPos(ball, ball->pos.x, ball->pos.y, ball->pos.z)
 *     Ball struct offsets: +0x164=X, +0x168=Y, +0x16C=Z
 *   Clears ball_positions_dirty after update
 *
 * PATH-FOLLOWING GEAR:
 *   [0xD8B] = gear_object_list (AthenaList of Gear objects)
 *   [0xFC7] = gear_has_active_path (bool)
 *   [0xFC8] = gear_path_follower (Gear struct)
 *   When exactly 1 gear object exists AND has active path:
 *     Reads camera position from app->camera (+0x5DC, offsets +0x758/75C/760)
 *     Calls Gear_AdvanceAlongPath(gear, camX, camY, camZ)
 *     -> Gear follows spline path with gradient descent collision avoidance
 *
 * RUMBLE BOARD TIMERS:
 *   [0x221] = rumble_timer_1 (RumbleBoard at offset 0x221*4=0x884)
 *   [0x226] = rumble_timer_2 (RumbleBoard at offset 0x226*4=0x898)
 *   RumbleBoard_TickTimer called for each
 *
 * RUMBLE INTENSITY DECAY:
 *   [0xE93] = rumble_active (bool)
 *   [0xA6E] = rumble_intensity (starts at -800, decays by -10 per frame when active)
 *   When rumble_active: target=-800, else target=0
 *   If intensity not at target and overflow condition met: intensity -= 10
 *
 * OBJECT UPDATE LOOP (SceneObject list at [0x22E]):
 *   [0x22E] = scene_object_list (AthenaList)
 *   [0x22F] = scene_object_count
 *   [0x230] = scene_object_iterator_index
 *   [0x331] = scene_object_array_ptr
 *   For each SceneObject:
 *     vtable[1]() - Update function
 *     if (obj->flags +0x18 != 0):
 *       thunk_Gfx_SetRenderState(scene_obj_list, obj)
 *       vtable[0]() - Render function
 * 
 * PHYSICS/COLLISION UPDATE (when NOT single-gear or camera-locked):
 *   Calls via Scene vtable:
 *     [0x4C]() - UpdateBallPhysics / Step1
 *     [0x50]() - UpdateCollisions / Step2
 *     [0x54]() - ResolveCollisions / Step3
 *     [0x58]() - PostPhysicsUpdate / Step4
 *
 * LEVEL OBJECT UPDATE (another AthenaList at [0xD8B]):
 *   [0xD8C] = level_object_count
 *   [0xD8D] = level_object_iterator_index
 *   [0xE8E] = level_object_array_ptr
 *   For each level object: Scene vtable[0x7C]() - LevelObjectUpdate
 *
 * POST-UPDATE CALLBACK:
 *   [0xEBF] = post_update_callback_obj
 *   callback_obj->vtable[1]() - Final cleanup/notifications
 *
 * EXECUTION ORDER SUMMARY:
 *   1. Increment tick counter
 *   2. Demo timer check (demo version popup)
 *   3. ESC/pause key check
 *   4. Ball position propagation (dirty flag)
 *   5. Gear path following
 *   6. Rumble board timers tick
 *   7. Rumble intensity decay
 *   8. SceneObject update+render loop
 *   9. Physics pipeline (4 vtable calls)
 *   10. Level object update loop
 *   11. Post-update callback
 */
void __fastcall Scene_Update(int *param_1)
{
    // Full decompilation saved - see comments above for documented offsets
}