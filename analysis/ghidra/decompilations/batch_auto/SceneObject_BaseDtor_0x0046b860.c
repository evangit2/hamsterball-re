
void __fastcall SceneObject_BaseDtor(undefined4 *param_1)

{
  undefined4 *puVar1;
  int iVar2;
  int iVar3;
  undefined4 *puVar4;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cd5db;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *param_1 = &PTR_SceneObject_DeletingDtor_004d9368;
  puVar1 = param_1 + 1;
  local_4 = 0;
  iVar3 = AthenaList_NextIndex((int)puVar1);
  param_1[iVar3 + 3] = 0;
  if ((int)param_1[2] < 1) {
    puVar4 = (undefined4 *)0x0;
  }
  else {
    puVar4 = *(undefined4 **)param_1[0x104];
    param_1[iVar3 + 3] = 1;
  }
  while (puVar4 != (undefined4 *)0x0) {
    (**(code **)*puVar4)(1);
    iVar2 = param_1[iVar3 + 3];
    if ((int)param_1[2] <= iVar2) break;
    puVar4 = *(undefined4 **)(param_1[0x104] + iVar2 * 4);
    param_1[iVar3 + 3] = iVar2 + 1;
  }
  AthenaList_Free((int)puVar1);
  local_4 = 0xffffffff;
  Vec3List_Free(puVar1);
  ExceptionList = local_c;
  return;
}

