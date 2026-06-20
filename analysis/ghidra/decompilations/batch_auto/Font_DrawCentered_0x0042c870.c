
void __thiscall
Font_DrawCentered(void *this,byte *param_1,int param_2,int param_3,undefined4 param_4,
                 undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8)

{
  ulonglong uVar1;
  undefined **ppuVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cb0b8;
  local_c = ExceptionList;
  ppuVar2 = &PTR_Vec3_dtor_004cf300;
  local_4 = 0;
  ExceptionList = &local_c;
  uVar3 = param_5;
  uVar4 = param_6;
  uVar5 = param_7;
  uVar6 = param_8;
  uVar1 = Font_MeasureText((char *)param_1);
  Font_DrawGlyph(this,param_1,param_2 - (int)uVar1 / 2,param_3,ppuVar2,uVar3,uVar4,uVar5,uVar6);
  local_4 = 0xffffffff;
  Matrix_Identity(&param_4);
  ExceptionList = local_c;
  return;
}

