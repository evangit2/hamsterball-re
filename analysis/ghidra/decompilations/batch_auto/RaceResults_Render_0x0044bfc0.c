
void __fastcall RaceResults_Render(int param_1)

{
  int iVar1;
  byte *pbVar2;
  ulonglong uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined **ppuVar12;
  undefined4 uVar13;
  undefined4 uVar14;
  undefined4 uVar15;
  undefined4 uVar16;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cc5d8;
  local_c = ExceptionList;
  iVar1 = *(int *)(param_1 + 4);
  ppuVar12 = &PTR_Vec3_dtor_004cf300;
  uVar13 = *(undefined4 *)(iVar1 + 0x1508);
  uVar14 = *(undefined4 *)(iVar1 + 0x150c);
  uVar15 = *(undefined4 *)(iVar1 + 0x1510);
  uVar16 = *(undefined4 *)(iVar1 + 0x1514);
  ExceptionList = &local_c;
  Gfx_DrawQuadRandomColor
            (*(void **)(*(int *)(param_1 + 0xc) + 0x390),&PTR_LAB_004cf584,
             *(float *)(param_1 + 0x20),*(float *)(param_1 + 0x24),180.0,125.0,&PTR_LAB_004cf584,
             0x3f800000,0,0,0x3f800000);
  uVar11 = 0x3f800000;
  uVar10 = 0;
  uVar9 = 0;
  uVar8 = 0;
  uVar7 = 0x44c08e;
  Matrix_Scale4x4(&stack0xffffffd4,0,0,0,0x3f800000);
  local_4 = 0;
  Matrix_Scale4x4(&stack0xffffffc0,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  iVar5 = 5;
  local_4 = 0xffffffff;
  iVar4 = 5;
  uVar3 = __ftol2();
  iVar1 = (int)uVar3;
  uVar3 = __ftol2();
  UI_DrawTextShadow(*(void **)(*(int *)(param_1 + 0xc) + 0x318),*(byte **)(param_1 + 0x2c),
                    (int)uVar3,iVar1,iVar4,iVar5,uVar7,uVar8,uVar9,uVar10,uVar11,ppuVar12,uVar13,
                    uVar14,uVar15,uVar16);
  uVar11 = 0x3f800000;
  uVar10 = 0;
  uVar9 = 0;
  uVar8 = 0;
  uVar7 = 0x44c10d;
  Matrix_Scale4x4(&stack0xffffffd4,0,0,0,0x3f800000);
  local_4 = 1;
  Matrix_Scale4x4(&stack0xffffffc0,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  iVar6 = 3;
  local_4 = 0xffffffff;
  iVar5 = 3;
  uVar3 = __ftol2();
  iVar1 = (int)uVar3;
  uVar3 = __ftol2();
  iVar4 = (int)uVar3;
  pbVar2 = (byte *)AthenaString_Format(0x4f7448,&DAT_004cf500);
  UI_DrawTextCentered(*(void **)(*(int *)(param_1 + 0xc) + 0x318),pbVar2,iVar4,iVar1,iVar5,iVar6,
                      uVar7,uVar8,uVar9,uVar10,uVar11,ppuVar12,uVar13,uVar14,uVar15,uVar16);
  uVar11 = 0x3f800000;
  uVar10 = 0;
  uVar9 = 0;
  uVar8 = 0;
  uVar7 = 0x44c1b3;
  Matrix_Scale4x4(&stack0xffffffd4,0,0,0,0x3f800000);
  local_4 = 2;
  Matrix_Scale4x4(&stack0xffffffc0,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  iVar6 = 3;
  local_4 = 0xffffffff;
  iVar5 = 3;
  uVar3 = __ftol2();
  iVar1 = (int)uVar3;
  uVar3 = __ftol2();
  iVar4 = (int)uVar3;
  pbVar2 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d6c7c);
  UI_DrawTextShadow_Wrapper
            (*(void **)(*(int *)(param_1 + 0xc) + 0x31c),pbVar2,iVar4,iVar1,iVar5,iVar6,uVar7,uVar8,
             uVar9,uVar10,uVar11,ppuVar12,uVar13,uVar14,uVar15,uVar16);
  ExceptionList = local_c;
  return;
}

