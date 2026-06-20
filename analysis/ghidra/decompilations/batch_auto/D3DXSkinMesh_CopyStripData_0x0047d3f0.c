
void __fastcall D3DXSkinMesh_CopyStripData(int param_1)

{
  int iVar1;
  int iVar2;
  void *pvVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  
  iVar2 = AthenaList_GetSize(param_1 + 0x450);
  pvVar3 = operator_new(iVar2 << 5);
  *(void **)(*(int *)(param_1 + 0x44c) + 0x10) = pvVar3;
  *(undefined4 *)(param_1 + 0x458) = 0;
  if (*(int *)(param_1 + 0x454) < 1) {
    puVar5 = (undefined4 *)0x0;
  }
  else {
    puVar5 = (undefined4 *)**(undefined4 **)(param_1 + 0x85c);
    *(undefined4 *)(param_1 + 0x458) = 1;
  }
  if (puVar5 != (undefined4 *)0x0) {
    iVar2 = 0;
    do {
      puVar4 = (undefined4 *)(*(int *)(*(int *)(param_1 + 0x44c) + 0x10) + iVar2);
      *puVar4 = *puVar5;
      puVar4[1] = puVar5[1];
      puVar4[2] = puVar5[2];
      puVar4[3] = puVar5[3];
      puVar4[4] = puVar5[4];
      puVar4[5] = puVar5[5];
      puVar4[6] = puVar5[6];
      puVar4[7] = puVar5[7];
      iVar1 = *(int *)(param_1 + 0x458);
      iVar2 = iVar2 + 0x20;
      if (*(int *)(param_1 + 0x454) <= iVar1) break;
      puVar5 = *(undefined4 **)(*(int *)(param_1 + 0x85c) + iVar1 * 4);
      *(int *)(param_1 + 0x458) = iVar1 + 1;
    } while (puVar5 != (undefined4 *)0x0);
  }
  AthenaList_Append((void *)(param_1 + 0x34),*(int *)(param_1 + 0x44c));
  return;
}

