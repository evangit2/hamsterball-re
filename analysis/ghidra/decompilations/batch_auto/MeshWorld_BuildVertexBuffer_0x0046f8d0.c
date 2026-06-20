
void __fastcall MeshWorld_BuildVertexBuffer(int param_1)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  void *pvVar5;
  int iVar6;
  uint uVar7;
  int iVar8;
  int iVar9;
  undefined4 *puVar10;
  undefined4 *puVar11;
  undefined4 unaff_EDI;
  undefined4 *puVar12;
  int local_10;
  
  *(undefined1 *)(param_1 + 0x459) = 1;
  iVar3 = AthenaList_NextIndex(param_1 + 0x2c);
  *(undefined4 *)(param_1 + 0x34 + iVar3 * 4) = 0;
  if (*(int *)(param_1 + 0x30) < 1) {
    iVar8 = 0;
  }
  else {
    iVar8 = **(int **)(param_1 + 0x438);
    *(undefined4 *)(param_1 + 0x34 + iVar3 * 4) = 1;
  }
  while (iVar8 != 0) {
    if (*(char *)(iVar8 + 0x85c) == '\0') {
      *(undefined1 *)(param_1 + 0x459) = 0;
    }
    iVar4 = *(int *)(param_1 + 0x34 + iVar3 * 4);
    if (*(int *)(param_1 + 0x30) <= iVar4) break;
    iVar8 = *(int *)(*(int *)(param_1 + 0x438) + iVar4 * 4);
    *(int *)(param_1 + 0x34 + iVar3 * 4) = iVar4 + 1;
  }
  if (*(char *)(param_1 + 0x459) != '\x01') {
    return;
  }
  *(undefined4 *)(param_1 + 0x444) = 0;
  iVar3 = AthenaList_NextIndex(param_1 + 0x2c);
  *(undefined4 *)(param_1 + 0x34 + iVar3 * 4) = 0;
  if (*(int *)(param_1 + 0x30) < 1) {
    iVar8 = 0;
  }
  else {
    iVar8 = **(int **)(param_1 + 0x438);
    *(undefined4 *)(param_1 + 0x34 + iVar3 * 4) = 1;
  }
  while (iVar8 != 0) {
    iVar4 = AthenaList_NextIndex(iVar8 + 0x424);
    *(undefined4 *)(iVar8 + 0x42c + iVar4 * 4) = 0;
    if (*(int *)(iVar8 + 0x428) < 1) {
      iVar9 = 0;
    }
    else {
      iVar9 = **(int **)(iVar8 + 0x830);
      *(undefined4 *)(iVar8 + 0x42c + iVar4 * 4) = 1;
    }
    while (iVar9 != 0) {
      *(int *)(param_1 + 0x444) = *(int *)(param_1 + 0x444) + *(int *)(iVar9 + 4) + 2;
      iVar6 = *(int *)(iVar8 + 0x42c + iVar4 * 4);
      if (*(int *)(iVar8 + 0x428) <= iVar6) break;
      iVar9 = *(int *)(*(int *)(iVar8 + 0x830) + iVar6 * 4);
      *(int *)(iVar8 + 0x42c + iVar4 * 4) = iVar6 + 1;
    }
    iVar4 = *(int *)(param_1 + 0x34 + iVar3 * 4);
    if (*(int *)(param_1 + 0x30) <= iVar4) break;
    iVar8 = *(int *)(*(int *)(param_1 + 0x438) + iVar4 * 4);
    *(int *)(param_1 + 0x34 + iVar3 * 4) = iVar4 + 1;
  }
  pvVar5 = operator_new(*(int *)(param_1 + 0x444) << 5);
  iVar3 = 0;
  *(void **)(param_1 + 0x448) = pvVar5;
  local_10 = 0;
  iVar8 = AthenaList_NextIndex(param_1 + 0x2c);
  *(undefined4 *)(param_1 + 0x34 + iVar8 * 4) = 0;
  if (*(int *)(param_1 + 0x30) < 1) {
    iVar4 = 0;
  }
  else {
    iVar4 = **(int **)(param_1 + 0x438);
    *(undefined4 *)(param_1 + 0x34 + iVar8 * 4) = 1;
  }
  while (iVar4 != 0) {
    iVar6 = AthenaList_NextIndex(iVar4 + 0x424);
    iVar9 = 0;
    *(undefined4 *)(iVar4 + 0x42c + iVar6 * 4) = 0;
    if (0 < *(int *)(iVar4 + 0x428)) {
      iVar9 = **(int **)(iVar4 + 0x830);
      *(undefined4 *)(iVar4 + 0x42c + iVar6 * 4) = 1;
    }
    while (iVar9 != 0) {
      *(int *)(iVar9 + 8) = iVar3;
      puVar11 = *(undefined4 **)(iVar9 + 0x10);
      puVar12 = (undefined4 *)(iVar3 * 0x20 + *(int *)(param_1 + 0x448));
      for (uVar7 = (uint)((*(int *)(iVar9 + 4) + 2) * 0x20) >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
        *puVar12 = *puVar11;
        puVar11 = puVar11 + 1;
        puVar12 = puVar12 + 1;
      }
      for (iVar3 = 0; iVar3 != 0; iVar3 = iVar3 + -1) {
        *(undefined1 *)puVar12 = *(undefined1 *)puVar11;
        puVar11 = (undefined4 *)((int)puVar11 + 1);
        puVar12 = (undefined4 *)((int)puVar12 + 1);
      }
      iVar3 = local_10 + 2 + *(int *)(iVar9 + 4);
      _free(*(void **)(iVar9 + 0x10));
      *(undefined4 *)(iVar9 + 0x10) = 0;
      iVar2 = *(int *)(iVar4 + 0x42c + iVar6 * 4);
      local_10 = iVar3;
      if (*(int *)(iVar4 + 0x428) <= iVar2) break;
      iVar9 = *(int *)(*(int *)(iVar4 + 0x830) + iVar2 * 4);
      *(int *)(iVar4 + 0x42c + iVar6 * 4) = iVar2 + 1;
    }
    iVar9 = *(int *)(param_1 + 0x34 + iVar8 * 4);
    if (*(int *)(param_1 + 0x30) <= iVar9) break;
    iVar4 = *(int *)(*(int *)(param_1 + 0x438) + iVar9 * 4);
    *(int *)(param_1 + 0x34 + iVar8 * 4) = iVar9 + 1;
  }
  piVar1 = *(int **)(*(int *)(param_1 + 4) + 0x154);
  puVar11 = (undefined4 *)(param_1 + 0x44c);
  iVar3 = (**(code **)(*piVar1 + 0x5c))(piVar1,*(int *)(param_1 + 0x444) << 5);
  if ((iVar3 < 0) &&
     (piVar1 = *(int **)(*(int *)(param_1 + 4) + 0x154),
     iVar3 = (**(code **)(*piVar1 + 0x5c))
                       (piVar1,*(int *)(param_1 + 0x444) << 5,0x200,0x112,1,puVar11), iVar3 < 0)) {
    *(undefined1 *)(param_1 + 0x459) = 0;
    *puVar11 = 0;
    return;
  }
  puVar12 = (undefined4 *)&stack0xffffffe4;
  (**(code **)(*(int *)*puVar11 + 0x2c))((int *)*puVar11,0,0,puVar12,0,unaff_EDI);
  puVar10 = *(undefined4 **)(param_1 + 0x448);
  for (uVar7 = (uint)(*(int *)(param_1 + 0x444) << 5) >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
    *puVar12 = *puVar10;
    puVar10 = puVar10 + 1;
    puVar12 = puVar12 + 1;
  }
  for (iVar3 = 0; iVar3 != 0; iVar3 = iVar3 + -1) {
    *(undefined1 *)puVar12 = *(undefined1 *)puVar10;
    puVar10 = (undefined4 *)((int)puVar10 + 1);
    puVar12 = (undefined4 *)((int)puVar12 + 1);
  }
  (**(code **)(*(int *)*puVar11 + 0x30))((int *)*puVar11);
  return;
}

