/*
 * Function: LevelBoard_Intermediate_dtor
 * Address: 0x0041cc80
 * Signature: void __fastcall LevelBoard_Intermediate_dtor(undefined4 *param_1)
 *
 * Patterns: scene, board, level. Calls: LevelBoard_Intermediate_dtor, Scene_dtor. Offsets: 0, Lines: 6
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall LevelBoard_Intermediate_dtor(undefined4 *param_1)

{
  *param_1 = &PTR_LevelBoard_Intermediate_dtor_004d05a0;
  Scene_dtor(param_1);
  return;
}
