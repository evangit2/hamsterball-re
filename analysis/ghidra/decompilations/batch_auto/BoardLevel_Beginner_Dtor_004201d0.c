/*
 * Function: BoardLevel_Beginner_Dtor
 * Address: 0x004201d0
 * Signature: void __fastcall BoardLevel_Beginner_Dtor(undefined4 *param_1)
 *
 * Patterns: SEH frame, scene, board, level. Calls: BoardLevel_Beginner_Dtor, _eh_vector_destructor_iterator_, Scene_dtor. Offsets: 1, Lines: 17
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall BoardLevel_Beginner_Dtor(undefined4 *param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004ca4c8;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *param_1 = &PTR_BoardLevel_Generic_dtor2_004d1098;
  local_4 = 0;
  _eh_vector_destructor_iterator_(param_1 + 0x10db,0x418,8,Vec3List_Free);
  local_4 = 0xffffffff;
  Scene_dtor(param_1);
  ExceptionList = local_c;
  return;
}
