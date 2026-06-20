/*
 * Function: BoardLevel_Toob_dtor
 * Address: 0x0041f720
 * Signature: void __fastcall BoardLevel_Toob_dtor(undefined4 *param_1)
 *
 * Patterns: SEH frame, scene, board, level. Calls: BoardLevel_Toob_dtor, _eh_vector_destructor_iterator_, Scene_dtor. Offsets: 1, Lines: 33
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall BoardLevel_Toob_dtor(undefined4 *param_1)

{
  void *pvStack_c;
  undefined1 *puStack_8;
  uint local_4;
  
  puStack_8 = &LAB_004ca573;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_BoardLevel_Toob_dtor_004d0e78;
  local_4 = 1;
  if ((undefined4 *)param_1[0x10db] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x10db])(1);
  }
  if ((undefined4 *)param_1[0x10dc] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x10dc])(1);
  }
  if ((undefined4 *)param_1[0x10dd] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x10dd])(1);
  }
  if ((undefined4 *)param_1[0x10de] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x10de])(1);
  }
  if ((undefined4 *)param_1[0x10df] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x10df])(1);
  }
  local_4 = local_4 & 0xffffff00;
  _eh_vector_destructor_iterator_(param_1 + 0x10e3,0x418,8,Vec3List_Free);
  local_4 = 0xffffffff;
  Scene_dtor(param_1);
  ExceptionList = pvStack_c;
  return;
}
