/*
 * Function: BoardLevel1_WarmUp_dtor
 * Address: 0x0041cb10
 * Signature: void __fastcall BoardLevel1_WarmUp_dtor(undefined4 *param_1)
 *
 * Patterns: scene, board, level. Calls: BoardLevel1_WarmUp_dtor, Scene_dtor. Offsets: 0, Lines: 6
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall BoardLevel1_WarmUp_dtor(undefined4 *param_1)

{
  *param_1 = &PTR_BoardLevel1_WarmUp_dtor_004d04a8;
  Scene_dtor(param_1);
  return;
}
