/*
 * Function: Ball_dtor2
 * Address: 0x00401cc0
 * Signature: void __fastcall Ball_dtor2(undefined4 *param_1)
 *
 * Patterns: SEH frame, matrix math, ball. Calls: Ball_dtor2, Matrix_Identity, GameObject_dtor. Offsets: 1, Lines: 17
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall Ball_dtor2(undefined4 *param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004c9208;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *param_1 = &PTR_Ball_dtor_004cf3a0;
  local_4 = 0;
  Matrix_Identity(param_1 + 0x321);
  local_4 = 0xffffffff;
  GameObject_dtor(param_1);
  ExceptionList = local_c;
  return;
}
