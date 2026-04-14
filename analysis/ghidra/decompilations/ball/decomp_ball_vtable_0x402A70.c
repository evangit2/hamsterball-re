
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00402a70(int param_1)

{
  int iVar1;
  int iVar2;
  ulonglong uVar3;
  int iVar4;
  int iVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  float *pfVar12;
  undefined4 uVar13;
  undefined4 uVar14;
  undefined4 uVar15;
  float afStack_18 [3];
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_004c9248;
  pvStack_c = ExceptionList;
  iVar1 = *(int *)(*(int *)(*(int *)(param_1 + 0x14) + 0x878) + 0x220);
  if (((*(char *)(iVar1 + 0x10) == '\0') && (*(char *)(iVar1 + 0x11) == '\0')) &&
     (*(int *)(param_1 + 0xc28) != 0)) {
    uVar15 = *(undefined4 *)(param_1 + 0x16c);
    uVar14 = *(undefined4 *)(param_1 + 0x168);
    uVar13 = *(undefined4 *)(param_1 + 0x164);
    pfVar12 = afStack_18;
    uVar11 = 0x402afb;
    ExceptionList = &pvStack_c;
    FUN_00454b50(*(void **)(*(int *)(*(int *)(param_1 + 0x14) + 0x878) + 0x174),pfVar12,uVar13,
                 uVar14);
    if (*(char *)(*(int *)(*(int *)(*(int *)(param_1 + 0x14) + 0x878) + 0x220) + 0x12) != '\0') {
      afStack_18[0] = afStack_18[0] * _DAT_004cf3f0;
    }
    uVar10 = 0x3f800000;
    uVar9 = 0;
    uVar8 = 0;
    uVar7 = 0;
    uVar6 = 0x402b38;
    FUN_00453150(&stack0xffffffc4,0,0,0,0x3f800000);
    uStack_4 = 0;
    FUN_00453150(&stack0xffffffb0,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    iVar1 = *(int *)(param_1 + 0x10);
    iVar5 = 3;
    uStack_4 = 0xffffffff;
    iVar4 = 3;
    uVar3 = FUN_004ba754();
    iVar2 = (int)uVar3;
    uVar3 = FUN_004ba754();
    FUN_004013a0(*(void **)(iVar1 + 0x31c),*(byte **)(param_1 + 0xc28),(int)uVar3,iVar2,iVar4,iVar5,
                 uVar6,uVar7,uVar8,uVar9,uVar10,uVar11,pfVar12,uVar13,uVar14,uVar15);
  }
  ExceptionList = pvStack_c;
  return;
}
