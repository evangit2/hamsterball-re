/*
 * Function: Scene_CreateObject4f
 * Address: 0x00418870
 * Signature: void __thiscall
Scene_CreateObject4f
          (void *this,undefined4 param_1,float param_2,float param_3,float param_4,float param_5,
          undefined4 param_6,undefined4 param_7,undefined4 param_8,undefined4 param_9,
          undefined4 param_10)
 *
 * Patterns: SEH frame, matrix math, rendering, ball, scene. Calls: Scene_CreateObject4f, Ball_GetTransform, Gfx_DrawQuadRandomColor, Matrix_Identity. Offsets: 0, Lines: 28
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall
Scene_CreateObject4f
          (void *this,undefined4 param_1,float param_2,float param_3,float param_4,float param_5,
          undefined4 param_6,undefined4 param_7,undefined4 param_8,undefined4 param_9,
          undefined4 param_10)

{
  undefined4 in_stack_ffffffc4;
  undefined4 in_stack_ffffffc8;
  undefined4 in_stack_ffffffcc;
  undefined4 in_stack_ffffffd0;
  undefined4 in_stack_ffffffd4;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004c94e0;
  local_c = ExceptionList;
  local_4 = 1;
  ExceptionList = &local_c;
  Ball_GetTransform(this,(undefined4 *)&stack0xffffffc4);
  Gfx_DrawQuadRandomColor
            (this,&PTR_LAB_004cf584,param_2,param_3,param_4,param_5,in_stack_ffffffc4,
             in_stack_ffffffc8,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4);
  local_4 = 0xffffffff;
  Matrix_Identity(&param_6);
  ExceptionList = local_c;
  return;
}
