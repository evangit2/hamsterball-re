
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall TourneyContinueDialog_Render(void *this,void *param_1)

{
  int iVar1;
  ulonglong uVar2;
  int iVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  undefined4 in_stack_ffffff8c;
  undefined **ppuVar14;
  undefined4 in_stack_ffffff90;
  undefined4 in_stack_ffffff94;
  undefined4 in_stack_ffffff98;
  undefined4 in_stack_ffffff9c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20 [5];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cc1a0;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Matrix_Scale4x4(&stack0xffffff8c,0,0,0,0x3f400000);
  Graphics_DrawScreenRect(param_1,0,0,800,600);
  Matrix_Scale4x4(&stack0xffffff8c,0,0,0x3f800000,0x3f000000);
  Graphics_DrawRectAndReset
            (param_1,this,in_stack_ffffff8c,in_stack_ffffff90,in_stack_ffffff94,in_stack_ffffff98,
             in_stack_ffffff9c);
  Matrix_Scale4x4(&stack0xffffff8c,0,0,0x3f800000,0x3f800000);
  UI_DrawRectAndReset(param_1);
  Matrix_Scale4x4(&stack0xffffff8c,0,0x3f800000,0x3f800000,0x3f000000);
  UI_DrawRectAndReset(param_1);
  uVar12 = 0x3f800000;
  uVar10 = 0;
  uVar8 = 0;
  uVar6 = 0;
  uVar5 = 0x4460a5;
  Matrix_Scale4x4(&stack0xffffff8c,0,0,0,0x3f800000);
  local_4 = 0;
  Matrix_Scale4x4(&stack0xffffff78,0x3f800000,0x3f800000,0,0x3f800000);
  iVar4 = 5;
  local_4 = 0xffffffff;
  iVar3 = 5;
  uVar2 = __ftol2();
  iVar1 = (int)uVar2;
  Font_MeasureText("CONTINUE TOURNAMENT?");
  uVar2 = __ftol2();
  UI_DrawTextShadow(*(void **)(*(int *)((int)this + 0x878) + 0x318),(byte *)"CONTINUE TOURNAMENT?",
                    (int)uVar2,iVar1,iVar3,iVar4,uVar5,uVar6,uVar8,uVar10,uVar12,in_stack_ffffff8c,
                    in_stack_ffffff90,in_stack_ffffff94,in_stack_ffffff98,in_stack_ffffff9c);
  uVar12 = 0x3f800000;
  uVar10 = 0;
  uVar8 = 0;
  uVar6 = 0;
  uVar5 = 0x44615e;
  Matrix_Scale4x4(&stack0xffffff8c,0,0,0,0x3f800000);
  local_4 = 1;
  Matrix_Scale4x4(&stack0xffffff78,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  iVar4 = 3;
  local_4 = 0xffffffff;
  iVar3 = 3;
  uVar2 = __ftol2();
  iVar1 = (int)uVar2;
  uVar2 = __ftol2();
  UI_DrawTextShadow(*(void **)(*(int *)((int)this + 0x878) + 0x318),(byte *)"WOULD YOU LIKE TO",
                    (int)uVar2,iVar1,iVar3,iVar4,uVar5,uVar6,uVar8,uVar10,uVar12,in_stack_ffffff8c,
                    in_stack_ffffff90,in_stack_ffffff94,in_stack_ffffff98,in_stack_ffffff9c);
  uVar12 = 0x3f800000;
  uVar10 = 0;
  uVar8 = 0;
  uVar6 = 0;
  uVar5 = 0x4461ff;
  Matrix_Scale4x4(&stack0xffffff8c,0,0,0,0x3f800000);
  local_4 = 2;
  Matrix_Scale4x4(&stack0xffffff78,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  iVar4 = 3;
  local_4 = 0xffffffff;
  iVar3 = 3;
  uVar2 = __ftol2();
  iVar1 = (int)uVar2;
  uVar2 = __ftol2();
  UI_DrawTextShadow(*(void **)(*(int *)((int)this + 0x878) + 0x318),
                    (byte *)"CONTINUE YOUR PREVIOUSLY",(int)uVar2,iVar1,iVar3,iVar4,uVar5,uVar6,
                    uVar8,uVar10,uVar12,in_stack_ffffff8c,in_stack_ffffff90,in_stack_ffffff94,
                    in_stack_ffffff98,in_stack_ffffff9c);
  uVar12 = 0x3f800000;
  uVar10 = 0;
  uVar8 = 0;
  uVar6 = 0;
  uVar5 = 0x4462a2;
  Matrix_Scale4x4(&stack0xffffff8c,0,0,0,0x3f800000);
  local_4 = 3;
  Matrix_Scale4x4(&stack0xffffff78,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  iVar4 = 3;
  local_4 = 0xffffffff;
  iVar3 = 3;
  uVar2 = __ftol2();
  iVar1 = (int)uVar2;
  uVar2 = __ftol2();
  UI_DrawTextShadow(*(void **)(*(int *)((int)this + 0x878) + 0x318),(byte *)"SAVED TOURNAMENT?",
                    (int)uVar2,iVar1,iVar3,iVar4,uVar5,uVar6,uVar8,uVar10,uVar12,in_stack_ffffff8c,
                    in_stack_ffffff90,in_stack_ffffff94,in_stack_ffffff98,in_stack_ffffff9c);
  Matrix_Scale4x4(&local_34,0,0,0x3f000000,0x3f400000);
  local_4 = 4;
  if ((*(int *)((int)this + 0x864) == (int)this + 0x87c) ||
     ((*(int *)((int)this + 0x8b4) == (int)this + 0x87c && (*(int *)((int)this + 0x864) == 0)))) {
    iVar1 = Matrix_Scale4x4(local_20,0x3f000000,0x3f000000,0x3f800000,0x3f800000);
    local_30 = *(undefined4 *)(iVar1 + 4);
    local_2c = *(undefined4 *)(iVar1 + 8);
    local_28 = *(undefined4 *)(iVar1 + 0xc);
    local_24 = *(undefined4 *)(iVar1 + 0x10);
    Matrix_Identity(local_20);
  }
  iVar4 = 0x37;
  uVar2 = Font_MeasureText((char *)&PTR_DAT_004cf870);
  iVar1 = (int)uVar2 + 0x1e;
  uVar2 = __ftol2();
  iVar3 = (int)uVar2;
  Font_MeasureText((char *)&PTR_DAT_004cf870);
  uVar2 = __ftol2();
  Graphics_DrawScreenRect(param_1,(int)uVar2,iVar3,iVar1,iVar4);
  Matrix_Scale4x4(&local_48,0,0,0x3f000000,0x3f400000);
  local_4._0_1_ = 5;
  if ((*(int *)((int)this + 0x864) == (int)this + 0x898) ||
     ((*(int *)((int)this + 0x8b4) == (int)this + 0x898 && (*(int *)((int)this + 0x864) == 0)))) {
    iVar1 = Matrix_Scale4x4(local_20,0x3f000000,0x3f000000,0x3f800000,0x3f800000);
    local_44 = *(undefined4 *)(iVar1 + 4);
    local_40 = *(undefined4 *)(iVar1 + 8);
    local_3c = *(undefined4 *)(iVar1 + 0xc);
    local_38 = *(undefined4 *)(iVar1 + 0x10);
    Matrix_Identity(local_20);
  }
  ppuVar14 = &PTR_Vec3_dtor_004cf300;
  iVar4 = 0x37;
  uVar5 = local_44;
  uVar6 = local_40;
  uVar8 = local_3c;
  uVar10 = local_38;
  uVar2 = Font_MeasureText((char *)&PTR_DAT_004cf870);
  iVar1 = (int)uVar2 + 0x1e;
  uVar2 = __ftol2();
  iVar3 = (int)uVar2;
  Font_MeasureText((char *)&PTR_DAT_004cf870);
  uVar2 = __ftol2();
  Graphics_DrawScreenRect(param_1,(int)uVar2,iVar3,iVar1,iVar4);
  uVar13 = 0x3f800000;
  uVar11 = 0;
  uVar9 = 0;
  uVar7 = 0;
  uVar12 = 0x44657d;
  Matrix_Scale4x4(&stack0xffffff8c,0,0,0,0x3f800000);
  local_4._0_1_ = 6;
  Matrix_Scale4x4(&stack0xffffff78,0x3f400000,0x3f800000,0x3f400000,0x3f800000);
  iVar4 = 5;
  local_4._0_1_ = 5;
  iVar3 = 5;
  uVar2 = __ftol2();
  iVar1 = (int)uVar2;
  Font_MeasureText((char *)&PTR_DAT_004cf870);
  uVar2 = __ftol2();
  UI_DrawTextShadow(*(void **)(*(int *)((int)this + 0x878) + 0x318),(byte *)&PTR_DAT_004cf870,
                    (int)uVar2,iVar1,iVar3,iVar4,uVar12,uVar7,uVar9,uVar11,uVar13,ppuVar14,uVar5,
                    uVar6,uVar8,uVar10);
  uVar13 = 0x3f800000;
  uVar11 = 0;
  uVar9 = 0;
  uVar7 = 0;
  uVar12 = 0x446635;
  Matrix_Scale4x4(&stack0xffffff8c,0,0,0,0x3f800000);
  local_4._0_1_ = 7;
  Matrix_Scale4x4(&stack0xffffff78,0x3f800000,0x3f400000,0x3f400000,0x3f800000);
  iVar4 = 5;
  local_4._0_1_ = 5;
  iVar3 = 5;
  uVar2 = __ftol2();
  iVar1 = (int)uVar2;
  Font_MeasureText("NO");
  uVar2 = __ftol2();
  UI_DrawTextShadow(*(void **)(*(int *)((int)this + 0x878) + 0x318),&DAT_004d48a0,(int)uVar2,iVar1,
                    iVar3,iVar4,uVar12,uVar7,uVar9,uVar11,uVar13,ppuVar14,uVar5,uVar6,uVar8,uVar10);
  local_4 = CONCAT31(local_4._1_3_,4);
  Matrix_Identity(&local_48);
  local_4 = 0xffffffff;
  Matrix_Identity(&local_34);
  ExceptionList = local_c;
  return;
}

