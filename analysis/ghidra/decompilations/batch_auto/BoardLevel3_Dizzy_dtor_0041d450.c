/*
 * Function: LevelBoard_Dizzy_dtor
 * Address: 0x0041d450
 * Signature: void __fastcall LevelBoard_Dizzy_dtor(undefined4 *param_1)
 *
 * Patterns: SEH frame, scene, board, level. Calls: LevelBoard_Dizzy_dtor, Vtable_CallOffset48, Vec3List_Free, Scene_dtor. Offsets: 2, Lines: 35
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall LevelBoard_Dizzy_dtor(undefined4 *param_1)

{
  void *pvStack_c;
  undefined1 *puStack_8;
  int local_4;
  
  puStack_8 = &LAB_004ca234;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_LevelBoard_Dizzy_dtor_004d0890;
  local_4 = 2;
  if (param_1[0x12f7] != 0) {
    Vtable_CallOffset48(param_1[0x12f7]);
  }
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
  local_4._0_1_ = 1;
  Vec3List_Free(param_1 + 0x11e4);
  local_4 = (uint)local_4._1_3_ << 8;
  Vec3List_Free(param_1 + 0x10de);
  local_4 = 0xffffffff;
  Scene_dtor(param_1);
  ExceptionList = pvStack_c;
  return;
}
