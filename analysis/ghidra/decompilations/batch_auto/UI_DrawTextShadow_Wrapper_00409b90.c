/*
 * Function: UI_DrawTextShadow_Wrapper
 * Address: 0x00409b90
 * Signature: void __thiscall
UI_DrawTextShadow_Wrapper
          (void *this,byte *param_1,int param_2,int param_3,int param_4,int param_5,
          undefined4 param_6,undefined4 param_7,undefined4 param_8,undefined4 param_9,
          undefined4 param_10,undefined4 param_11,undefined4 param_12,undefined4 param_13,
          undefined4 param_14,undefined4 param_15)
 *
 * Patterns: SEH frame, matrix math, rendering. Calls: UI_DrawTextShadow_Wrapper, UI_DrawTextShadow, Matrix_Identity. Offsets: 0, Lines: 25
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall
UI_DrawTextShadow_Wrapper
          (void *this,byte *param_1,int param_2,int param_3,int param_4,int param_5,
          undefined4 param_6,undefined4 param_7,undefined4 param_8,undefined4 param_9,
          undefined4 param_10,undefined4 param_11,undefined4 param_12,undefined4 param_13,
          undefined4 param_14,undefined4 param_15)

{
  void *local_c;
  undefined1 *puStack_8;
  uint local_4;
  
  puStack_8 = &LAB_004c9500;
  local_c = ExceptionList;
  local_4 = 1;
  ExceptionList = &local_c;
  UI_DrawTextShadow(this,param_1,param_2,param_3,param_4,param_5,&PTR_Vec3_dtor_004cf300,param_7,
                    param_8,param_9,param_10,&PTR_Vec3_dtor_004cf300,param_12,param_13,param_14,
                    param_15);
  local_4 = local_4 & 0xffffff00;
  Matrix_Identity(&param_6);
  local_4 = 0xffffffff;
  Matrix_Identity(&param_11);
  ExceptionList = local_c;
  return;
}
