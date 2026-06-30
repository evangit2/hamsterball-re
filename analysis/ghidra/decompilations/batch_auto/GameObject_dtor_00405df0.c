/*
 * Function: GameObject_dtor
 * Address: 0x00405df0
 * Signature: void __fastcall GameObject_dtor(undefined4 *param_1)
 *
 * Patterns: frees memory, SEH frame, matrix math, board. Calls: GameObject_dtor, Vtable_CallOffset48, _free, Vec3List_Free, Matrix_Identity, ToggleTimer_Cleanup, Matrix4_Identity, Timer_Cleanup. Offsets: 7, Lines: 44
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall GameObject_dtor(undefined4 *param_1)

{
  void *pvStack_c;
  undefined1 *puStack_8;
  int iStack_4;
  
  puStack_8 = &LAB_004c91ad;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_GameObject_sub2_dtor_004cf314;
  iStack_4 = 7;
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
  iStack_4._0_1_ = 6;
  Vec3List_Free(param_1 + 0x204);
  iStack_4._0_1_ = 5;
  Vec3List_Free(param_1 + 0xcb);
  iStack_4._0_1_ = 4;
  Matrix_Identity(param_1 + 0xaa);
  iStack_4._0_1_ = 3;
  ToggleTimer_Cleanup(param_1 + 0x99);
  iStack_4._0_1_ = 2;
  Matrix4_Identity(param_1 + 0x82);
  iStack_4._0_1_ = 1;
  Matrix4_Identity(param_1 + 0x6e);
  iStack_4 = (uint)iStack_4._1_3_ << 8;
  Timer_Cleanup(param_1 + 0x42);
  iStack_4 = 0xffffffff;
  UITimer_dtor(param_1 + 7);
  ExceptionList = pvStack_c;
  return;
}
