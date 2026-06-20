
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall HighScoreEntry_Render(void *this,void *param_1)

{
  byte *pbVar1;
  void *this_00;
  ulonglong uVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  undefined4 uVar7;
  undefined **ppuVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  undefined4 uVar14;
  undefined4 uVar15;
  undefined4 uVar16;
  undefined4 uVar17;
  undefined4 in_stack_ffffff70;
  undefined4 in_stack_ffffff74;
  undefined4 in_stack_ffffff78;
  undefined4 in_stack_ffffff7c;
  undefined4 in_stack_ffffff80;
  char *pcVar18;
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
  puStack_8 = &LAB_004cb0a0;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Matrix_Scale4x4(&stack0xffffff70,0,0,0x3f800000,*(undefined4 *)((int)this + 0x880));
  Graphics_DrawPoint(param_1);
  Matrix_Scale4x4(&stack0xffffff70,0,0,0x3f800000,*(undefined4 *)((int)this + 0x880));
  Graphics_DrawPoint(param_1);
  Matrix_Scale4x4(&stack0xffffff70,0x3f800000,0x3f800000,0,0x3f800000);
  UI_DrawRectAndReset(param_1);
  uVar16 = 0x3f800000;
  uVar14 = 0;
  uVar12 = 0;
  uVar10 = 0;
  uVar7 = 0x42be9b;
  Matrix_Scale4x4(&stack0xffffff70,0,0,0,0x3f800000);
  local_4 = 0;
  Matrix_Scale4x4(&stack0xffffff5c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  iVar5 = 3;
  iVar4 = 3;
  local_4 = 0xffffffff;
  iVar3 = 0xc3;
  uVar2 = __ftol2();
  UI_DrawTextCenteredAbsolute
            (*(void **)(*(int *)((int)this + 0x878) + 0x318),(byte *)"A NEW HIGH SCORE!!",(int)uVar2
             ,iVar3,iVar4,iVar5,uVar7,uVar10,uVar12,uVar14,uVar16,in_stack_ffffff70,
             in_stack_ffffff74,in_stack_ffffff78,in_stack_ffffff7c,in_stack_ffffff80);
  if (_DAT_004cf3f0 <= *(float *)((int)this + 0x880)) {
    uVar16 = 0x3f800000;
    uVar14 = 0x3f800000;
    uVar12 = 0x3f800000;
    uVar10 = 0x3f800000;
    *(undefined4 *)(*(int *)(*(int *)((int)this + 0x878) + 0x318) + 0x428) = 0x3fa00000;
    uVar7 = 0x42bf50;
    Matrix_Scale4x4(&local_34,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    local_4 = 1;
    if (*(char *)((int)this + 0x88c) != '\0') {
      uVar14 = 0x3f800000;
      uVar12 = 0x3f400000;
      uVar10 = 0x3f400000;
      uVar7 = 0x42bf7f;
      iVar3 = Matrix_Scale4x4(local_20,0x3f400000,0x3f400000,0x3f800000,0x3f800000);
      local_30 = *(undefined4 *)(iVar3 + 4);
      local_2c = *(undefined4 *)(iVar3 + 8);
      local_28 = *(undefined4 *)(iVar3 + 0xc);
      local_24 = *(undefined4 *)(iVar3 + 0x10);
      uVar16 = 0x42bfa4;
      Matrix_Identity(local_20);
    }
    Matrix_Scale4x4(&stack0xffffff70,0,0,0,0x3f800000);
    ppuVar8 = &PTR_Vec3_dtor_004cf300;
    iVar6 = 3;
    iVar5 = 3;
    iVar4 = 0xf5;
    iVar3 = 400;
    uVar9 = local_30;
    uVar11 = local_2c;
    uVar13 = local_28;
    uVar15 = local_24;
    __ftol2();
    pbVar1 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d0408);
    UI_DrawTextCenteredAbsolute
              (*(void **)(*(int *)((int)this + 0x878) + 0x318),pbVar1,iVar3,iVar4,iVar5,iVar6,
               ppuVar8,uVar9,uVar11,uVar13,uVar15,uVar7,uVar10,uVar12,uVar14,uVar16);
    uVar17 = 0x3f800000;
    uVar15 = 0;
    uVar13 = 0;
    uVar11 = 0;
    *(undefined4 *)(*(int *)(*(int *)((int)this + 0x878) + 0x318) + 0x428) = 0x3f800000;
    uVar9 = 0x42c064;
    Matrix_Scale4x4(&stack0xffffff70,0,0,0,0x3f800000);
    local_4._0_1_ = 2;
    Matrix_Scale4x4(&stack0xffffff5c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    local_4._0_1_ = 1;
    UI_DrawTextCenteredAbsolute
              (*(void **)(*(int *)((int)this + 0x878) + 0x318),(byte *)"ENTER YOUR NAME!",400,0x140,
               3,3,uVar9,uVar11,uVar13,uVar15,uVar17,uVar7,uVar10,uVar12,uVar14,uVar16);
    Matrix_Scale4x4(&stack0xffffff70,0x3f800000,0x3f800000,0x3f800000,0x3f000000);
    UI_DrawRectAndReset(param_1);
    pcVar18 = (char *)((int)this + 0x8b0);
    uVar13 = 0x42c12c;
    uVar2 = Font_MeasureText(pcVar18);
    uVar11 = 0x3f800000;
    uVar9 = 0;
    uVar16 = 0;
    uVar14 = 0;
    if (*(char *)((int)this + 0x8a0) == '\0') {
      uVar15 = 0x42c1c5;
      Matrix_Scale4x4(&stack0xffffff70,0,0,0,0x3f800000);
      local_4._0_1_ = 4;
      Matrix_Scale4x4(&stack0xffffff5c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
      iVar6 = 3;
      iVar5 = 3;
      iVar4 = 0x18b;
      iVar3 = 400 - (int)uVar2 / 2;
      local_4 = CONCAT31(local_4._1_3_,1);
      pbVar1 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d3ad8);
      this_00 = *(void **)(*(int *)((int)this + 0x878) + 0x318);
    }
    else {
      uVar15 = 0x42c154;
      Matrix_Scale4x4(&stack0xffffff70,0,0,0,0x3f800000);
      local_4._0_1_ = 3;
      Matrix_Scale4x4(&stack0xffffff5c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
      iVar6 = 3;
      iVar5 = 3;
      iVar4 = 0x18b;
      iVar3 = 400 - (int)uVar2 / 2;
      local_4 = CONCAT31(local_4._1_3_,1);
      pbVar1 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d3adc);
      this_00 = *(void **)(*(int *)((int)this + 0x878) + 0x318);
    }
    UI_DrawTextShadow(this_00,pbVar1,iVar3,iVar4,iVar5,iVar6,uVar15,uVar14,uVar16,uVar9,uVar11,uVar7
                      ,uVar10,uVar12,uVar13,pcVar18);
    uVar16 = 0x3f800000;
    uVar14 = 0x3f800000;
    uVar12 = 0x3f800000;
    uVar10 = 0x3f800000;
    uVar7 = 0x42c251;
    Matrix_Scale4x4(&local_48,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    local_4 = CONCAT31(local_4._1_3_,5);
    if (*(int *)((int)this + 0x864) == (int)this + 0x8cc) {
      Matrix_Scale4x4(&stack0xffffff70,0x3f000000,0x3f000000,0x3f800000,0x3f800000);
      UI_DrawRectAndReset(param_1);
      uVar14 = 0;
      uVar12 = 0x3f800000;
      uVar10 = 0x3f800000;
      uVar7 = 0x42c2d9;
      iVar3 = Matrix_Scale4x4(local_20,0x3f800000,0x3f800000,0,0x3f800000);
      local_44 = *(undefined4 *)(iVar3 + 4);
      local_40 = *(undefined4 *)(iVar3 + 8);
      local_3c = *(undefined4 *)(iVar3 + 0xc);
      local_38 = *(undefined4 *)(iVar3 + 0x10);
      uVar16 = 0x42c2fe;
      Matrix_Identity(local_20);
    }
    else {
      Matrix_Scale4x4(&stack0xffffff70,0x3f000000,0x3f000000,0x3f800000,0x3f800000);
      UI_DrawRectAndReset(param_1);
    }
    Matrix_Scale4x4(&stack0xffffff70,0,0,0,0x3f800000);
    uVar2 = __ftol2();
    iVar3 = (int)uVar2;
    iVar5 = 10;
    uVar2 = __ftol2();
    iVar4 = (int)uVar2;
    uVar2 = __ftol2();
    Graphics_DrawScreenRect(param_1,(int)uVar2,iVar4,iVar5,iVar3);
    Matrix_Scale4x4(&stack0xffffff70,0,0,0,0x3f800000);
    iVar5 = 10;
    uVar2 = __ftol2();
    iVar3 = (int)uVar2;
    uVar2 = __ftol2();
    iVar4 = (int)uVar2;
    uVar2 = __ftol2();
    Graphics_DrawScreenRect(param_1,(int)uVar2,iVar4,iVar3,iVar5);
    Matrix_Scale4x4(&stack0xffffff70,0,0,0,0x3f800000);
    ppuVar8 = &PTR_Vec3_dtor_004cf300;
    iVar5 = 2;
    iVar4 = 2;
    uVar9 = local_44;
    uVar11 = local_40;
    uVar13 = local_3c;
    uVar15 = local_38;
    uVar2 = __ftol2();
    iVar3 = (int)uVar2;
    uVar2 = __ftol2();
    UI_DrawTextCenteredAbsolute
              (*(void **)(*(int *)((int)this + 0x878) + 0x318),&DAT_004d3ad0,(int)uVar2,iVar3,iVar4,
               iVar5,ppuVar8,uVar9,uVar11,uVar13,uVar15,uVar7,uVar10,uVar12,uVar14,uVar16);
    uVar14 = 0x3f800000;
    uVar12 = 0x3f800000;
    uVar10 = 0x3f800000;
    uVar7 = 0x42c485;
    iVar3 = Matrix_Scale4x4(local_20,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    local_44 = *(undefined4 *)(iVar3 + 4);
    local_40 = *(undefined4 *)(iVar3 + 8);
    local_3c = *(undefined4 *)(iVar3 + 0xc);
    local_38 = *(undefined4 *)(iVar3 + 0x10);
    uVar16 = 0x42c4aa;
    Matrix_Identity(local_20);
    if (*(int *)((int)this + 0x864) == (int)this + 0x8e8) {
      Matrix_Scale4x4(&stack0xffffff70,0x3f000000,0x3f000000,0x3f800000,0x3f800000);
      UI_DrawRectAndReset(param_1);
      uVar14 = 0;
      uVar12 = 0x3f800000;
      uVar10 = 0x3f800000;
      uVar7 = 0x42c52a;
      iVar3 = Matrix_Scale4x4(local_20,0x3f800000,0x3f800000,0,0x3f800000);
      local_44 = *(undefined4 *)(iVar3 + 4);
      local_40 = *(undefined4 *)(iVar3 + 8);
      local_3c = *(undefined4 *)(iVar3 + 0xc);
      local_38 = *(undefined4 *)(iVar3 + 0x10);
      uVar16 = 0x42c54f;
      Matrix_Identity(local_20);
    }
    else {
      Matrix_Scale4x4(&stack0xffffff70,0x3f000000,0x3f000000,0x3f800000,0x3f800000);
      UI_DrawRectAndReset(param_1);
    }
    Matrix_Scale4x4(&stack0xffffff70,0,0,0,0x3f800000);
    uVar2 = __ftol2();
    iVar3 = (int)uVar2;
    iVar5 = 10;
    uVar2 = __ftol2();
    iVar4 = (int)uVar2;
    uVar2 = __ftol2();
    Graphics_DrawScreenRect(param_1,(int)uVar2,iVar4,iVar5,iVar3);
    Matrix_Scale4x4(&stack0xffffff70,0,0,0,0x3f800000);
    iVar5 = 10;
    uVar2 = __ftol2();
    iVar3 = (int)uVar2;
    uVar2 = __ftol2();
    iVar4 = (int)uVar2;
    uVar2 = __ftol2();
    Graphics_DrawScreenRect(param_1,(int)uVar2,iVar4,iVar3,iVar5);
    Matrix_Scale4x4(&stack0xffffff70,0,0,0,0x3f800000);
    ppuVar8 = &PTR_Vec3_dtor_004cf300;
    iVar5 = 2;
    iVar4 = 2;
    uVar9 = local_44;
    uVar11 = local_40;
    uVar13 = local_3c;
    uVar15 = local_38;
    uVar2 = __ftol2();
    iVar3 = (int)uVar2;
    uVar2 = __ftol2();
    UI_DrawTextCenteredAbsolute
              (*(void **)(*(int *)((int)this + 0x878) + 0x318),(byte *)0x4d3ac8,(int)uVar2,iVar3,
               iVar4,iVar5,ppuVar8,uVar9,uVar11,uVar13,uVar15,uVar7,uVar10,uVar12,uVar14,uVar16);
    local_4 = CONCAT31(local_4._1_3_,1);
    Matrix_Identity(&local_48);
    local_4 = 0xffffffff;
    Matrix_Identity(&local_34);
  }
  ExceptionList = local_c;
  return;
}

