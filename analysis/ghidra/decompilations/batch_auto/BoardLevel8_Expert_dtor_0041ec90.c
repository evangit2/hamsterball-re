/*
 * Function: LevelBoard_Expert_dtor
 * Address: 0x0041ec90
 * Signature: void __fastcall LevelBoard_Expert_dtor(undefined4 *param_1)
 *
 * Patterns: SEH frame, scene, board, level. Calls: LevelBoard_Expert_dtor, Vec3List_Free, Scene_dtor. Offsets: 3, Lines: 42
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall LevelBoard_Expert_dtor(undefined4 *param_1)

{
  void *pvStack_c;
  undefined1 *puStack_8;
  int local_4;
  
  puStack_8 = &LAB_004ca432;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_LevelBoard_Expert_dtor_004d0b00;
  local_4 = 3;
  if ((undefined4 *)param_1[0x10de] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x10de])(1);
  }
  param_1[0x10de] = 0;
  if ((undefined4 *)param_1[0x10df] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x10df])(1);
  }
  param_1[0x10df] = 0;
  if ((undefined4 *)param_1[0x12ec] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x12ec])(1);
  }
  param_1[0x12ec] = 0;
  if ((undefined4 *)param_1[0x12ed] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x12ed])(1);
  }
  param_1[0x12ed] = 0;
  if ((undefined4 *)param_1[0x12ee] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x12ee])(1);
  }
  param_1[0x12ee] = 0;
  local_4._0_1_ = 2;
  Vec3List_Free(param_1 + 0x12ef);
  local_4._0_1_ = 1;
  Vec3List_Free(param_1 + 0x11e6);
  local_4 = (uint)local_4._1_3_ << 8;
  Vec3List_Free(param_1 + 0x10e0);
  local_4 = 0xffffffff;
  Scene_dtor(param_1);
  ExceptionList = pvStack_c;
  return;
}
