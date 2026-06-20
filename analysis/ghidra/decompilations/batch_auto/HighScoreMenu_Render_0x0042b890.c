
void __thiscall HighScoreMenu_Render(void *this,void *param_1)

{
  int iVar1;
  byte *pbVar2;
  uint uVar3;
  undefined1 *puVar4;
  bool bVar5;
  ulonglong uVar6;
  undefined1 *puVar7;
  int iVar8;
  int iVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  int iVar13;
  undefined4 uVar14;
  int iVar15;
  undefined4 uVar16;
  undefined4 uVar17;
  undefined4 in_stack_ffffffc4;
  undefined4 in_stack_ffffffc8;
  undefined4 in_stack_ffffffcc;
  undefined4 in_stack_ffffffd0;
  undefined4 in_stack_ffffffd4;
  int *piVar18;
  undefined1 *local_14;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cb058;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  Matrix_Scale4x4(&stack0xffffffc4,0x3f266666,0,0x3f800000,*(undefined4 *)((int)this + 0xce0));
  Graphics_DrawScreenRect(param_1,0,0,800,600);
  Matrix_Scale4x4(&stack0xffffffc4,0x3f800000,0x3f800000,0,0x3f400000);
  iVar15 = 100;
  iVar13 = 800;
  uVar6 = __ftol2();
  Graphics_DrawScreenRect(param_1,0,(int)uVar6,iVar13,iVar15);
  Matrix_Scale4x4(&stack0xffffffc4,0x3f800000,0x3f800000,0,0x3f400000);
  iVar15 = 10;
  iVar13 = 800;
  uVar6 = __ftol2();
  Graphics_DrawScreenRect(param_1,0,(int)uVar6,iVar13,iVar15);
  Matrix_Scale4x4(&stack0xffffffc4,0x3f800000,0x3f800000,0,0x3f400000);
  iVar15 = 100;
  iVar13 = 800;
  uVar6 = __ftol2();
  Graphics_DrawScreenRect(param_1,0,(int)uVar6,iVar13,iVar15);
  Matrix_Scale4x4(&stack0xffffffc4,0x3f800000,0x3f800000,0,0x3f400000);
  iVar15 = 10;
  iVar13 = 800;
  uVar6 = __ftol2();
  Graphics_DrawScreenRect(param_1,0,(int)uVar6,iVar13,iVar15);
  uVar16 = 0x3f800000;
  uVar14 = 0;
  uVar12 = 0;
  uVar11 = 0;
  uVar10 = 0x42ba11;
  Matrix_Scale4x4(&stack0xffffffc4,0,0,0,0x3f800000);
  local_4 = 0;
  Matrix_Scale4x4(&stack0xffffffb0,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  iVar15 = 2;
  local_4 = 0xffffffff;
  iVar13 = 2;
  uVar6 = __ftol2();
  UI_DrawTextCenteredAbsolute
            (*(void **)(*(int *)((int)this + 0xcdc) + 0x318),(byte *)"RADICAL RODENT ROLLERS!",400,
             (int)uVar6,iVar13,iVar15,uVar10,uVar11,uVar12,uVar14,uVar16,in_stack_ffffffc4,
             in_stack_ffffffc8,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4);
  iVar13 = 0x96;
  puVar4 = (undefined1 *)0x0;
  do {
    uVar3 = (uint)puVar4 & 0x80000001;
    bVar5 = uVar3 == 0;
    if ((int)uVar3 < 0) {
      bVar5 = (uVar3 - 1 | 0xfffffffe) == 0xffffffff;
    }
    if (bVar5) {
      uVar17 = 0x3f800000;
      uVar16 = 0;
      uVar14 = 0;
      uVar12 = 0;
      uVar11 = 0x42baaf;
      Matrix_Scale4x4(&stack0xffffffc4,0,0,0,0x3f800000);
      local_4 = 1;
      Matrix_Scale4x4(&stack0xffffffb0,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
      piVar18 = *(int **)(*(int *)((int)this + 0xcdc) + 0x230);
      uVar10 = 2;
      iVar8 = 2;
      local_4 = 0xffffffff;
      iVar15 = iVar13;
      uVar6 = __ftol2();
      iVar1 = (int)uVar6;
      puVar7 = puVar4;
      (**(code **)(*piVar18 + 0x18))();
      pbVar2 = (byte *)AthenaString_Format(0x4f7448,(byte *)"%d. %s");
      UI_DrawTextShadow_Wrapper
                (*(void **)(*(int *)((int)this + 0xcdc) + 0x318),pbVar2,(int)puVar7,iVar1,iVar15,
                 iVar8,uVar10,uVar11,uVar12,uVar14,uVar16,uVar17,in_stack_ffffffc4,in_stack_ffffffc8
                 ,in_stack_ffffffcc,in_stack_ffffffd0);
      uVar14 = 0x3f800000;
      uVar12 = 0;
      uVar11 = 0;
      uVar10 = 0;
      iVar15 = 0x42bb59;
      Matrix_Scale4x4(&stack0xffffffc0,0,0,0,0x3f800000);
      local_14 = &stack0xffffffac;
      puStack_8 = (undefined1 *)0x2;
      Matrix_Scale4x4(&stack0xffffffac,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
      piVar18 = *(int **)(*(int *)((int)this + 0xcdc) + 0x230);
    }
    else {
      uVar17 = 0x3f800000;
      uVar16 = 0;
      uVar14 = 0;
      uVar12 = 0;
      uVar11 = 0x42bbba;
      Matrix_Scale4x4(&stack0xffffffc4,0,0,0,0x3f800000);
      local_4 = 3;
      Matrix_Scale4x4(&stack0xffffffb0,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
      piVar18 = *(int **)(*(int *)((int)this + 0xcdc) + 0x230);
      uVar10 = 2;
      iVar8 = 2;
      local_4 = 0xffffffff;
      iVar15 = iVar13;
      uVar6 = __ftol2();
      iVar1 = (int)uVar6;
      puVar7 = puVar4;
      (**(code **)(*piVar18 + 0x18))();
      pbVar2 = (byte *)AthenaString_Format(0x4f7448,(byte *)"%d. %s");
      UI_DrawTextShadow_Wrapper
                (*(void **)(*(int *)((int)this + 0xcdc) + 0x318),pbVar2,(int)puVar7,iVar1,iVar15,
                 iVar8,uVar10,uVar11,uVar12,uVar14,uVar16,uVar17,in_stack_ffffffc4,in_stack_ffffffc8
                 ,in_stack_ffffffcc,in_stack_ffffffd0);
      local_14 = &stack0xffffffc0;
      uVar14 = 0x3f800000;
      uVar12 = 0;
      uVar11 = 0;
      uVar10 = 0;
      iVar15 = 0x42bc64;
      Matrix_Scale4x4(&stack0xffffffc0,0,0,0,0x3f800000);
      puStack_8 = (undefined1 *)0x4;
      Matrix_Scale4x4(&stack0xffffffac,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
      piVar18 = *(int **)(*(int *)((int)this + 0xcdc) + 0x230);
    }
    iVar9 = 2;
    iVar8 = 2;
    puStack_8 = (undefined1 *)0xffffffff;
    iVar1 = iVar13;
    uVar6 = __ftol2();
    (**(code **)(*piVar18 + 0x14))(puVar4,(int)uVar6);
    pbVar2 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
    UI_DrawTextCentered(*(void **)(*(int *)((int)this + 0xcdc) + 0x318),pbVar2,iVar1,iVar8,iVar9,
                        iVar15,uVar10,uVar11,uVar12,uVar14,uVar17,in_stack_ffffffc4,
                        in_stack_ffffffc8,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4);
    iVar13 = iVar13 + 0x41;
    puVar4 = local_14;
  } while ((int)local_14 < 5);
  UIList_Render(this,param_1);
  ExceptionList = pvStack_c;
  return;
}

