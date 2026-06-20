
void __thiscall ConfirmMenu_Render(void *this,void *param_1)

{
  byte *pbVar1;
  char *pcVar2;
  ulonglong uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  undefined4 uVar7;
  undefined **ppuVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  int iVar13;
  undefined4 uVar14;
  undefined4 uVar15;
  undefined4 uVar16;
  undefined4 in_stack_ffffff8c;
  undefined4 in_stack_ffffff90;
  undefined **in_stack_ffffff94;
  undefined4 in_stack_ffffff98;
  undefined4 uVar17;
  undefined4 in_stack_ffffff9c;
  undefined4 uVar18;
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
  puStack_8 = &LAB_004cc740;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Matrix_Scale4x4(&stack0xffffff8c,0x3f800000,0x3f800000,0x3f800000,
                  *(undefined4 *)((int)this + 0x1c));
  Gfx_DrawQuadRandomColor
            (*(void **)(*(int *)((int)this + 0xc) + 0x354),&PTR_LAB_004cf584,40.0,25.0,702.0,569.0,
             &PTR_LAB_004cf584,0x3f800000,0,0,0x3f800000);
  Matrix_Scale4x4(&stack0xffffff8c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  iVar13 = 0x120;
  uVar3 = __ftol2();
  SceneObject_InitAtPosition
            (*(void **)((int)this + 0x90),(int)uVar3,iVar13,in_stack_ffffff8c,in_stack_ffffff90,
             in_stack_ffffff94,in_stack_ffffff98,in_stack_ffffff9c);
  Matrix_Scale4x4(&stack0xffffff8c,0x3f800000,0x3f800000,0,0x3f800000);
  UI_DrawRectAndReset(param_1);
  uVar14 = 0x3f800000;
  uVar11 = 0;
  uVar10 = 0;
  uVar9 = 0;
  uVar7 = 0x44ce26;
  Matrix_Scale4x4(&stack0xffffff8c,0,0,0,0x3f800000);
  local_4 = 0;
  Matrix_Scale4x4(&stack0xffffff78,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  local_4 = 0xffffffff;
  UI_DrawTextCenteredAbsolute
            (*(void **)(*(int *)((int)this + 0xc) + 0x318),*(byte **)((int)this + 0x24),
             (200 - *(int *)((int)this + 0x20)) * 2,0x23,3,3,uVar7,uVar9,uVar10,uVar11,uVar14,
             in_stack_ffffff8c,in_stack_ffffff90,in_stack_ffffff94,in_stack_ffffff98,
             in_stack_ffffff9c);
  if (*(int *)((int)this + 0x28) < *(int *)((int)this + 0x10)) {
    uVar14 = 0x3f800000;
    uVar11 = 0;
    uVar10 = 0;
    uVar9 = 0;
    uVar7 = 0x44ce8c;
    Matrix_Scale4x4(&stack0xffffff8c,0,0,0,0x3f800000);
    local_4 = 1;
    Matrix_Scale4x4(&stack0xffffff78,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    local_4 = 0xffffffff;
    UI_DrawTextCentered(*(void **)(*(int *)((int)this + 0xc) + 0x318),(byte *)"TIME REMAINING:",
                        0x1ea,0x84,3,3,uVar7,uVar9,uVar10,uVar11,uVar14,in_stack_ffffff8c,
                        in_stack_ffffff90,in_stack_ffffff94,in_stack_ffffff98,in_stack_ffffff9c);
    if (0 < *(int *)((int)this + 0x5c)) {
      uVar14 = 0x3f800000;
      uVar11 = 0;
      uVar10 = 0;
      uVar9 = 0;
      uVar7 = 0x44ceec;
      Matrix_Scale4x4(&stack0xffffff8c,0,0,0,0x3f800000);
      local_4 = 2;
      Matrix_Scale4x4(&stack0xffffff78,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
      iVar6 = 3;
      iVar5 = 3;
      iVar4 = 0x84;
      iVar13 = 0x226;
      local_4 = 0xffffffff;
      pbVar1 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
      UI_DrawTextCentered(*(void **)(*(int *)((int)this + 0xc) + 0x318),pbVar1,iVar13,iVar4,iVar5,
                          iVar6,uVar7,uVar9,uVar10,uVar11,uVar14,in_stack_ffffff8c,in_stack_ffffff90
                          ,in_stack_ffffff94,in_stack_ffffff98,in_stack_ffffff9c);
      uVar14 = 0x3f800000;
      uVar11 = 0;
      uVar10 = 0;
      uVar9 = 0;
      uVar7 = 0x44cf66;
      Matrix_Scale4x4(&stack0xffffff8c,0,0,0,0x3f800000);
      local_4 = 3;
      Matrix_Scale4x4(&stack0xffffff78,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
      iVar6 = 3;
      iVar5 = 3;
      iVar4 = 0x98;
      iVar13 = 0x226;
      local_4 = 0xffffffff;
      pbVar1 = (byte *)AthenaString_Format(0x4f7448,(byte *)".%.1d");
      UI_DrawTextShadow_Wrapper
                (*(void **)(*(int *)((int)this + 0xc) + 0x31c),pbVar1,iVar13,iVar4,iVar5,iVar6,uVar7
                 ,uVar9,uVar10,uVar11,uVar14,in_stack_ffffff8c,in_stack_ffffff90,in_stack_ffffff94,
                 in_stack_ffffff98,in_stack_ffffff9c);
    }
  }
  if (*(int *)((int)this + 0x30) < *(int *)((int)this + 0x10)) {
    uVar14 = 0x3f800000;
    uVar11 = 0;
    uVar10 = 0;
    uVar9 = 0;
    uVar7 = 0x44cff6;
    Matrix_Scale4x4(&stack0xffffff8c,0,0,0,0x3f800000);
    local_4 = 4;
    Matrix_Scale4x4(&stack0xffffff78,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    local_4 = 0xffffffff;
    UI_DrawTextCentered(*(void **)(*(int *)((int)this + 0xc) + 0x318),(byte *)"PAR FOR THE COURSE:",
                        0x1ea,0xa8,3,3,uVar7,uVar9,uVar10,uVar11,uVar14,in_stack_ffffff8c,
                        in_stack_ffffff90,in_stack_ffffff94,in_stack_ffffff98,in_stack_ffffff9c);
    if (0 < *(int *)((int)this + 0x60)) {
      uVar7 = 0x3f800000;
      in_stack_ffffff98 = 0x3f800000;
      in_stack_ffffff94 = (undefined **)0x3f800000;
      in_stack_ffffff90 = 0x3f800000;
      in_stack_ffffff8c = 0x44d051;
      Matrix_Scale4x4(&local_48,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
      local_4 = 5;
      if (*(int *)((int)this + 0x38) < *(int *)((int)this + 0x10)) {
        in_stack_ffffff98 = 0x3f800000;
        in_stack_ffffff94 = (undefined **)0x3f800000;
        in_stack_ffffff90 = 0x3f800000;
        in_stack_ffffff8c = 0x44d06e;
        iVar13 = Matrix_Scale4x4(local_20,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
        local_44 = *(undefined4 *)(iVar13 + 4);
        local_40 = *(undefined4 *)(iVar13 + 8);
        local_3c = *(undefined4 *)(iVar13 + 0xc);
        local_38 = *(undefined4 *)(iVar13 + 0x10);
        uVar7 = 0x44d093;
        Matrix_Identity(local_20);
      }
      if ((*(int *)((int)this + 0x38) < *(int *)((int)this + 0x10)) &&
         (*(int *)((int)this + 0x60) <= *(int *)((int)this + 0x5c))) {
        if (*(int *)((int)this + 0x10) < *(int *)((int)this + 0x48)) {
          uVar7 = 0;
          in_stack_ffffff90 = 0;
        }
        else {
          uVar7 = 0x3f800000;
          in_stack_ffffff90 = 0x3f800000;
        }
        in_stack_ffffff94 = (undefined **)0x3f800000;
        in_stack_ffffff8c = 0x44d0ba;
        iVar13 = Matrix_Scale4x4(local_20,in_stack_ffffff90,0x3f800000,uVar7,0x3f800000);
        in_stack_ffffff98 = 0x44d0c4;
        Vec3_Copy(&local_48,iVar13);
        uVar7 = 0x44d0cd;
        Matrix_Identity(local_20);
      }
      Matrix_Scale4x4(&stack0xffffff8c,0,0,0,0x3f800000);
      ppuVar8 = &PTR_Vec3_dtor_004cf300;
      iVar6 = 3;
      iVar5 = 3;
      iVar4 = 0xa8;
      iVar13 = 0x226;
      uVar9 = local_44;
      uVar10 = local_40;
      uVar11 = local_3c;
      uVar14 = local_38;
      pbVar1 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
      UI_DrawTextCentered(*(void **)(*(int *)((int)this + 0xc) + 0x318),pbVar1,iVar13,iVar4,iVar5,
                          iVar6,ppuVar8,uVar9,uVar10,uVar11,uVar14,in_stack_ffffff8c,
                          in_stack_ffffff90,in_stack_ffffff94,in_stack_ffffff98,uVar7);
      Matrix_Scale4x4(&stack0xffffff8c,0,0,0,0x3f800000);
      ppuVar8 = &PTR_Vec3_dtor_004cf300;
      iVar6 = 3;
      iVar5 = 3;
      iVar4 = 0xbc;
      iVar13 = 0x226;
      uVar9 = local_44;
      uVar10 = local_40;
      uVar11 = local_3c;
      uVar14 = local_38;
      pbVar1 = (byte *)AthenaString_Format(0x4f7448,(byte *)".%.1d");
      UI_DrawTextShadow_Wrapper
                (*(void **)(*(int *)((int)this + 0xc) + 0x31c),pbVar1,iVar13,iVar4,iVar5,iVar6,
                 ppuVar8,uVar9,uVar10,uVar11,uVar14,in_stack_ffffff8c,in_stack_ffffff90,
                 in_stack_ffffff94,in_stack_ffffff98,uVar7);
      if ((*(int *)((int)this + 0x48) < *(int *)((int)this + 0x10)) &&
         (*(int *)((int)this + 0x60) <= *(int *)((int)this + 0x5c))) {
        uVar15 = 0x3f800000;
        uVar14 = 0;
        uVar11 = 0;
        uVar10 = 0;
        uVar9 = 0x44d1fb;
        Matrix_Scale4x4(&stack0xffffff8c,0,0,0,0x3f800000);
        local_4._0_1_ = 6;
        Matrix_Scale4x4(&stack0xffffff78,0,0x3f800000,0,0x3f800000);
        iVar6 = 3;
        iVar5 = 3;
        iVar4 = 0xb9;
        iVar13 = 0x23f;
        local_4 = CONCAT31(local_4._1_3_,5);
        Difficulty_GetTimeModifier(*(void **)((int)this + 0xc),1000.0);
        __ftol2();
        pbVar1 = (byte *)AthenaString_Format(0x4f7448,(byte *)"Bonus +%d!");
        UI_DrawTextShadow_Wrapper
                  (*(void **)(*(int *)((int)this + 0xc) + 0x31c),pbVar1,iVar13,iVar4,iVar5,iVar6,
                   uVar9,uVar10,uVar11,uVar14,uVar15,in_stack_ffffff8c,in_stack_ffffff90,
                   in_stack_ffffff94,in_stack_ffffff98,uVar7);
      }
      local_4 = 0xffffffff;
      in_stack_ffffff9c = 0x44d26c;
      Matrix_Identity(&local_48);
    }
  }
  if (*(int *)((int)this + 0x40) < *(int *)((int)this + 0x10)) {
    uVar14 = 0x3f800000;
    uVar11 = 0;
    uVar10 = 0;
    uVar9 = 0;
    uVar7 = 0x44d28a;
    Matrix_Scale4x4(&stack0xffffff8c,0,0,0,0x3f800000);
    local_4 = 7;
    Matrix_Scale4x4(&stack0xffffff78,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    local_4 = 0xffffffff;
    UI_DrawTextCentered(*(void **)(*(int *)((int)this + 0xc) + 0x318),(byte *)"BONUS:",0x1ea,0xfe,3,
                        3,uVar7,uVar9,uVar10,uVar11,uVar14,in_stack_ffffff8c,in_stack_ffffff90,
                        in_stack_ffffff94,in_stack_ffffff98,in_stack_ffffff9c);
    if (*(int *)((int)this + 0x68) != 0) {
      uVar18 = 0x3f800000;
      uVar17 = 0x3f800000;
      uVar15 = 0x3f800000;
      uVar14 = 0x3f800000;
      in_stack_ffffff8c = 0x44d2e5;
      Matrix_Scale4x4(&local_48,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
      local_4 = 8;
      Matrix_Scale4x4(&stack0xffffff8c,0,0,0,0x3f800000);
      ppuVar8 = &PTR_Vec3_dtor_004cf300;
      iVar6 = 3;
      iVar5 = 3;
      iVar4 = 0xfe;
      iVar13 = 0x203;
      uVar7 = local_44;
      uVar9 = local_40;
      uVar10 = local_3c;
      uVar11 = local_38;
      pbVar1 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
      UI_DrawTextShadow_Wrapper
                (*(void **)(*(int *)((int)this + 0xc) + 0x318),pbVar1,iVar13,iVar4,iVar5,iVar6,
                 ppuVar8,uVar7,uVar9,uVar10,uVar11,in_stack_ffffff8c,uVar14,uVar15,uVar17,uVar18);
      pcVar2 = (char *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
      Font_MeasureText(pcVar2);
      in_stack_ffffff94 = &PTR_PTR_004f7448;
      in_stack_ffffff90 = 0x44d398;
      pcVar2 = (char *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
      in_stack_ffffff98 = 0x44d3aa;
      Font_MeasureText(pcVar2);
      local_4 = 0xffffffff;
      in_stack_ffffff9c = 0x44d3b7;
      Matrix_Identity(&local_48);
    }
  }
  uVar14 = 0x3f800000;
  uVar11 = 0;
  uVar10 = 0;
  uVar9 = 0;
  uVar7 = 0x44d3c9;
  Matrix_Scale4x4(&stack0xffffff8c,0,0,0,0x3f800000);
  local_4 = 9;
  Matrix_Scale4x4(&stack0xffffff78,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  local_4 = 0xffffffff;
  UI_DrawTextCentered(*(void **)(*(int *)((int)this + 0xc) + 0x318),(byte *)"YOUR RANK:",
                      0x1cc - *(int *)((int)this + 0x70),0x154,3,3,uVar7,uVar9,uVar10,uVar11,uVar14,
                      in_stack_ffffff8c,in_stack_ffffff90,in_stack_ffffff94,in_stack_ffffff98,
                      in_stack_ffffff9c);
  uVar14 = 0x3f800000;
  uVar11 = 0x3f800000;
  uVar10 = 0x3f800000;
  uVar9 = 0x3f800000;
  uVar7 = 0x44d421;
  Matrix_Scale4x4(&local_34,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  local_4 = 10;
  if (*(char *)((int)this + 0x78) != '\0') {
    uVar11 = 0x3f4ccccd;
    uVar10 = 0x3f800000;
    uVar9 = 0x3f4ccccd;
    uVar7 = 0x44d443;
    iVar13 = Matrix_Scale4x4(local_20,0x3f4ccccd,0x3f800000,0x3f4ccccd,0x3f800000);
    local_30 = *(undefined4 *)(iVar13 + 4);
    local_2c = *(undefined4 *)(iVar13 + 8);
    local_28 = *(undefined4 *)(iVar13 + 0xc);
    local_24 = *(undefined4 *)(iVar13 + 0x10);
    uVar14 = 0x44d468;
    Matrix_Identity(local_20);
  }
  pbVar1 = (&PTR_s_HAMSTER_PELLET_004f70c8)[*(int *)((int)this + 0x88)];
  Matrix_Scale4x4(&stack0xffffff8c,0,0,0,0x3f800000);
  UI_DrawTextCentered(*(void **)(*(int *)((int)this + 0xc) + 0x318),pbVar1,
                      0x1cc - *(int *)((int)this + 0x70),0x178,3,3,&PTR_Vec3_dtor_004cf300,local_30,
                      local_2c,local_28,local_24,uVar7,uVar9,uVar10,uVar11,uVar14);
  if (*(int *)((int)this + 0x8c) != 0) {
    Matrix_Scale4x4(&stack0xffffff8c,0x3f400000,0x3f400000,0x3f800000,0x3f800000);
    UI_DrawRectAndReset(param_1);
    Matrix_Scale4x4(&stack0xffffff8c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    Gfx_DrawQuadRandomColor
              (*(void **)((int)this + 0x8c),&PTR_LAB_004cf584,
               (float)(*(int *)((int)this + 0x70) + 0x1e3),331.0,170.0,170.0,&PTR_LAB_004cf584,0,0,
               0x3f800000,0x3f800000);
  }
  if (*(char *)((int)this + 0x94) != '\0') {
    uVar16 = 0x3f800000;
    uVar12 = 0;
    uVar18 = 0;
    uVar17 = 0;
    uVar15 = 0x44d5ce;
    Matrix_Scale4x4(&stack0xffffff8c,0,0,0,0x3f800000);
    local_4._0_1_ = 0xb;
    Matrix_Scale4x4(&stack0xffffff78,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    local_4 = CONCAT31(local_4._1_3_,10);
    UI_DrawTextCenteredAbsolute
              (*(void **)(*(int *)((int)this + 0xc) + 0x31c),(byte *)"Click the mouse to continue!",
               0x1d1,0x22b,2,2,uVar15,uVar17,uVar18,uVar12,uVar16,uVar7,uVar9,uVar10,uVar11,uVar14);
  }
  local_4 = 0xffffffff;
  Matrix_Identity(&local_34);
  ExceptionList = local_c;
  return;
}

