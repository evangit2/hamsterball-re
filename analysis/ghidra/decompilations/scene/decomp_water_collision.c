/* Water Ripple & Level Collision Systems - Deep Documentation
 *
 * WaterRipple_Render (0x46B070) - Water ripple rendering with mesh and alpha
 * Level_HandleCollision (0x40DCD0) - Level collision event dispatcher
 *
 * ═══════════════════════════════════════════════════════════════
 * WaterRipple_Render (0x46B070)
 * ═══════════════════════════════════════════════════════════════
 *
 * Signature: void __thiscall WaterRipple_Render(WaterRipple* this, float height)
 *
 * RENDERING PIPELINE:
 *   1. WaterRipple_UpdateVertices(this) — animate ripple mesh
 *   2. Graphics_BeginFrame — prepare rendering
 *   3. Compute water surface Y: max(this+0x10, this+0x14) for height
 *   4. Ball_TestPlaneIntersection(this+0x04->collision, 0,0,0, waterY)
 *      — Only render if ball is above water surface
 *   5. Set alpha blending ON (D3DRS_ALPHABLENDENABLE via vtable[200])
 *   6. Mark this+0x04->0x708 = 1 (render state changed)
 *   7. Ball_InitRenderState(this+0x04) — reset render state
 *   8. Graphics_ApplyMaterialAndDraw(this+0x04, this+0x30) — draw ripple mesh
 *   9. Two render paths:
 *      a. If this+0x84 != NULL (index buffer exists):
 *         - D3D DrawIndexedPrimitive (vtable[0x14C]) with index count
 *         - Set index buffer at device+0x08
 *         - Set vertex count (0x118 vtable call, primitive count = this+0x88-2)
 *         - Increment draw counter at this+0x04->0x7CC
 *         - Set sound mode
 *      b. Else (no index buffer):
 *         - D3D DrawPrimitiveUP (vtable[0x120]) with vertex count*2, format 0x20
 *
 * WATER RIPPLE OFFSETS:
 *   this+0x04 = Graphics* (D3D device wrapper)
 *   this+0x08 = vertex count / stride
 *   this+0x0C = vertex format flags
 *   this+0x10 = water Y min
 *   this+0x14 = water Y max
 *   this+0x30 = material data (for ApplyMaterialAndDraw)
 *   this+0x84 = index buffer (IDirect3DIndexBuffer8*)
 *   this+0x88 = index count
 *   this+0x04->0x154 = Graphics* (for vtable[200] alpha blend)
 *   this+0x04->0x708 = render state flag
 *   this+0x04->0x748 = collision mesh pointer (for plane test)
 *   this+0x04->0x7CC = draw call counter
 *
 * ═══════════════════════════════════════════════════════════════
 * Level_HandleCollision (0x40DCD0)
 * ═══════════════════════════════════════════════════════════════
 *
 * Signature: void __thiscall Level_HandleCollision(Level* this, Ball* param_1, CollisionObj* param_2)
 *
 * Level-specific collision event handler. Delegates to CreateNoDizzy() at the end.
 * The event type is read from param_2[1]+0x864 (object name string).
 *
 * E:CATAPULTBOTTOM:
 *   - Only if param_1+0x808 (impact_counter) < 1
 *   - Set impact_counter = 1000 (launch!)
 *   - Iterate this+0x43B8 object list (catapults)
 *   - Find object where obj+0x10D4 == param_2 (matching collision target)
 *   - Set obj+0x10EC = param_1 (ball pointer)
 *   - Call Catapult_Launch(obj)
 *   - Play sound at App+0x464
 *
 * E:OPENSESAME:
 *   - Get first object from this+0x4BEC list (trapdoor list)
 *   - Call Trapdoor_Open(trapdoor)
 *
 * N:TRAPDOOR:
 *   - Iterate this+0x47D0 object list (trapdoors)
 *   - Find objects where obj+0x10D4 == param_2 OR obj+0x10DC == param_2
 *   - Call Trapdoor_Activate(obj)
 *
 * E:BITE:
 *   - Set this+0x43A8 = 0 (reset bite timer?)
 *   - Set this+0x43A0 = 0x41C80000 (25.0f float, bite damage value)
 *
 * E:MACETRIGGER:
 *   - Iterate this+0x5000 object list (maces)
 *   - Set obj+0x10F0 = 1 (activate flag)
 *
 * N:MACE:
 *   - Iterate this+0x5000 object list (maces)
 *   - Find object where obj+0x10D4 == param_2 (matching trigger)
 *     AND obj+0x10E8 != 0x42A00000 (not already at max, 80.0f)
 *     AND obj+0x10F4 == 0 (not already triggered)
 *   - Call param_1->vtable[0x20]() (ball bounce callback)
 *
 * Finally: CreateNoDizzy(this, param_1, param_2)
 *
 * LEVEL OBJECT LIST OFFSETS:
 *   this+0x43B8 = catapult list (AthenaList)
 *   this+0x43BC = catapult count
 *   this+0x47C4 = catapult array
 *   this+0x47D0 = trapdoor list (AthenaList)
 *   this+0x47D4 = trapdoor count
 *   this+0x4BDC = trapdoor array
 *   this+0x4BEC = trapdoor_alt list
 *   this+0x4FF4 = trapdoor_alt array
 *   this+0x5000 = mace list (AthenaList)
 *   this+0x5004 = mace count
 *   this+0x540C = mace array
 *   this+0x878  = App* pointer
 *
 * GAME OBJECT OFFSETS:
 *   obj+0x10D4 = trigger_id (used to match collision source)
 *   obj+0x10DC = secondary_trigger_id
 *   obj+0x10E8 = float value (mace height/speed)
 *   obj+0x10EC = ball_ptr (set during catapult launch)
 *   obj+0x10F0 = activate flag (1 = active)
 *   obj+0x10F4 = triggered flag (0 = not yet)
 */