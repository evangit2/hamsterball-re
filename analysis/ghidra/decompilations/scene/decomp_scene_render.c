/* Scene_Render - Rendering dispatch (1P/2P split screen support)
 * Address: 0x41A2E0
 * Called from: App_Run game loop via vtable offset 0x28
 * 
 * Scene struct offsets:
 *   +0x362C = player_list (AthenaList of player viewports)
 *   +0x3630 = player_count (int)
 *   +0x3634 = player_iterator_index
 *   +0x3A38 = player_array_ptr (Ball* array)
 *   +0x87C  = D3D_viewport_obj (viewport interface)
 *   +0x878  = scene_manager_ptr
 *   +0x29D0 = current_ball_ptr (Ball being tracked by camera)
 *
 * RENDER PIPELINE (per viewport):
 *   Graphics_SetViewport(params)  - Set clip rect for 1P/2P
 *   Scene_SetCamera(scene, ball, 1) - Position camera around ball
 *   vtable[0x60](gfx)  - RenderBackground (sky/far plane)
 *   vtable[0x64](gfx)  - RenderOpaqueObjects (level geometry)
 *   vtable[0x68](gfx)  - RenderTransparentObjects (glass, effects)
 *   vtable[0x70](gfx)  - RenderOverlay (HUD, score)
 *   vtable[0x6C](gfx)  - RenderPostEffects (fade, transitions)
 *
 * SPLIT SCREEN LOGIC:
 *   player_count == 0: Full screen, no ball tracking
 *   player_count == 1: Full screen, camera tracks ball[0]
 *     Ball position copied to scene_manager::camera_target (+0x854-85C)
 *     SetCamera positions view around ball->pos (+0x164/168/16C)
 *   player_count == 2: Split screen, each player gets half
 *     Iterates ball list, sets viewport half for each
 *     Each ball gets its own SetCamera call
 *
 * KEY OBSERVATION:
 *   The rendering order is: Background -> Opaque -> Transparent -> Overlay -> PostFX
 *   This is a standard depth-sorted rendering pipeline with
 *   separate passes for opaque and transparent geometry.
 *   2P mode splits viewport horizontally (Graphics_SetViewport handles rects).
 */
void __thiscall Scene_Render(void *this, int *param_1)
{
    // Full decompilation saved - see comments above for documented offsets
}