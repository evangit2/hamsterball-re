/*
 * Function: ArenaBoard_Wobbly_DeletingDtor
 * Address: 0x00423790
 * Signature: void __fastcall ArenaBoard_Wobbly_DeletingDtor(undefined4 *param_1)
 *
 * Patterns: SEH frame, board. Calls: ArenaBoard_Wobbly_DeletingDtor, ArenaBoard_dtor. Offsets: 0, Lines: 20
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall ArenaBoard_Wobbly_DeletingDtor(undefined4 *param_1)

{
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cab68;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_ArenaBoard_Expert_ScalarDtor_004d1b18;
  local_4 = 0;
  if ((undefined4 *)param_1[0x11f8] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x11f8])(1);
  }
  param_1[0x11f8] = 0;
  local_4 = 0xffffffff;
  ArenaBoard_dtor(param_1);
  ExceptionList = pvStack_c;
  return;
}
