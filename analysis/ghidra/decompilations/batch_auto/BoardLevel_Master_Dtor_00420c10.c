/*
 * Function: BoardLevel_Master_Dtor
 * Address: 0x00420c10
 * Signature: void __fastcall BoardLevel_Master_Dtor(undefined4 *param_1)
 *
 * Patterns: SEH frame, scene, board, level. Calls: BoardLevel_Master_Dtor, Vec3List_Free, _eh_vector_destructor_iterator_, Scene_dtor. Offsets: 5, Lines: 66
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall BoardLevel_Master_Dtor(undefined4 *param_1)

{
  void *pvStack_c;
  undefined1 *puStack_8;
  int local_4;
  
  puStack_8 = &LAB_004ca80b;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_BoardLevel_Generic_dtor4_004d12b0;
  local_4 = 5;
  if ((undefined4 *)param_1[0x10e5] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x10e5])(1);
  }
  param_1[0x10e5] = 0;
  if ((undefined4 *)param_1[0x10e6] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x10e6])(1);
  }
  param_1[0x10e6] = 0;
  if ((undefined4 *)param_1[0x1504] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x1504])(1);
  }
  param_1[0x1504] = 0;
  if ((undefined4 *)param_1[0x1505] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x1505])(1);
  }
  param_1[0x1505] = 0;
  if ((undefined4 *)param_1[0x1508] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x1508])(1);
  }
  param_1[0x1508] = 0;
  if ((undefined4 *)param_1[0x1509] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x1509])(1);
  }
  param_1[0x1509] = 0;
  if ((undefined4 *)param_1[0x1610] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x1610])(1);
  }
  param_1[0x1610] = 0;
  if ((undefined4 *)param_1[0x1611] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x1611])(1);
  }
  param_1[0x1611] = 0;
  if ((undefined4 *)param_1[0x1612] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x1612])(1);
  }
  param_1[0x1612] = 0;
  if ((undefined4 *)param_1[0x181f] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x181f])(1);
  }
  param_1[0x181f] = 0;
  local_4._0_1_ = 4;
  Vec3List_Free(param_1 + 0x1820);
  local_4._0_1_ = 3;
  Vec3List_Free(param_1 + 0x1719);
  local_4._0_1_ = 2;
  Vec3List_Free(param_1 + 0x1613);
  local_4._0_1_ = 1;
  Vec3List_Free(param_1 + 0x150a);
  local_4 = (uint)local_4._1_3_ << 8;
  _eh_vector_destructor_iterator_(param_1 + 0x10e7,0x418,4,Vec3List_Free);
  local_4 = 0xffffffff;
  Scene_dtor(param_1);
  ExceptionList = pvStack_c;
  return;
}
