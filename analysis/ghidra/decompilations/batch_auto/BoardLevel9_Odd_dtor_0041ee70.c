/*
 * Function: LevelBoard_Odd_dtor
 * Address: 0x0041ee70
 * Signature: void __fastcall LevelBoard_Odd_dtor(undefined4 *param_1)
 *
 * Patterns: scene, board, level. Calls: LevelBoard_Odd_dtor, Scene_dtor. Offsets: 0, Lines: 6
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall LevelBoard_Odd_dtor(undefined4 *param_1)

{
  *param_1 = &PTR_LevelBoard_Odd_dtor_004d0bc0;
  Scene_dtor(param_1);
  return;
}
