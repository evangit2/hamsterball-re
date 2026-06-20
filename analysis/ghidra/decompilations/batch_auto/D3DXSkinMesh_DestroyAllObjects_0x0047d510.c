
void __fastcall D3DXSkinMesh_DestroyAllObjects(int param_1)

{
  int iVar1;
  undefined4 *puVar2;
  
  *(undefined4 *)(param_1 + 0x3c) = 0;
  if (*(int *)(param_1 + 0x38) < 1) {
    puVar2 = (undefined4 *)0x0;
  }
  else {
    puVar2 = (undefined4 *)**(undefined4 **)(param_1 + 0x440);
    *(undefined4 *)(param_1 + 0x3c) = 1;
  }
  while (puVar2 != (undefined4 *)0x0) {
    (**(code **)*puVar2)(1);
    iVar1 = *(int *)(param_1 + 0x3c);
    if (*(int *)(param_1 + 0x38) <= iVar1) break;
    puVar2 = *(undefined4 **)(*(int *)(param_1 + 0x440) + iVar1 * 4);
    *(int *)(param_1 + 0x3c) = iVar1 + 1;
  }
  AthenaList_Free(param_1 + 0x34);
  return;
}

