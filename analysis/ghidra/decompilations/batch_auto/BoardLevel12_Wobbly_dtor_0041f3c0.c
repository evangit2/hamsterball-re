/*
 * Function: BoardLevel12_Wobbly_dtor
 * Address: 0x0041f3c0
 * Signature: void __fastcall BoardLevel12_Wobbly_dtor(undefined4 *param_1)
 *
 * Patterns: SEH frame, scene, board, level. Calls: BoardLevel12_Wobbly_dtor, Scene_dtor. Offsets: 0, Lines: 44
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall BoardLevel12_Wobbly_dtor(undefined4 *param_1)

{
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004ca4c8;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_BoardLevel12_Wobbly_dtor_004d0d38;
  local_4 = 0;
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
  if ((undefined4 *)param_1[0x10df] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x10df])(1);
  }
  param_1[0x10df] = 0;
  if ((undefined4 *)param_1[0x10e0] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x10e0])(1);
  }
  param_1[0x10e0] = 0;
  if ((undefined4 *)param_1[0x10e1] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x10e1])(1);
  }
  param_1[0x10e1] = 0;
  local_4 = 0xffffffff;
  Scene_dtor(param_1);
  ExceptionList = pvStack_c;
  return;
}
