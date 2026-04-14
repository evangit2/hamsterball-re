/* Level Rendering Pipeline - Deep Documentation
 * 
 * Three functions form the level rendering pipeline:
 *   Level_UpdateAndRender (0x40B600) - Main two-pass render with shadows
 *   Level_RenderObjects (0x40B570)   - Transparent pass after alpha blend ON
 *   Level_RenderDynamicObjects (0x40B420) - Sky, water ripples, dynamic vtable[8]
 *
 * ═══════════════════════════════════════════════════════════════
 * Level_UpdateAndRender (0x40B600)
 * ═══════════════════════════════════════════════════════════════
 *
 * Signature: void __thiscall Level_UpdateAndRender(Scene* this, Ball* param_1)
 *
 * RENDERING PIPELINE (6 phases):
 *
 * PHASE 1: Build visible object list
 *   - Clear visible_list at this+0x3A48 (AthenaList)
 *   - Append all balls from this+0x29D4 (player 1 ball list)
 *   - Append all balls from this+0x3204 (player 2 ball list)
 *
 * PHASE 2: Opaque render pass
 *   - If param_1+0x70C != 0: turn alpha blending OFF
 *     (Graphics_vtable[200](0xE, 0) via param_1+0x154)
 *   - Increment render counter at param_1+0x7C8
 *   - Iterate ball_list_1: call each ball->vtable[0x1C]() (opaque render)
 *   - Iterate ball_list_2: call each ball->vtable[0x1C]() (opaque render)
 *
 * PHASE 3: Alpha blending ON
 *   - If param_1+0x70C != 1: turn alpha blending ON
 *     (Graphics_vtable[200](0xE, 1) via param_1+0x154)
 *   - Increment render counter
 *
 * PHASE 4: Waypoint arrow (conditional)
 *   - Conditions: App+0x220[0x11] != 0 (race active)
 *                App+0x234 == 0 (not paused?)
 *                App+0x910 != NULL (waypoint list exists)
 *   - Call WaypointList_SetNextWaypoint(App+0x910, this+0x361C)
 *   - Set scale 0.3EE66666 (~0.45?) at waypoint+0x2FC
 *   - Call waypoint->vtable[8]()
 *
 * PHASE 5: Visible objects render
 *   - Iterate visible_list at this+0x3A48
 *   - Call each object->vtable[8]() (standard render)
 *
 * PHASE 6: Ball shadows (conditional)
 *   - If this+0x8AC[0xCB8] (AthenaList at Level+some offset) has items:
 *   - Iterate ball_list_1: call Ball_RenderShadow(ball)
 *   - Iterate ball_list_2: call Ball_RenderShadow(ball)
 *
 * SCENE OFFSETS:
 *   +0x29D4  = ball_list_1 (AthenaList<Ball*>, player 1)
 *   +0x29D8  = ball_list_1_count
 *   +0x29DC  = ball_list_1_iterator
 *   +0x2DE0  = ball_list_1_array (Ball**)
 *   +0x3204  = ball_list_2 (AthenaList<Ball*>, player 2)
 *   +0x3208  = ball_list_2_count
 *   +0x320C  = ball_list_2_iterator
 *   +0x3610  = ball_list_2_array (Ball**)
 *   +0x361C  = waypoint_arrow (SceneObject*)
 *   +0x3A44  = use_skydome flag (bool)
 *   +0x3A48  = visible_object_list (AthenaList)
 *   +0x3AFC  = dynamic_object (has vtable[8])
 *   +0x3F18  = WaterRipple object
 *   +0x8AC   = Level* (contains collision/render data)
 *   +0x8B0   = Level* (secondary, for skydome vtable[48])
 *   +0x878   = App* back-pointer
 *
 * BALL OFFSETS:
 *   +0x154  = Graphics* (D3D device wrapper)
 *   +0x70C  = alpha_blend_state (0=off, 1=on)
 *   +0x7C8  = render_call_counter
 *
 * ═══════════════════════════════════════════════════════════════
 * Level_RenderObjects (0x40B570)
 * ═══════════════════════════════════════════════════════════════
 *
 * Signature: void __thiscall Level_RenderObjects(Scene* this, Graphics* param_1)
 *
 * TRANSPARENT RENDER PASS:
 *   1. Call Graphics_BeginFrame(param_1, 0)
 *   2. Call this+0x8AC->vtable[0x4C]() (Level-specific render, likely mesh)
 *   3. Call Graphics_BeginFrame(param_1, 0) again
 *   4. Iterate visible_object_list at this+0x3A48
 *   5. Call each object->vtable[0x0C]() (transparent render callback)
 *
 * Note: vtable[0x0C] is the transparent render callback, while vtable[0x08]
 * is the standard render and vtable[0x1C] is the opaque render.
 *
 * ═══════════════════════════════════════════════════════════════
 * Level_RenderDynamicObjects (0x40B420)
 * ═══════════════════════════════════════════════════════════════
 *
 * Signature: void __fastcall Level_RenderDynamicObjects(Scene* this)
 *
 * DYNAMIC OBJECTS RENDER (3 phases):
 *
 * PHASE 1: Sky / background
 *   - If this+0x3A44 == 0: call this+0x8AC->vtable[0x48]() (skybox render)
 *   - If this+0x3A44 == 1: call this+0x8B0->vtable[0x48](1, 1) (skydome render)
 *
 * PHASE 2: Water ripples
 *   - Iterate ripple list at this+0x2160 (AthenaList<Ripple*>)
 *   - For each ripple:
 *     a. Timer_Init / Timer_Cleanup scope
 *     b. Gfx_SetPositionAndRender(ripple+0x1C, +0x20, +0x24) (rotation/anim)
 *     c. Gfx_ScaleX(DAT_004CF44C - ripple+0x14) (scale X from time)
 *     d. Gfx_ScaleZ(-ripple+0x10) (scale Z from time)
 *     e. Gfx_SetPosition(ripple+0x04, +0x08, +0x0C) (world position)
 *     f. WaterRipple_Render(this+0x3F18, &timer)
 *
 * PHASE 3: Dynamic object vtable callback
 *   - Call this+0x3AFC->vtable[8]() (dynamic object render callback)
 *
 * RIPPLE OBJECT OFFSETS:
 *   +0x04  = X position
 *   +0x08  = Y position
 *   +0x0C  = Z position
 *   +0x10  = scale Z factor (negated)
 *   +0x14  = scale X time factor
 *   +0x1C  = rotation/anim X
 *   +0x20  = rotation/anim Y
 *   +0x24  = rotation/anim Z
 *
 * ═══════════════════════════════════════════════════════════════
 * COMPLETE LEVEL RENDER FLOW
 * ═══════════════════════════════════════════════════════════════
 *
 * From Scene_Render (0x41A2E0), the render order is:
 *
 * 1. Level_RenderDynamicObjects  - Sky/skydome, water ripples, dynamic vtable
 * 2. Level_UpdateAndRender        - Opaque balls, alpha blend, waypoints, visible, shadows
 * 3. Level_RenderObjects          - Transparent objects after alpha ON
 *
 * This ensures:
 *   - Sky renders first (background)
 *   - Water ripples render on top of sky
 *   - Opaque ball geometry renders before transparent
 *   - Alpha blending is toggled ON for transparent objects
 *   - Shadows render last (on top of everything)
 *   - Transparent pass runs after opaque + shadows
 */