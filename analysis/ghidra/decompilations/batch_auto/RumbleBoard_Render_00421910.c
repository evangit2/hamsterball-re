/*
 * Function: ArenaBoard_Render
 * Address: 0x00421910
 * Signature: void __thiscall ArenaBoard_Render(void *this,undefined4 param_1)
 *
 * Patterns: vtable dispatch, SEH frame, matrix math, rendering, scene, board. Calls: ArenaBoard_Render, __ftol2, Scene_CreateObject4f, Matrix_Scale4x4, CONCAT31, Matrix_Identity, AthenaString_Format, UI_DrawTextCentered. Offsets: 23, Lines: 251
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall ArenaBoard_Render(void *this,undefined4 param_1)

{
  int iVar1;
  byte *pbVar2;
  int *piVar3;
  int iVar4;
  ulonglong uVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  undefined **ppuVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  undefined4 uVar14;
  undefined **ppuVar15;
  undefined4 uVar16;
  undefined4 uVar17;
  undefined4 uVar18;
  undefined4 uVar19;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined **local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20 [5];
  void *local_c;
  undefined1 *puStack_8;
  uint local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004ca8c8;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  uVar5 = __ftol2();
  uVar16 = *(undefined4 *)((int)this + 0x1508);
  uVar17 = *(undefined4 *)((int)this + 0x150c);
  uVar18 = *(undefined4 *)((int)this + 0x1510);
  local_34 = &PTR_Vec3_dtor_004cf300;
  uVar19 = *(undefined4 *)((int)this + 0x4340);
  ppuVar15 = &PTR_Vec3_dtor_004cf300;
  iVar4 = (int)uVar5 / 2;
  local_4 = 0;
  local_30 = uVar16;
  local_2c = uVar17;
  local_28 = uVar18;
  local_24 = uVar19;
  Scene_CreateObject4f
            (*(void **)(*(int *)((int)this + 0x878) + 0x390),&PTR_LAB_004cf584,
             (float)(iVar4 + -0x58),10.0,180.0,105.0,&PTR_Vec3_dtor_004cf300,uVar16,uVar17,uVar18,
             uVar19);
  if (*(char *)((int)this + 0x47c5) == '\0') {
    uVar19 = *(undefined4 *)((int)this + 0x4340);
    uVar18 = 0x3f800000;
    uVar17 = 0x3f800000;
    uVar16 = 0x3f800000;
    ppuVar15 = (undefined **)0x421a22;
    Matrix_Scale4x4(&local_48,0x3f800000,0x3f800000,0x3f800000,uVar19);
    local_4 = CONCAT31(local_4._1_3_,1);
    if ((*(int *)((int)this + 0x47ac) < 0x44c) && (*(char *)((int)this + 0x89c) != '\0')) {
      uVar18 = 0;
      uVar17 = 0;
      uVar16 = 0x3f800000;
      ppuVar15 = (undefined **)0x421a52;
      iVar1 = Matrix_Scale4x4(local_20,0x3f800000,0,0,*(undefined4 *)((int)this + 0x4340));
      local_44 = *(undefined4 *)(iVar1 + 4);
      local_40 = *(undefined4 *)(iVar1 + 8);
      local_3c = *(undefined4 *)(iVar1 + 0xc);
      local_38 = *(undefined4 *)(iVar1 + 0x10);
      uVar19 = 0x421a77;
      Matrix_Identity(local_20);
    }
    if (*(int *)((int)this + 0x47ac) < 600) {
      uVar18 = 0;
      uVar17 = 0;
      uVar16 = 0x3f800000;
      ppuVar15 = (undefined **)0x421a9a;
      iVar1 = Matrix_Scale4x4(local_20,0x3f800000,0,0,*(undefined4 *)((int)this + 0x4340));
      local_44 = *(undefined4 *)(iVar1 + 4);
      local_40 = *(undefined4 *)(iVar1 + 8);
      local_3c = *(undefined4 *)(iVar1 + 0xc);
      local_38 = *(undefined4 *)(iVar1 + 0x10);
      uVar19 = 0x421abf;
      Matrix_Identity(local_20);
    }
    Matrix_Scale4x4(&stack0xffffff8c,0,0,0,*(undefined4 *)((int)this + 0x4340));
    ppuVar9 = &PTR_Vec3_dtor_004cf300;
    iVar8 = 5;
    iVar7 = 5;
    iVar6 = 0;
    iVar4 = iVar4 + 0x26;
    iVar1 = iVar4;
    uVar10 = local_44;
    uVar11 = local_40;
    uVar12 = local_3c;
    uVar13 = local_38;
    pbVar2 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
    UI_DrawTextCentered(*(void **)(*(int *)((int)this + 0x878) + 0x328),pbVar2,iVar1,iVar6,iVar7,
                        iVar8,ppuVar9,uVar10,uVar11,uVar12,uVar13,ppuVar15,uVar16,uVar17,uVar18,
                        uVar19);
    Matrix_Scale4x4(&stack0xffffff8c,0,0,0,*(undefined4 *)((int)this + 0x4340));
    ppuVar9 = &PTR_Vec3_dtor_004cf300;
    iVar7 = 5;
    iVar6 = 5;
    iVar1 = 0x39;
    uVar10 = local_44;
    uVar11 = local_40;
    uVar12 = local_3c;
    uVar13 = local_38;
    pbVar2 = (byte *)AthenaString_Format(0x4f7448,(byte *)".%.1d");
    UI_DrawTextShadow_Wrapper
              (*(void **)(*(int *)((int)this + 0x878) + 0x318),pbVar2,iVar4,iVar1,iVar6,iVar7,
               ppuVar9,uVar10,uVar11,uVar12,uVar13,ppuVar15,uVar16,uVar17,uVar18,uVar19);
    local_4 = local_4 & 0xffffff00;
    uVar19 = 0x421be4;
    Matrix_Identity(&local_48);
  }
  Matrix_Scale4x4(&stack0xffffff8c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  SceneObject_InitAtPosition
            (*(void **)(*(int *)((int)this + 0x878) + 0x438),0x4b,0x4b,ppuVar15,uVar16,uVar17,uVar18
             ,uVar19);
  uVar14 = 0x3f800000;
  uVar13 = 0;
  uVar12 = 0;
  uVar11 = 0;
  uVar10 = 0x421c31;
  Matrix_Scale4x4(&stack0xffffff8c,0,0,0,0x3f800000);
  local_4._0_1_ = 2;
  Matrix_Scale4x4(&stack0xffffff78,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  iVar7 = 5;
  iVar6 = 5;
  iVar1 = 0x10;
  iVar4 = 0x4b;
  local_4._0_1_ = 0;
  pbVar2 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
  UI_DrawTextCenteredAbsolute
            (*(void **)(*(int *)((int)this + 0x878) + 0x328),pbVar2,iVar4,iVar1,iVar6,iVar7,uVar10,
             uVar11,uVar12,uVar13,uVar14,ppuVar15,uVar16,uVar17,uVar18,uVar19);
  if (*(char *)(*(int *)((int)this + 0x878) + 0x677) == '\0') {
    Matrix_Scale4x4(&stack0xffffff8c,0,0x3f000000,0x3f800000,0x3f800000);
    SceneObject_InitAtPosition
              (*(void **)(*(int *)((int)this + 0x878) + 0x438),0x2d5,0x4b,ppuVar15,uVar16,uVar17,
               uVar18,uVar19);
    uVar14 = 0x3f800000;
    uVar13 = 0;
    uVar12 = 0;
    uVar11 = 0;
    uVar10 = 0x421cf3;
    Matrix_Scale4x4(&stack0xffffff8c,0,0,0,0x3f800000);
    local_4._0_1_ = 3;
    Matrix_Scale4x4(&stack0xffffff78,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    iVar7 = 5;
    iVar6 = 5;
    iVar1 = 0x10;
    iVar4 = 0x2d5;
    local_4._0_1_ = 0;
    pbVar2 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
    UI_DrawTextCenteredAbsolute
              (*(void **)(*(int *)((int)this + 0x878) + 0x328),pbVar2,iVar4,iVar1,iVar6,iVar7,uVar10
               ,uVar11,uVar12,uVar13,uVar14,ppuVar15,uVar16,uVar17,uVar18,uVar19);
  }
  if (*(char *)(*(int *)((int)this + 0x878) + 0x717) == '\0') {
    Matrix_Scale4x4(&stack0xffffff8c,0x3f800000,0x3e800000,0x3e800000,0x3f800000);
    SceneObject_InitAtPosition
              (*(void **)(*(int *)((int)this + 0x878) + 0x438),0x4b,0x20d,ppuVar15,uVar16,uVar17,
               uVar18,uVar19);
    uVar14 = 0x3f800000;
    uVar13 = 0;
    uVar12 = 0;
    uVar11 = 0;
    uVar10 = 0x421dbc;
    Matrix_Scale4x4(&stack0xffffff8c,0,0,0,0x3f800000);
    local_4._0_1_ = 4;
    Matrix_Scale4x4(&stack0xffffff78,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    iVar7 = 5;
    iVar6 = 5;
    iVar1 = 0x1d2;
    iVar4 = 0x4b;
    local_4._0_1_ = 0;
    pbVar2 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
    UI_DrawTextCenteredAbsolute
              (*(void **)(*(int *)((int)this + 0x878) + 0x328),pbVar2,iVar4,iVar1,iVar6,iVar7,uVar10
               ,uVar11,uVar12,uVar13,uVar14,ppuVar15,uVar16,uVar17,uVar18,uVar19);
  }
  if (*(char *)(*(int *)((int)this + 0x878) + 0x7b7) == '\0') {
    Matrix_Scale4x4(&stack0xffffff8c,0x3f800000,0x3f800000,0,0x3f800000);
    SceneObject_InitAtPosition
              (*(void **)(*(int *)((int)this + 0x878) + 0x438),0x2d5,0x20d,ppuVar15,uVar16,uVar17,
               uVar18,uVar19);
    uVar14 = 0x3f800000;
    uVar13 = 0;
    uVar12 = 0;
    uVar11 = 0;
    uVar10 = 0x421e84;
    Matrix_Scale4x4(&stack0xffffff8c,0,0,0,0x3f800000);
    local_4._0_1_ = 5;
    Matrix_Scale4x4(&stack0xffffff78,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    iVar7 = 5;
    iVar6 = 5;
    iVar1 = 0x1d2;
    iVar4 = 0x2d5;
    local_4._0_1_ = 0;
    pbVar2 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
    UI_DrawTextCenteredAbsolute
              (*(void **)(*(int *)((int)this + 0x878) + 0x328),pbVar2,iVar4,iVar1,iVar6,iVar7,uVar10
               ,uVar11,uVar12,uVar13,uVar14,ppuVar15,uVar16,uVar17,uVar18,uVar19);
  }
  uVar19 = 0x421efb;
  iVar4 = AthenaList_NextIndex((int)this + 0x8b8);
  *(undefined4 *)((int)this + iVar4 * 4 + 0x8c0) = 0;
  if (*(int *)((int)this + 0x8bc) < 1) {
    piVar3 = (int *)0x0;
  }
  else {
    piVar3 = (int *)**(undefined4 **)((int)this + 0xcc4);
    *(undefined4 *)((int)this + iVar4 * 4 + 0x8c0) = 1;
  }
  while (piVar3 != (int *)0x0) {
    uVar18 = 0x421f2a;
    uVar19 = param_1;
    (**(code **)(*piVar3 + 8))();
    iVar1 = *(int *)((int)this + iVar4 * 4 + 0x8c0);
    if (*(int *)((int)this + 0x8bc) <= iVar1) break;
    piVar3 = *(int **)(*(int *)((int)this + 0xcc4) + iVar1 * 4);
    *(int *)((int)this + iVar4 * 4 + 0x8c0) = iVar1 + 1;
  }
  if ((*(char *)((int)this + 0x47c5) != '\0') && (*(char *)((int)this + 0x47cc) != '\0')) {
    uVar14 = 0x3f800000;
    uVar13 = 0;
    uVar12 = 0;
    uVar11 = 0;
    uVar10 = 0x421f6b;
    Matrix_Scale4x4(&stack0xffffff8c,0,0,0,0x3f800000);
    local_4._0_1_ = 6;
    Matrix_Scale4x4(&stack0xffffff78,0x3f800000,0x3f800000,0,0x3f800000);
    local_4 = (uint)local_4._1_3_ << 8;
    UI_DrawTextCenteredAbsolute
              (*(void **)(*(int *)((int)this + 0x878) + 0x318),(byte *)"TIE BREAKER!",400,0x28,3,3,
               uVar10,uVar11,uVar12,uVar13,uVar14,ppuVar15,uVar16,uVar17,uVar18,uVar19);
  }
  local_4 = 0xffffffff;
  Matrix_Identity(&local_34);
  ExceptionList = local_c;
  return;
}
