/*
 * Function: BoardLevel_Sky_Dtor
 * Address: 0x0041fbc0
 * Signature: void __fastcall BoardLevel_Sky_Dtor(undefined4 *param_1)
 *
 * Patterns: SEH frame, scene, board, level. Calls: BoardLevel_Sky_Dtor, Vec3List_Free, Scene_dtor. Offsets: 1, Lines: 39
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall BoardLevel_Sky_Dtor(undefined4 *param_1)

{
  void *pvStack_c;
  undefined1 *puStack_8;
  uint local_4;
  
  puStack_8 = &LAB_004ca606;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_BoardLevel_Generic_dtor_004d0fc8;
  local_4 = 1;
  if ((undefined4 *)param_1[0x11eb] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x11eb])(1);
  }
  if ((undefined4 *)param_1[0x10dc] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x10dc])(1);
  }
  if ((undefined4 *)param_1[0x10db] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x10db])(1);
  }
  if ((undefined4 *)param_1[0x10e3] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x10e3])(1);
  }
  if ((undefined4 *)param_1[0x10e1] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x10e1])(1);
  }
  if ((undefined4 *)param_1[0x10e2] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x10e2])(1);
  }
  if ((undefined4 *)param_1[0x10dd] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x10dd])(1);
  }
  local_4 = local_4 & 0xffffff00;
  Vec3List_Free(param_1 + 0x10e5);
  local_4 = 0xffffffff;
  Scene_dtor(param_1);
  ExceptionList = pvStack_c;
  return;
}
