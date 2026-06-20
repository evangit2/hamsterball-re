/*
 * Function: BoardLevel2_Intermediate_dtor
 * Address: 0x0041cc80
 * Signature: void __fastcall BoardLevel2_Intermediate_dtor(undefined4 *param_1)
 *
 * Patterns: scene, board, level. Calls: BoardLevel2_Intermediate_dtor, Scene_dtor. Offsets: 0, Lines: 6
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall BoardLevel2_Intermediate_dtor(undefined4 *param_1)

{
  *param_1 = &PTR_BoardLevel2_Intermediate_dtor_004d05a0;
  Scene_dtor(param_1);
  return;
}
