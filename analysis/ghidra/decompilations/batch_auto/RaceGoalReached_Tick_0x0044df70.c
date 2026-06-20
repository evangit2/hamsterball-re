
void __thiscall RaceGoalReached_Tick(void *this,void *param_1)

{
  byte *pbVar1;
  ulonglong uVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  undefined4 uVar6;
  undefined **ppuVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  int iVar13;
  undefined4 uVar14;
  undefined4 in_stack_ffffff90;
  undefined4 in_stack_ffffff94;
  undefined4 in_stack_ffffff98;
  undefined4 in_stack_ffffff9c;
  undefined4 in_stack_ffffffa0;
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
  undefined4 local_20 [4];
  void *pvStack_10;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cc828;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  Matrix_Scale4x4(&stack0xffffff90,0x3f800000,0x3f800000,0x3f800000,0);
  UI_DrawRectAndReset(param_1);
  Matrix_Scale4x4(&stack0xffffff90,0x3f800000,0x3f800000,0x3f800000,
                  *(undefined4 *)((int)this + 0x1c));
  Gfx_DrawQuadRandomColor
            (*(void **)(*(int *)((int)this + 0xc) + 0x354),&PTR_LAB_004cf584,40.0,25.0,702.0,569.0,
             &PTR_LAB_004cf584,0x3f800000,0,0,0x3f800000);
  Matrix_Scale4x4(&stack0xffffff90,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  iVar13 = 0x102;
  uVar2 = __ftol2();
  SceneObject_InitAtPosition
            (*(void **)((int)this + 0x4e0),(int)uVar2,iVar13,in_stack_ffffff90,in_stack_ffffff94,
             in_stack_ffffff98,in_stack_ffffff9c,in_stack_ffffffa0);
  Matrix_Scale4x4(&stack0xffffff90,0x3f800000,0x3f800000,0,0x3f800000);
  UI_DrawRectAndReset(param_1);
  uVar14 = 0x3f800000;
  uVar11 = 0;
  uVar9 = 0;
  uVar8 = 0;
  uVar6 = 0x44e0c8;
  Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
  local_4 = 0;
  Matrix_Scale4x4(&stack0xffffff7c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  local_4 = 0xffffffff;
  UI_DrawTextCenteredAbsolute
            (*(void **)(*(int *)((int)this + 0xc) + 0x318),*(byte **)((int)this + 0x24),
             (200 - *(int *)((int)this + 0x20)) * 2,0x23,3,3,uVar6,uVar8,uVar9,uVar11,uVar14,
             in_stack_ffffff90,in_stack_ffffff94,in_stack_ffffff98,in_stack_ffffff9c,
             in_stack_ffffffa0);
  if ((*(int *)((int)this + 0x4c) < *(int *)((int)this + 0x10)) &&
     (*(int *)(*(int *)((int)this + 8) + 0x1c) <= *(int *)(*(int *)((int)this + 4) + 0x2994))) {
    in_stack_ffffffa0 = 99;
    in_stack_ffffff9c = 0x208;
    in_stack_ffffff98 = 0x44e13e;
    Graphics_SetScaleAndPosition(*(void **)(*(int *)((int)this + 0xc) + 0x37c),0x208,99);
  }
  if (*(int *)((int)this + 0x28) < *(int *)((int)this + 0x10)) {
    uVar14 = 0x3f800000;
    uVar11 = 0;
    uVar9 = 0;
    uVar8 = 0;
    uVar6 = 0x44e15c;
    Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
    local_4 = 1;
    Matrix_Scale4x4(&stack0xffffff7c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    local_4 = 0xffffffff;
    UI_DrawTextCentered(*(void **)(*(int *)((int)this + 0xc) + 0x318),(byte *)0x4d03d0,0x1ea,0x84,3,
                        3,uVar6,uVar8,uVar9,uVar11,uVar14,in_stack_ffffff90,in_stack_ffffff94,
                        in_stack_ffffff98,in_stack_ffffff9c,in_stack_ffffffa0);
    if (0 < *(int *)((int)this + 0x4ac)) {
      uVar11 = 0x3f800000;
      uVar9 = 0;
      uVar8 = 0;
      uVar6 = 0;
      uVar14 = 0x44e1bf;
      Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
      local_4 = 2;
      Matrix_Scale4x4(&stack0xffffff7c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
      iVar5 = 3;
      iVar4 = 3;
      iVar3 = 0x84;
      iVar13 = 0x226;
      local_4 = 0xffffffff;
      pbVar1 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
      UI_DrawTextCentered(*(void **)(*(int *)((int)this + 0xc) + 0x318),pbVar1,iVar13,iVar3,iVar4,
                          iVar5,uVar14,uVar6,uVar8,uVar9,uVar11,in_stack_ffffff90,in_stack_ffffff94,
                          in_stack_ffffff98,in_stack_ffffff9c,in_stack_ffffffa0);
      uVar14 = 0x3f800000;
      uVar11 = 0;
      uVar9 = 0;
      uVar8 = 0;
      uVar6 = 0x44e23c;
      Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
      local_4 = 3;
      Matrix_Scale4x4(&stack0xffffff7c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
      iVar5 = 3;
      iVar4 = 3;
      iVar3 = 0x98;
      iVar13 = 0x226;
      local_4 = 0xffffffff;
      pbVar1 = (byte *)AthenaString_Format(0x4f7448,(byte *)".%.1d");
      UI_DrawTextShadow_Wrapper
                (*(void **)(*(int *)((int)this + 0xc) + 0x31c),pbVar1,iVar13,iVar3,iVar4,iVar5,uVar6
                 ,uVar8,uVar9,uVar11,uVar14,in_stack_ffffff90,in_stack_ffffff94,in_stack_ffffff98,
                 in_stack_ffffff9c,in_stack_ffffffa0);
    }
  }
  if (*(int *)((int)this + 0x30) < *(int *)((int)this + 0x10)) {
    uVar14 = 0x3f800000;
    uVar11 = 0;
    uVar9 = 0;
    uVar8 = 0;
    uVar6 = 0x44e2cf;
    Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
    local_4 = 4;
    Matrix_Scale4x4(&stack0xffffff7c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    local_4 = 0xffffffff;
    UI_DrawTextCentered(*(void **)(*(int *)((int)this + 0xc) + 0x318),(byte *)"BEST RACE TIME:",
                        0x1ea,0xa8,3,3,uVar6,uVar8,uVar9,uVar11,uVar14,in_stack_ffffff90,
                        in_stack_ffffff94,in_stack_ffffff98,in_stack_ffffff9c,in_stack_ffffffa0);
    if (0 < *(int *)((int)this + 0x4b0)) {
      uVar6 = 0x3f800000;
      in_stack_ffffff9c = 0x3f800000;
      in_stack_ffffff98 = 0x3f800000;
      in_stack_ffffff94 = 0x3f800000;
      in_stack_ffffff90 = 0x44e32d;
      Matrix_Scale4x4(&local_48,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
      local_4 = 5;
      if (*(int *)(*(int *)((int)this + 8) + 0x1c) ==
          *(int *)(*(int *)((int)this + 0xc) + 0x86c +
                  *(int *)(*(int *)(*(int *)((int)this + 0xc) + 0x220) + 8) * 4)) {
        in_stack_ffffff9c = 0;
        in_stack_ffffff98 = 0x3f800000;
        in_stack_ffffff94 = 0;
        in_stack_ffffff90 = 0x44e35d;
        iVar13 = Matrix_Scale4x4(&local_34,0,0x3f800000,0,0x3f800000);
        local_44 = *(undefined4 *)(iVar13 + 4);
        local_40 = *(undefined4 *)(iVar13 + 8);
        local_3c = *(undefined4 *)(iVar13 + 0xc);
        local_38 = *(undefined4 *)(iVar13 + 0x10);
        uVar6 = 0x44e382;
        Matrix_Identity(&local_34);
      }
      Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
      ppuVar7 = &PTR_Vec3_dtor_004cf300;
      iVar5 = 3;
      iVar4 = 3;
      iVar3 = 0xa8;
      iVar13 = 0x226;
      uVar14 = local_44;
      uVar8 = local_40;
      uVar9 = local_3c;
      uVar11 = local_38;
      pbVar1 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
      UI_DrawTextCentered(*(void **)(*(int *)((int)this + 0xc) + 0x318),pbVar1,iVar13,iVar3,iVar4,
                          iVar5,ppuVar7,uVar14,uVar8,uVar9,uVar11,in_stack_ffffff90,
                          in_stack_ffffff94,in_stack_ffffff98,in_stack_ffffff9c,uVar6);
      Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
      ppuVar7 = &PTR_Vec3_dtor_004cf300;
      iVar5 = 3;
      iVar4 = 3;
      iVar3 = 0xbc;
      iVar13 = 0x226;
      uVar8 = local_44;
      uVar9 = local_40;
      uVar11 = local_3c;
      uVar14 = local_38;
      pbVar1 = (byte *)AthenaString_Format(0x4f7448,(byte *)".%.1d");
      UI_DrawTextShadow_Wrapper
                (*(void **)(*(int *)((int)this + 0xc) + 0x31c),pbVar1,iVar13,iVar3,iVar4,iVar5,
                 ppuVar7,uVar8,uVar9,uVar11,uVar14,in_stack_ffffff90,in_stack_ffffff94,
                 in_stack_ffffff98,in_stack_ffffff9c,uVar6);
      local_4 = 0xffffffff;
      in_stack_ffffffa0 = 0x44e4a7;
      Matrix_Identity(&local_48);
    }
  }
  if (*(int *)((int)this + 0x40) < *(int *)((int)this + 0x10)) {
    uVar14 = 0x3f800000;
    uVar11 = 0;
    uVar9 = 0;
    uVar8 = 0;
    uVar6 = 0x44e4c5;
    Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
    local_4 = 6;
    Matrix_Scale4x4(&stack0xffffff7c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    local_4 = 0xffffffff;
    UI_DrawTextCentered(*(void **)(*(int *)((int)this + 0xc) + 0x318),(byte *)"WEASEL\'S TIME:",
                        0x1ea,0xcc,3,3,uVar6,uVar8,uVar9,uVar11,uVar14,in_stack_ffffff90,
                        in_stack_ffffff94,in_stack_ffffff98,in_stack_ffffff9c,in_stack_ffffffa0);
    if (0 < *(int *)((int)this + 0x4b4)) {
      uVar6 = 0x3f800000;
      in_stack_ffffff9c = 0x3f800000;
      in_stack_ffffff98 = 0x3f800000;
      in_stack_ffffff94 = 0x3f800000;
      in_stack_ffffff90 = 0x44e523;
      Matrix_Scale4x4(&local_48,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
      local_4 = 7;
      if ((*(int *)((int)this + 0x48) < *(int *)((int)this + 0x10)) &&
         (*(int *)(*(int *)((int)this + 8) + 0x1c) <= *(int *)(*(int *)((int)this + 4) + 0x2994))) {
        in_stack_ffffff9c = 0;
        in_stack_ffffff98 = 0x3f800000;
        in_stack_ffffff94 = 0;
        in_stack_ffffff90 = 0x44e551;
        iVar13 = Matrix_Scale4x4(&local_34,0,0x3f800000,0,0x3f800000);
        local_44 = *(undefined4 *)(iVar13 + 4);
        local_40 = *(undefined4 *)(iVar13 + 8);
        local_3c = *(undefined4 *)(iVar13 + 0xc);
        local_38 = *(undefined4 *)(iVar13 + 0x10);
        uVar6 = 0x44e576;
        Matrix_Identity(&local_34);
      }
      Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
      ppuVar7 = &PTR_Vec3_dtor_004cf300;
      iVar5 = 3;
      iVar4 = 3;
      iVar3 = 0xcc;
      iVar13 = 0x226;
      uVar14 = local_44;
      uVar8 = local_40;
      uVar9 = local_3c;
      uVar11 = local_38;
      pbVar1 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
      UI_DrawTextCentered(*(void **)(*(int *)((int)this + 0xc) + 0x318),pbVar1,iVar13,iVar3,iVar4,
                          iVar5,ppuVar7,uVar14,uVar8,uVar9,uVar11,in_stack_ffffff90,
                          in_stack_ffffff94,in_stack_ffffff98,in_stack_ffffff9c,uVar6);
      Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
      ppuVar7 = &PTR_Vec3_dtor_004cf300;
      iVar5 = 3;
      iVar4 = 3;
      iVar3 = 0xe0;
      iVar13 = 0x226;
      uVar8 = local_44;
      uVar9 = local_40;
      uVar11 = local_3c;
      uVar14 = local_38;
      pbVar1 = (byte *)AthenaString_Format(0x4f7448,(byte *)".%.1d");
      UI_DrawTextShadow_Wrapper
                (*(void **)(*(int *)((int)this + 0xc) + 0x31c),pbVar1,iVar13,iVar3,iVar4,iVar5,
                 ppuVar7,uVar8,uVar9,uVar11,uVar14,in_stack_ffffff90,in_stack_ffffff94,
                 in_stack_ffffff98,in_stack_ffffff9c,uVar6);
      local_4 = 0xffffffff;
      in_stack_ffffffa0 = 0x44e69b;
      Matrix_Identity(&local_48);
    }
  }
  if (*(int *)((int)this + 0x50) < *(int *)((int)this + 0x10)) {
    uVar14 = 0x3f800000;
    uVar11 = 0;
    uVar9 = 0;
    uVar8 = 0;
    uVar6 = 0x44e6b9;
    Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
    local_4 = 8;
    Matrix_Scale4x4(&stack0xffffff7c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    local_4 = 0xffffffff;
    UI_DrawTextCentered(*(void **)(*(int *)((int)this + 0xc) + 0x318),(byte *)"BROKEN BALLS:",0x24e,
                        0x104,3,3,uVar6,uVar8,uVar9,uVar11,uVar14,in_stack_ffffff90,
                        in_stack_ffffff94,in_stack_ffffff98,in_stack_ffffff9c,in_stack_ffffffa0);
    if (*(int *)((int)this + 0x54) < *(int *)((int)this + 0x10)) {
      uVar6 = 0x3f800000;
      in_stack_ffffff9c = 0x3f800000;
      in_stack_ffffff98 = 0x3f800000;
      in_stack_ffffff94 = 0x3f800000;
      in_stack_ffffff90 = 0x44e717;
      Matrix_Scale4x4(&local_48,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
      local_4 = 9;
      if (*(int *)(*(int *)((int)this + 8) + 0x28) == 0) {
        in_stack_ffffff9c = 0;
        in_stack_ffffff98 = 0x3f800000;
        in_stack_ffffff94 = 0;
        in_stack_ffffff90 = 0x44e734;
        iVar13 = Matrix_Scale4x4(&local_34,0,0x3f800000,0,0x3f800000);
        local_44 = *(undefined4 *)(iVar13 + 4);
        local_40 = *(undefined4 *)(iVar13 + 8);
        local_3c = *(undefined4 *)(iVar13 + 0xc);
        local_38 = *(undefined4 *)(iVar13 + 0x10);
        uVar6 = 0x44e759;
        Matrix_Identity(&local_34);
      }
      if (0 < *(int *)(*(int *)((int)this + 8) + 0x28)) {
        in_stack_ffffff9c = 0;
        in_stack_ffffff98 = 0x3f800000;
        in_stack_ffffff94 = 0x3f800000;
        in_stack_ffffff90 = 0x44e76e;
        iVar13 = Matrix_Scale4x4(&local_34,0x3f800000,0x3f800000,0,0x3f800000);
        local_44 = *(undefined4 *)(iVar13 + 4);
        local_40 = *(undefined4 *)(iVar13 + 8);
        local_3c = *(undefined4 *)(iVar13 + 0xc);
        local_38 = *(undefined4 *)(iVar13 + 0x10);
        uVar6 = 0x44e793;
        Matrix_Identity(&local_34);
      }
      if (5 < *(int *)(*(int *)((int)this + 8) + 0x28)) {
        in_stack_ffffff9c = 0;
        in_stack_ffffff98 = 0;
        in_stack_ffffff94 = 0x3f800000;
        in_stack_ffffff90 = 0x44e7a9;
        iVar13 = Matrix_Scale4x4(&local_34,0x3f800000,0,0,0x3f800000);
        local_44 = *(undefined4 *)(iVar13 + 4);
        local_40 = *(undefined4 *)(iVar13 + 8);
        local_3c = *(undefined4 *)(iVar13 + 0xc);
        local_38 = *(undefined4 *)(iVar13 + 0x10);
        uVar6 = 0x44e7ce;
        Matrix_Identity(&local_34);
      }
      Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
      ppuVar7 = &PTR_Vec3_dtor_004cf300;
      iVar5 = 3;
      iVar4 = 3;
      iVar3 = 0x104;
      iVar13 = 0x25d;
      uVar8 = local_44;
      uVar9 = local_40;
      uVar11 = local_3c;
      uVar14 = local_38;
      pbVar1 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
      UI_DrawTextShadow_Wrapper
                (*(void **)(*(int *)((int)this + 0xc) + 0x318),pbVar1,iVar13,iVar3,iVar4,iVar5,
                 ppuVar7,uVar8,uVar9,uVar11,uVar14,in_stack_ffffff90,in_stack_ffffff94,
                 in_stack_ffffff98,in_stack_ffffff9c,uVar6);
      local_4 = 0xffffffff;
      in_stack_ffffffa0 = 0x44e851;
      Matrix_Identity(&local_48);
    }
  }
  if (*(int *)((int)this + 0x58) < *(int *)((int)this + 0x10)) {
    uVar14 = 0x3f800000;
    uVar11 = 0;
    uVar9 = 0;
    uVar8 = 0;
    uVar6 = 0x44e86f;
    Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
    local_4 = 10;
    Matrix_Scale4x4(&stack0xffffff7c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    local_4 = 0xffffffff;
    UI_DrawTextCentered(*(void **)(*(int *)((int)this + 0xc) + 0x318),(byte *)"DIZZIED BALLS:",0x24e
                        ,0x128,3,3,uVar6,uVar8,uVar9,uVar11,uVar14,in_stack_ffffff90,
                        in_stack_ffffff94,in_stack_ffffff98,in_stack_ffffff9c,in_stack_ffffffa0);
    if (*(int *)((int)this + 0x5c) < *(int *)((int)this + 0x10)) {
      uVar6 = 0x3f800000;
      in_stack_ffffff9c = 0x3f800000;
      in_stack_ffffff98 = 0x3f800000;
      in_stack_ffffff94 = 0x3f800000;
      in_stack_ffffff90 = 0x44e8cd;
      Matrix_Scale4x4(&local_48,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
      local_4 = 0xb;
      if (*(int *)(*(int *)((int)this + 8) + 0x2c) == 0) {
        in_stack_ffffff9c = 0;
        in_stack_ffffff98 = 0x3f800000;
        in_stack_ffffff94 = 0;
        in_stack_ffffff90 = 0x44e8ea;
        iVar13 = Matrix_Scale4x4(&local_34,0,0x3f800000,0,0x3f800000);
        local_44 = *(undefined4 *)(iVar13 + 4);
        local_40 = *(undefined4 *)(iVar13 + 8);
        local_3c = *(undefined4 *)(iVar13 + 0xc);
        local_38 = *(undefined4 *)(iVar13 + 0x10);
        uVar6 = 0x44e90f;
        Matrix_Identity(&local_34);
      }
      Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
      ppuVar7 = &PTR_Vec3_dtor_004cf300;
      iVar5 = 3;
      iVar4 = 3;
      iVar3 = 0x128;
      iVar13 = 0x25d;
      uVar8 = local_44;
      uVar9 = local_40;
      uVar11 = local_3c;
      uVar14 = local_38;
      pbVar1 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
      UI_DrawTextShadow_Wrapper
                (*(void **)(*(int *)((int)this + 0xc) + 0x318),pbVar1,iVar13,iVar3,iVar4,iVar5,
                 ppuVar7,uVar8,uVar9,uVar11,uVar14,in_stack_ffffff90,in_stack_ffffff94,
                 in_stack_ffffff98,in_stack_ffffff9c,uVar6);
      local_4 = 0xffffffff;
      in_stack_ffffffa0 = 0x44e992;
      Matrix_Identity(&local_48);
    }
  }
  if (*(int *)((int)this + 0x60) < *(int *)((int)this + 0x10)) {
    uVar11 = 0x3f800000;
    uVar9 = 0;
    uVar8 = 0;
    uVar6 = 0;
    uVar14 = 0x44e9b0;
    Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
    local_4 = 0xc;
    Matrix_Scale4x4(&stack0xffffff7c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    local_4 = 0xffffffff;
    UI_DrawTextCentered(*(void **)(*(int *)((int)this + 0xc) + 0x318),(byte *)"BRONZE TIME:",0x262,
                        0x170,3,3,uVar14,uVar6,uVar8,uVar9,uVar11,in_stack_ffffff90,
                        in_stack_ffffff94,in_stack_ffffff98,in_stack_ffffff9c,in_stack_ffffffa0);
    uVar6 = 0x3f800000;
    in_stack_ffffff9c = 0x3f800000;
    in_stack_ffffff98 = 0x3f800000;
    in_stack_ffffff94 = 0x3f800000;
    in_stack_ffffff90 = 0x44ea02;
    Matrix_Scale4x4(&local_48,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    local_4 = 0xd;
    if (*(int *)(*(int *)((int)this + 8) + 0x1c) <= *(int *)(*(int *)((int)this + 4) + 0x29a8)) {
      in_stack_ffffff9c = 0;
      in_stack_ffffff98 = 0x3f800000;
      in_stack_ffffff94 = 0;
      in_stack_ffffff90 = 0x44ea28;
      iVar13 = Matrix_Scale4x4(&local_34,0,0x3f800000,0,0x3f800000);
      local_44 = *(undefined4 *)(iVar13 + 4);
      local_40 = *(undefined4 *)(iVar13 + 8);
      local_3c = *(undefined4 *)(iVar13 + 0xc);
      local_38 = *(undefined4 *)(iVar13 + 0x10);
      uVar6 = 0x44ea4d;
      Matrix_Identity(&local_34);
    }
    Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
    ppuVar7 = &PTR_Vec3_dtor_004cf300;
    iVar5 = 3;
    iVar4 = 3;
    iVar3 = 0x170;
    iVar13 = 0x29e;
    uVar14 = local_44;
    uVar8 = local_40;
    uVar9 = local_3c;
    uVar11 = local_38;
    pbVar1 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
    UI_DrawTextCentered(*(void **)(*(int *)((int)this + 0xc) + 0x318),pbVar1,iVar13,iVar3,iVar4,
                        iVar5,ppuVar7,uVar14,uVar8,uVar9,uVar11,in_stack_ffffff90,in_stack_ffffff94,
                        in_stack_ffffff98,in_stack_ffffff9c,uVar6);
    Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
    ppuVar7 = &PTR_Vec3_dtor_004cf300;
    iVar5 = 3;
    iVar4 = 3;
    iVar3 = 0x184;
    iVar13 = 0x29e;
    uVar8 = local_44;
    uVar9 = local_40;
    uVar11 = local_3c;
    uVar14 = local_38;
    pbVar1 = (byte *)AthenaString_Format(0x4f7448,(byte *)".%.1d");
    UI_DrawTextShadow_Wrapper
              (*(void **)(*(int *)((int)this + 0xc) + 0x31c),pbVar1,iVar13,iVar3,iVar4,iVar5,ppuVar7
               ,uVar8,uVar9,uVar11,uVar14,in_stack_ffffff90,in_stack_ffffff94,in_stack_ffffff98,
               in_stack_ffffff9c,uVar6);
    local_4 = 0xffffffff;
    in_stack_ffffffa0 = 0x44eb78;
    Matrix_Identity(&local_48);
  }
  if (*(int *)((int)this + 100) < *(int *)((int)this + 0x10)) {
    uVar11 = 0x3f800000;
    uVar9 = 0;
    uVar8 = 0;
    uVar6 = 0;
    uVar14 = 0x44eb96;
    Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
    local_4 = 0xe;
    Matrix_Scale4x4(&stack0xffffff7c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    local_4 = 0xffffffff;
    UI_DrawTextCentered(*(void **)(*(int *)((int)this + 0xc) + 0x318),(byte *)"SILVER TIME:",0x262,
                        0x194,3,3,uVar14,uVar6,uVar8,uVar9,uVar11,in_stack_ffffff90,
                        in_stack_ffffff94,in_stack_ffffff98,in_stack_ffffff9c,in_stack_ffffffa0);
    uVar6 = 0x3f800000;
    in_stack_ffffff9c = 0x3f800000;
    in_stack_ffffff98 = 0x3f800000;
    in_stack_ffffff94 = 0x3f800000;
    in_stack_ffffff90 = 0x44ebe8;
    Matrix_Scale4x4(&local_48,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    local_4 = 0xf;
    if (*(int *)(*(int *)((int)this + 8) + 0x1c) <= *(int *)(*(int *)((int)this + 4) + 0x29a4)) {
      in_stack_ffffff9c = 0;
      in_stack_ffffff98 = 0x3f800000;
      in_stack_ffffff94 = 0;
      in_stack_ffffff90 = 0x44ec0e;
      iVar13 = Matrix_Scale4x4(&local_34,0,0x3f800000,0,0x3f800000);
      local_44 = *(undefined4 *)(iVar13 + 4);
      local_40 = *(undefined4 *)(iVar13 + 8);
      local_3c = *(undefined4 *)(iVar13 + 0xc);
      local_38 = *(undefined4 *)(iVar13 + 0x10);
      uVar6 = 0x44ec33;
      Matrix_Identity(&local_34);
    }
    Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
    ppuVar7 = &PTR_Vec3_dtor_004cf300;
    iVar5 = 3;
    iVar4 = 3;
    iVar3 = 0x194;
    iVar13 = 0x29e;
    uVar14 = local_44;
    uVar8 = local_40;
    uVar9 = local_3c;
    uVar11 = local_38;
    pbVar1 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
    UI_DrawTextCentered(*(void **)(*(int *)((int)this + 0xc) + 0x318),pbVar1,iVar13,iVar3,iVar4,
                        iVar5,ppuVar7,uVar14,uVar8,uVar9,uVar11,in_stack_ffffff90,in_stack_ffffff94,
                        in_stack_ffffff98,in_stack_ffffff9c,uVar6);
    Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
    ppuVar7 = &PTR_Vec3_dtor_004cf300;
    iVar5 = 3;
    iVar4 = 3;
    iVar3 = 0x1a8;
    iVar13 = 0x29e;
    uVar8 = local_44;
    uVar9 = local_40;
    uVar11 = local_3c;
    uVar14 = local_38;
    pbVar1 = (byte *)AthenaString_Format(0x4f7448,(byte *)".%.1d");
    UI_DrawTextShadow_Wrapper
              (*(void **)(*(int *)((int)this + 0xc) + 0x31c),pbVar1,iVar13,iVar3,iVar4,iVar5,ppuVar7
               ,uVar8,uVar9,uVar11,uVar14,in_stack_ffffff90,in_stack_ffffff94,in_stack_ffffff98,
               in_stack_ffffff9c,uVar6);
    local_4 = 0xffffffff;
    in_stack_ffffffa0 = 0x44ed5e;
    Matrix_Identity(&local_48);
  }
  if (*(int *)((int)this + 0x68) < *(int *)((int)this + 0x10)) {
    uVar11 = 0x3f800000;
    uVar9 = 0;
    uVar8 = 0;
    uVar6 = 0;
    uVar14 = 0x44ed7c;
    Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
    local_4 = 0x10;
    Matrix_Scale4x4(&stack0xffffff7c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    local_4 = 0xffffffff;
    UI_DrawTextCentered(*(void **)(*(int *)((int)this + 0xc) + 0x318),(byte *)0x4d6d94,0x262,0x1b8,3
                        ,3,uVar14,uVar6,uVar8,uVar9,uVar11,in_stack_ffffff90,in_stack_ffffff94,
                        in_stack_ffffff98,in_stack_ffffff9c,in_stack_ffffffa0);
    uVar8 = 0x3f800000;
    uVar6 = 0x3f800000;
    in_stack_ffffff98 = 0x3f800000;
    in_stack_ffffff94 = 0x3f800000;
    in_stack_ffffff90 = 0x44edce;
    Matrix_Scale4x4(&local_34,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    local_4 = 0x11;
    if (*(int *)(*(int *)((int)this + 8) + 0x1c) <= *(int *)(*(int *)((int)this + 4) + 0x29a0)) {
      uVar6 = 0;
      in_stack_ffffff98 = 0x3f800000;
      in_stack_ffffff94 = 0;
      in_stack_ffffff90 = 0x44edf4;
      iVar13 = Matrix_Scale4x4(local_20,0,0x3f800000,0,0x3f800000);
      local_30 = *(undefined4 *)(iVar13 + 4);
      local_2c = *(undefined4 *)(iVar13 + 8);
      local_28 = *(undefined4 *)(iVar13 + 0xc);
      local_24 = *(undefined4 *)(iVar13 + 0x10);
      uVar8 = 0x44ee19;
      Matrix_Identity(local_20);
    }
    Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
    ppuVar7 = &PTR_Vec3_dtor_004cf300;
    iVar5 = 3;
    iVar4 = 3;
    iVar3 = 0x1b8;
    iVar13 = 0x29e;
    uVar14 = local_30;
    uVar9 = local_2c;
    uVar11 = local_28;
    uVar10 = local_24;
    pbVar1 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
    UI_DrawTextCentered(*(void **)(*(int *)((int)this + 0xc) + 0x318),pbVar1,iVar13,iVar3,iVar4,
                        iVar5,ppuVar7,uVar14,uVar9,uVar11,uVar10,in_stack_ffffff90,in_stack_ffffff94
                        ,in_stack_ffffff98,uVar6,uVar8);
    Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
    ppuVar7 = &PTR_Vec3_dtor_004cf300;
    iVar5 = 3;
    iVar4 = 3;
    iVar3 = 0x1cc;
    iVar13 = 0x29e;
    uVar9 = local_30;
    uVar11 = local_2c;
    uVar10 = local_28;
    uVar14 = local_24;
    pbVar1 = (byte *)AthenaString_Format(0x4f7448,(byte *)".%.1d");
    UI_DrawTextShadow_Wrapper
              (*(void **)(*(int *)((int)this + 0xc) + 0x31c),pbVar1,iVar13,iVar3,iVar4,iVar5,ppuVar7
               ,uVar9,uVar11,uVar10,uVar14,in_stack_ffffff90,in_stack_ffffff94,in_stack_ffffff98,
               uVar6,uVar8);
    local_4 = 0xffffffff;
    Matrix_Identity(&local_34);
  }
  if ((*(int *)((int)this + 0x6c) < *(int *)((int)this + 0x10)) &&
     (*(int *)(*(int *)((int)this + 8) + 0x1c) <= *(int *)(*(int *)((int)this + 4) + 0x29a8))) {
    in_stack_ffffff98 = 0x44ef75;
    Graphics_SetScaleAndPosition(*(void **)(*(int *)((int)this + 0xc) + 0x370),0x112,0x16f);
  }
  if ((*(int *)((int)this + 0x70) < *(int *)((int)this + 0x10)) &&
     (*(int *)(*(int *)((int)this + 8) + 0x1c) <= *(int *)(*(int *)((int)this + 4) + 0x29a4))) {
    in_stack_ffffff98 = 0x44efa6;
    Graphics_SetScaleAndPosition(*(void **)(*(int *)((int)this + 0xc) + 0x374),0x127,0x193);
  }
  if ((*(int *)((int)this + 0x74) < *(int *)((int)this + 0x10)) &&
     (*(int *)(*(int *)((int)this + 8) + 0x1c) <= *(int *)(*(int *)((int)this + 4) + 0x29a0))) {
    in_stack_ffffff98 = 0x44efd7;
    Graphics_SetScaleAndPosition(*(void **)(*(int *)((int)this + 0xc) + 0x378),0x140,0x1b7);
  }
  uVar6 = 0x44eff0;
  (**(code **)(*(int *)((int)this + 0x90) + 8))();
  if (*(char *)((int)this + 0x4e4) != '\0') {
    uVar12 = 0x3f800000;
    uVar10 = 0;
    uVar11 = 0;
    uVar9 = 0;
    uVar8 = 0x44f00a;
    Matrix_Scale4x4(&stack0xffffff8c,0,0,0,0x3f800000);
    puStack_8 = (undefined1 *)0x12;
    Matrix_Scale4x4(&stack0xffffff78,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    puStack_8 = (undefined1 *)0xffffffff;
    UI_DrawTextCenteredAbsolute
              (*(void **)(*(int *)((int)this + 0xc) + 0x31c),(byte *)"Click the mouse to continue!",
               0x1d1,0x22b,2,2,uVar8,uVar9,uVar11,uVar10,uVar12,uVar14,in_stack_ffffff90,
               in_stack_ffffff94,in_stack_ffffff98,uVar6);
  }
  ExceptionList = pvStack_10;
  return;
}

