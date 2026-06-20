/*
 * Function: SceneObject_InitAtPosition
 * Address: 0x0040af00
 * Signature: void __thiscall
SceneObject_InitAtPosition
          (void *this,int param_1,int param_2,undefined4 param_3,undefined4 param_4,
          undefined4 param_5,undefined4 param_6,undefined4 param_7)
 *
 * Patterns: SEH frame, matrix math, rendering, scene. Calls: SceneObject_InitAtPosition, Sprite_DrawCentered, Matrix_Identity. Offsets: 0, Lines: 20
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall
SceneObject_InitAtPosition
          (void *this,int param_1,int param_2,undefined4 param_3,undefined4 param_4,
          undefined4 param_5,undefined4 param_6,undefined4 param_7)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004c9578;
  local_c = ExceptionList;
  local_4 = 0;
  ExceptionList = &local_c;
  Sprite_DrawCentered(this,(float)param_1,(float)param_2,&PTR_Vec3_dtor_004cf300,param_4,param_5,
                      param_6,param_7);
  local_4 = 0xffffffff;
  Matrix_Identity(&param_3);
  ExceptionList = local_c;
  return;
}
