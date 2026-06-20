/*
 * Function: Sprite_DrawCentered
 * Address: 0x0040ae50
 * Signature: void __thiscall
Sprite_DrawCentered(void *this,float param_1,float param_2,undefined4 param_3,undefined4 param_4,
                   undefined4 param_5,undefined4 param_6,undefined4 param_7)
 *
 * Patterns: SEH frame, matrix math, rendering. Calls: Sprite_DrawCentered, Sprite_DrawRect, Matrix_Identity. Offsets: 1, Lines: 19
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall
Sprite_DrawCentered(void *this,float param_1,float param_2,undefined4 param_3,undefined4 param_4,
                   undefined4 param_5,undefined4 param_6,undefined4 param_7)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004c9578;
  local_c = ExceptionList;
  local_4 = 0;
  ExceptionList = &local_c;
  Sprite_DrawRect(this,param_1 - (float)(*(int *)((int)this + 200) / 2),
                  param_2 - (float)(*(int *)((int)this + 0xcc) / 2));
  local_4 = 0xffffffff;
  Matrix_Identity(&param_3);
  ExceptionList = local_c;
  return;
}
