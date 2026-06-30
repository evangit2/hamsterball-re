/*
 * Function: LevelBoard_Tower_dtor
 * Address: 0x0041e640
 * Signature: void __fastcall LevelBoard_Tower_dtor(undefined4 *param_1)
 *
 * Patterns: SEH frame, scene, board, level. Calls: LevelBoard_Tower_dtor, Vec3List_Free, Scene_dtor. Offsets: 4, Lines: 48
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall LevelBoard_Tower_dtor(undefined4 *param_1)

{
  void *pvStack_c;
  undefined1 *puStack_8;
  int local_4;
  
  puStack_8 = &LAB_004ca370;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_LevelBoard_Tower_dtor_004d0a08;
  local_4 = 4;
  if ((undefined4 *)param_1[0x10db] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x10db])(1);
  }
  param_1[0x10db] = 0;
  if ((undefined4 *)param_1[0x10dc] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x10dc])(1);
  }
  param_1[0x10dc] = 0;
  if ((undefined4 *)param_1[0x10dd] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x10dd])(1);
  }
  param_1[0x10dd] = 0;
  if ((undefined4 *)param_1[0x10de] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x10de])(1);
  }
  param_1[0x10de] = 0;
  if ((undefined4 *)param_1[0x10e4] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x10e4])(1);
  }
  param_1[0x10e4] = 0;
  if ((undefined4 *)param_1[0x10ed] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x10ed])(1);
  }
  param_1[0x10ed] = 0;
  local_4._0_1_ = 3;
  Vec3List_Free(param_1 + 0x1400);
  local_4._0_1_ = 2;
  Vec3List_Free(param_1 + 0x12fa);
  local_4._0_1_ = 1;
  Vec3List_Free(param_1 + 0x11f4);
  local_4 = (uint)local_4._1_3_ << 8;
  Vec3List_Free(param_1 + 0x10ee);
  local_4 = 0xffffffff;
  Scene_dtor(param_1);
  ExceptionList = pvStack_c;
  return;
}
