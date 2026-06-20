/*
 * Function: GameObject_dtor
 * Address: 0x00401480
 * Signature: void __fastcall GameObject_dtor(undefined4 *param_1)
 *
 * Description:
 * Full destructor for the GameObject base class (which Ball, Scene, Board, and
 * other game entities inherit from). This is a complex destructor that:
 *
 *   1. Sets the vtable to PTR_GameObject_sub2_dtor_004cf314 (base class vtable)
 *   2. Cleans up two child objects at offsets +0x96*4=+0x258 and +0x315*4=+0xCD4
 *      via Vtable_CallOffset48 (virtual destructor dispatch on owned sub-objects)
 *   3. Calls a virtual function on the object at +0x69*4=+0x1A4 (likely a CollisionMesh
 *      or SceneObject sub-component's destructor, called with arg=1 meaning "delete")
 *   4. Frees the memory at +0x30A*4=+0xC28 (a heap-allocated buffer, likely the ball name
 *      or a collision data buffer)
 *   5. Frees Vec3List at +0x204*4=+0x810 (ball trail particles list)
 *   6. Frees Vec3List at +0xCB*4=+0x32C (another Vec3 list, likely collision contacts)
 *   7. Resets matrix at +0xAA*4=+0x2A8 to identity
 *   8. Cleans up RumbleBoard timer at +0x99*4=+0x264
 *   9. Resets two 4x4 matrices at +0x82*4=+0x208 and +0x6E*4=+0x1B8 to identity
 *  10. Cleans up Timer at +0x42*4=+0x108
 *  11. Destroys UITimer at offset +7*4=+0x1C
 *
 * Uses SEH (Structured Exception Handling) for stack unwinding safety.
 * The local_4 variable tracks the cleanup stage (7 down to 0) so that if an
 * exception occurs, the unwind handler knows which resources have already been freed.
 *
 * Cross-references:
 *   - Called from Ball_dtor2 (0x401CC0) — Ball inherits from GameObject
 *   - Called from GameObject_sub2_dtor (0x402A50) — sub-class destructor
 *   - Called from Board_ctor (0x419030) — during board construction (base init)
 *   - Referenced from Unwind handlers (exception unwinding)
 *   - Duplicated at 0x405DF0 (same code, different address)
 *
 * Note: This destructor reveals the GameObject struct layout:
 *   +0x000: vtable pointer
 *   +0x01C: UITimer
 *   +0x108: Timer
 *   +0x1A4: CollisionMesh (or SceneObject sub-component)
 *   +0x1B8: 4x4 Matrix (transform)
 *   +0x208: 4x4 Matrix (rotation/scale)
 *   +0x264: RumbleBoard timer
 *   +0x258: Child object 1 (owned, virtual destructor)
 *   +0x2A8: Matrix (identity)
 *   +0x32C: Vec3List (collision contacts)
 *   +0x810: Vec3List (trail particles)
 *   +0xC28: Heap buffer (name or data)
 *   +0xCD4: Child object 2 (owned, virtual destructor)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall GameObject_dtor(undefined4 *param_1)

{
  void *pvStack_c;
  undefined1 *puStack_8;
  int local_4;
  
  puStack_8 = &LAB_004c91ad;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_GameObject_sub2_dtor_004cf314;
  local_4 = 7;
  if (param_1[0x96] != 0) {
    Vtable_CallOffset48(param_1[0x96]);
  }
  if (param_1[0x315] != 0) {
    Vtable_CallOffset48(param_1[0x315]);
  }
  if ((undefined4 *)param_1[0x69] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x69])(1);
    param_1[0x69] = 0;
  }
  if ((void *)param_1[0x30a] != (void *)0x0) {
    _free((void *)param_1[0x30a]);
    param_1[0x30a] = 0;
  }
  local_4._0_1_ = 6;
  Vec3List_Free(param_1 + 0x204);
  local_4._0_1_ = 5;
  Vec3List_Free(param_1 + 0xcb);
  local_4._0_1_ = 4;
  Matrix_Identity(param_1 + 0xaa);
  local_4._0_1_ = 3;
  RumbleBoard_CleanupTimer(param_1 + 0x99);
  local_4._0_1_ = 2;
  Matrix4_Identity(param_1 + 0x82);
  local_4._0_1_ = 1;
  Matrix4_Identity(param_1 + 0x6e);
  local_4 = (uint)local_4._1_3_ << 8;
  Timer_Cleanup(param_1 + 0x42);
  local_4 = 0xffffffff;
  UITimer_dtor(param_1 + 7);
  ExceptionList = pvStack_c;
  return;
}
