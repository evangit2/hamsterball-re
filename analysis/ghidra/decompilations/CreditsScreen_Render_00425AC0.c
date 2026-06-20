// CreditsScreen_Render @ 0x00425AC0
// Decompiled via GhidraMCP v5.12.0-headless
// Session 2928 - Batch decompilation

void __thiscall CreditsScreen_Render(void *this,void *param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  byte *pbVar4;
  void *this_00;
  ulonglong uVar5;
  undefined **ppuVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  undefined4 uVar14;
  undefined4 uVar15;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48 [5];
  undefined4 local_34 [5];
  undefined4 local_20 [5];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cabf8;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *(undefined4 *)(*(int *)(*(int *)((int)this + 0xcdc) + 0x31c) + 0x428) = 0x3f400000;
  Matrix_Scale4x4(&stack0xffffff78,0,0,*(undefined4 *)((int)this + 0xce0),0x3f800000);
  Graphics_DrawScreenRect(param_1,0,0,800,600);
  iVar1 = AthenaList_NextIndex((int)this + 0x44c);
  *(undefined4 *)((int)this + iVar1 * 4 + 0x454) = 0;
  if (*(int *)((int)this + 0x450) < 1) {
    iVar3 = 0;
  }
  else {
    iVar3 = **(int **)((int)this + 0x858);
    *(undefined4 *)((int)this + iVar1 * 4 + 0x454) = 1;
  }
  while (iVar3 != 0) {
    pbVar4 = *(byte **)(iVar3 + 0x18);
    if (pbVar4 != (byte *)0x0) {
      this_00 = *(void **)(*(int *)((int)this + 0xcdc) + 0x318);
      uVar15 = 0x3f800000;
      uVar14 = 0x3f800000;
      uVar13 = 0x3f800000;
      uVar12 = 0x3f800000;
      uVar11 = 0x425ba4;
      Matrix_Scale4x4(&local_5c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
      local_4 = 0;
      if (*pbVar4 == 0x2d) {
        uVar14 = 0x3f800000;
        uVar13 = 0x3f400000;
        uVar12 = 0x3f400000;
        uVar11 = 0x425bce;
        iVar2 = Matrix_Scale4x4(local_48,0x3f400000,0x3f400000,0x3f800000,0x3f800000);
        local_58 = *(undefined4 *)(iVar2 + 4);
        local_54 = *(undefined4 *)(iVar2 + 8);
        local_50 = *(undefined4 *)(iVar2 + 0xc);
        local_4c = *(undefined4 *)(iVar2 + 0x10);
        uVar15 = 0x425bf3;
        Matrix_Identity(local_48);
        this_00 = *(void **)(*(int *)((int)this + 0xcdc) + 800);
        pbVar4 = pbVar4 + 1;
      }
      if (*pbVar4 == 0x2a) {
        uVar14 = 0x3f400000;
        uVar13 = 0x3f800000;
        uVar12 = 0x3f800000;
        uVar11 = 0x425c26;
        iVar2 = Matrix_Scale4x4(local_34,0x3f800000,0x3f800000,0x3f400000,0x3f800000);
        local_58 = *(undefined4 *)(iVar2 + 4);
        local_54 = *(undefined4 *)(iVar2 + 8);
        local_50 = *(undefined4 *)(iVar2 + 0xc);
        local_4c = *(undefined4 *)(iVar2 + 0x10);
        uVar15 = 0x425c4b;
        Matrix_Identity(local_34);
        if (*(int *)((int)this + 0x864) == iVar3) {
          uVar14 = 0x3f400000;
          uVar13 = 0x3f800000;
          uVar12 = 0x3f400000;
          uVar11 = 0x425c70;
          iVar3 = Matrix_Scale4x4(local_20,0x3f400000,0x3f800000,0x3f400000,0x3f800000);
          local_58 = *(undefined4 *)(iVar3 + 4);
          local_54 = *(undefined4 *)(iVar3 + 8);
          local_50 = *(undefined4 *)(iVar3 + 0xc);
          local_4c = *(undefined4 *)(iVar3 + 0x10);
          uVar15 = 0x425c95;
          Matrix_Identity(local_20);
        }
        this_00 = *(void **)(*(int *)((int)this + 0xcdc) + 0x31c);
        pbVar4 = pbVar4 + 1;
      }
      Matrix_Scale4x4(&stack0xffffff78,0,0,0,0x3f800000);
      ppuVar6 = &PTR_Vec3_dtor_004cf300;
      iVar2 = 3;
      iVar3 = 3;
      uVar7 = local_58;
      uVar8 = local_54;
      uVar9 = local_50;
      uVar10 = local_4c;
      uVar5 = __ftol2();
      UI_DrawTextCenteredAbsolute
                (this_00,pbVar4,400,(int)uVar5,iVar3,iVar2,ppuVar6,uVar7,uVar8,uVar9,uVar10,uVar11,
                 uVar12,uVar13,uVar14,uVar15);
      local_4 = 0xffffffff;
      Matrix_Identity(&local_5c);
    }
    iVar2 = *(int *)((int)this + iVar1 * 4 + 0x454);
    if (*(int *)((int)this + 0x450) <= iVar2) break;
    iVar3 = *(int *)(*(int *)((int)this + 0x858) + iVar2 * 4);
    *(int *)((int)this + iVar1 * 4 + 0x454) = iVar2 + 1;
  }
  *(undefined4 *)(*(int *)(*(int *)((int)this + 0xcdc) + 0x31c) + 0x428) = 0x3f800000;
  UIList_Render(this,param_1);
  ExceptionList = local_c;
  return;
}