/*
 * Function: UI_DrawTextCentered
 * Address: 0x00409c60
 * Signature: void __thiscall
UI_DrawTextCentered(void *this,byte *param_1,int param_2,int param_3,int param_4,int param_5,
                   undefined4 param_6,undefined4 param_7,undefined4 param_8,undefined4 param_9,
                   undefined4 param_10,undefined4 param_11,undefined4 param_12,undefined4 param_13,
                   undefined4 param_14,undefined4 param_15)
 *
 * Patterns: SEH frame, matrix math, font/text, rendering. Calls: UI_DrawTextCentered, Font_MeasureText, UI_DrawTextShadow, Matrix_Identity. Offsets: 0, Lines: 45
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall
UI_DrawTextCentered(void *this,byte *param_1,int param_2,int param_3,int param_4,int param_5,
                   undefined4 param_6,undefined4 param_7,undefined4 param_8,undefined4 param_9,
                   undefined4 param_10,undefined4 param_11,undefined4 param_12,undefined4 param_13,
                   undefined4 param_14,undefined4 param_15)

{
  ulonglong uVar1;
  undefined **ppuVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined **ppuVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  void *local_c;
  undefined1 *puStack_8;
  uint local_4;
  
  puStack_8 = &LAB_004c9500;
  local_c = ExceptionList;
  ppuVar7 = &PTR_Vec3_dtor_004cf300;
  ppuVar2 = &PTR_Vec3_dtor_004cf300;
  local_4 = 1;
  ExceptionList = &local_c;
  uVar3 = param_7;
  uVar4 = param_8;
  uVar5 = param_9;
  uVar6 = param_10;
  uVar8 = param_12;
  uVar9 = param_13;
  uVar10 = param_14;
  uVar11 = param_15;
  uVar1 = Font_MeasureText((char *)param_1);
  UI_DrawTextShadow(this,param_1,param_2 - (int)uVar1,param_3,param_4,param_5,ppuVar2,uVar3,uVar4,
                    uVar5,uVar6,ppuVar7,uVar8,uVar9,uVar10,uVar11);
  local_4 = local_4 & 0xffffff00;
  Matrix_Identity(&param_6);
  local_4 = 0xffffffff;
  Matrix_Identity(&param_11);
  ExceptionList = local_c;
  return;
}
