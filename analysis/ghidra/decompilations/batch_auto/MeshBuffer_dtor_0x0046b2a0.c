
void __fastcall MeshBuffer_dtor(undefined4 *param_1)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cd53b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *param_1 = &PTR_MeshBuffer_DeletingDtor_004d9348;
  local_4 = 0;
  if (*(char *)(param_1 + 0x108) == '\0') {
    iVar2 = AthenaList_NextIndex((int)(param_1 + 1));
    param_1[iVar2 + 3] = 0;
    if ((int)param_1[2] < 1) {
      puVar3 = (undefined4 *)0x0;
    }
    else {
      puVar3 = *(undefined4 **)param_1[0x104];
      param_1[iVar2 + 3] = 1;
    }
    while (puVar3 != (undefined4 *)0x0) {
      (**(code **)*puVar3)(1);
      iVar1 = param_1[iVar2 + 3];
      if ((int)param_1[2] <= iVar1) break;
      puVar3 = *(undefined4 **)(param_1[0x104] + iVar1 * 4);
      param_1[iVar2 + 3] = iVar1 + 1;
    }
  }
  AthenaList_Free((int)(param_1 + 1));
  local_4 = 0xffffffff;
  Vec3List_Free(param_1 + 1);
  ExceptionList = local_c;
  return;
}

