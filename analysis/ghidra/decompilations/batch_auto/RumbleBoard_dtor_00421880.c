/*
 * Function: ArenaBoard_dtor
 * Address: 0x00421880
 * Signature: void __fastcall ArenaBoard_dtor(undefined4 *param_1)
 *
 * Patterns: SEH frame, scene, board. Calls: ArenaBoard_dtor, ToggleTimer_Cleanup, Vec3List_Free, Scene_dtor. Offsets: 2, Lines: 26
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall ArenaBoard_dtor(undefined4 *param_1)

{
  void *pvStack_c;
  undefined1 *puStack_8;
  int local_4;
  
  puStack_8 = &LAB_004ca884;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_ArenaBoard_dtor_004d1358;
  local_4 = 2;
  if ((undefined4 *)param_1[0x10e3] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x10e3])(1);
  }
  if ((undefined4 *)param_1[0x10e4] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x10e4])(1);
  }
  local_4._0_1_ = 1;
  ToggleTimer_Cleanup(param_1 + 0x11f2);
  local_4 = (uint)local_4._1_3_ << 8;
  Vec3List_Free(param_1 + 0x10e5);
  local_4 = 0xffffffff;
  Scene_dtor(param_1);
  ExceptionList = pvStack_c;
  return;
}
