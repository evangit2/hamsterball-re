
int * __fastcall Font_RenderToTextureComplex(int param_1)

{
  int *piVar1;
  int *_Memory;
  undefined4 uVar2;
  int iVar3;
  uint uVar4;
  undefined4 *unaff_EBP;
  int *unaff_ESI;
  undefined4 *puVar5;
  undefined4 *puVar6;
  int *unaff_EDI;
  int *piVar7;
  int *piVar8;
  undefined4 uStack_7c;
  undefined4 uStack_78;
  int *piStack_74;
  int *piStack_70;
  int *piStack_6c;
  uint uVar9;
  int *piVar10;
  undefined4 *puVar11;
  int *local_40;
  undefined4 local_3c;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_004cd988;
  pvStack_c = ExceptionList;
  local_40 = (int *)0x0;
  if (*(char *)(*(int *)(param_1 + 0x2c) + 2000) == '\0') {
    uVar9 = 0x220;
  }
  else {
    uVar9 = 0x280;
  }
  piStack_6c = (int *)0x4723a6;
  ExceptionList = &pvStack_c;
  D3DTexture_CreateSimple
            ((int)*(uint *)(param_1 + 0x20) / 3,*(uint *)(param_1 + 0x20),uVar9,0x112,
             *(int **)(*(int *)(param_1 + 0x2c) + 0x154),&local_40);
  Sleep(10);
  puVar11 = (undefined4 *)0x2000;
  local_3c = 0;
  piVar10 = local_40;
  (**(code **)(*local_40 + 0x38))();
  puVar5 = *(undefined4 **)(param_1 + 0xc);
  for (uVar9 = (uint)(*(int *)(param_1 + 0x1c) << 5) >> 2; uVar9 != 0; uVar9 = uVar9 - 1) {
    *unaff_EBP = *puVar5;
    puVar5 = puVar5 + 1;
    unaff_EBP = unaff_EBP + 1;
  }
  for (iVar3 = 0; iVar3 != 0; iVar3 = iVar3 + -1) {
    *(undefined1 *)unaff_EBP = *(undefined1 *)puVar5;
    puVar5 = (undefined4 *)((int)puVar5 + 1);
    unaff_EBP = (undefined4 *)((int)unaff_EBP + 1);
  }
  (**(code **)(*unaff_ESI + 0x3c))();
  puVar5 = (undefined4 *)&stack0xffffffb4;
  piStack_70 = (int *)0x4723ff;
  piStack_6c = unaff_EDI;
  (**(code **)(*unaff_EDI + 0x40))();
  piStack_70 = piVar10;
  uVar4 = *(int *)(param_1 + 0x20) << 1;
  puVar6 = *(undefined4 **)(param_1 + 0x10);
  for (uVar9 = uVar4 >> 2; uVar9 != 0; uVar9 = uVar9 - 1) {
    *puVar11 = *puVar6;
    puVar6 = puVar6 + 1;
    puVar11 = puVar11 + 1;
  }
  for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
    *(undefined1 *)puVar11 = *(undefined1 *)puVar6;
    puVar6 = (undefined4 *)((int)puVar6 + 1);
    puVar11 = (undefined4 *)((int)puVar11 + 1);
  }
  piStack_74 = (int *)0x472423;
  (**(code **)(*piStack_70 + 0x44))();
  piStack_74 = (int *)&stack0xffffffa8;
  uStack_78 = 0x2000;
  (**(code **)(*unaff_ESI + 0x58))();
  piVar10 = piStack_6c;
  puVar11 = *(undefined4 **)(param_1 + 0x18);
  for (uVar9 = *(int *)(param_1 + 0x20) / 3 & 0x3fffffff; uVar9 != 0; uVar9 = uVar9 - 1) {
    *puVar5 = *puVar11;
    puVar11 = puVar11 + 1;
    puVar5 = puVar5 + 1;
  }
  for (iVar3 = 0; iVar3 != 0; iVar3 = iVar3 + -1) {
    *(undefined1 *)puVar5 = *(undefined1 *)puVar11;
    puVar11 = (undefined4 *)((int)puVar11 + 1);
    puVar5 = (undefined4 *)((int)puVar5 + 1);
  }
  (**(code **)(*piStack_6c + 0x5c))();
  piVar1 = piStack_70;
  iVar3 = (**(code **)(*piStack_70 + 0x10))();
  _Memory = operator_new(iVar3 * 0xc);
  piVar8 = _Memory;
  (**(code **)(*piStack_74 + 0x54))(piStack_74,0x3dcccccd);
  iVar3 = (**(code **)(*piVar10 + 0x10))();
  piStack_74 = (int *)0x0;
  uVar2 = AthenaVector_Init(&stack0xffffffa0,iVar3 * 3);
  if ((char)uVar2 != '\0') {
    Vector_FillResize(&stack0xffffffa0,iVar3 * 3,&piStack_74);
  }
  uVar9 = (**(code **)(*piVar1 + 0x10))(piVar1);
  uStack_78 = 0;
  uVar2 = AthenaVector_Init(&piStack_74,uVar9);
  if ((char)uVar2 != '\0') {
    Vector_FillResize(&piStack_74,uVar9,&uStack_78);
  }
  piVar1 = piStack_70;
  uStack_7c = 0;
  piVar7 = piStack_70;
  (**(code **)(*piVar8 + 100))(piVar8,0x4000000,_Memory,unaff_ESI,piStack_70,&uStack_7c);
  _free(_Memory);
  if (piVar10 != (int *)0x0) {
    (**(code **)(*piVar10 + 8))(piVar10);
  }
  if (piVar1 != (int *)0x0) {
    _free(piVar1);
  }
  if (unaff_ESI != (int *)0x0) {
    _free(unaff_ESI);
  }
  ExceptionList = piStack_6c;
  return piVar7;
}

