/*
 * Function: ArenaBoard_Tower_Dtor
 * Address: 0x00422a60
 * Signature: void __fastcall ArenaBoard_Tower_Dtor(undefined4 *param_1)
 *
 * Patterns: SEH frame, board. Calls: ArenaBoard_Tower_Dtor, Vec3List_Free, ArenaBoard_dtor. Offsets: 2, Lines: 32
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall ArenaBoard_Tower_Dtor(undefined4 *param_1)

{
  void *pvStack_c;
  undefined1 *puStack_8;
  int local_4;
  
  puStack_8 = &LAB_004ca9a4;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_ArenaBoard_Odd_DeletingDtor_004d1740;
  local_4 = 2;
  if ((undefined4 *)param_1[0x11f8] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x11f8])(1);
  }
  param_1[0x11f8] = 0;
  if ((undefined4 *)param_1[0x12ff] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x12ff])(1);
  }
  param_1[0x12ff] = 0;
  if ((undefined4 *)param_1[0x1406] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x1406])(1);
  }
  param_1[0x1406] = 0;
  local_4._0_1_ = 1;
  Vec3List_Free(param_1 + 0x1300);
  local_4 = (uint)local_4._1_3_ << 8;
  Vec3List_Free(param_1 + 0x11f9);
  local_4 = 0xffffffff;
  ArenaBoard_dtor(param_1);
  ExceptionList = pvStack_c;
  return;
}
