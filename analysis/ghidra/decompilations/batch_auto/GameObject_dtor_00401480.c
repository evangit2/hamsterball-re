/*
 * Function: GameObject_dtor
 * Address: 0x00401480
 * Signature: void __fastcall GameObject_dtor(undefined4 *param_1)
 *
 * Patterns: frees memory, SEH frame, matrix math, board. Calls: GameObject_dtor, Vtable_CallOffset48, _free, Vec3List_Free, Matrix_Identity, RumbleBoard_CleanupTimer, Matrix4_Identity, Timer_Cleanup. Offsets: 7, Lines: 44
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
