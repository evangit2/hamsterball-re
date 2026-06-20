
void __fastcall SceneObject_dtor(undefined4 *param_1)

{
  undefined4 *puVar1;
  int iVar2;
  int *piVar3;
  int iVar4;
  undefined4 *puVar5;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cd77b;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_SceneObject_DeletingDtor_004d9bfc;
  puVar1 = param_1 + 3;
  local_4 = 0;
  iVar4 = AthenaList_NextIndex((int)puVar1);
  param_1[iVar4 + 5] = 0;
  if ((int)param_1[4] < 1) {
    puVar5 = (undefined4 *)0x0;
  }
  else {
    puVar5 = *(undefined4 **)param_1[0x106];
    param_1[iVar4 + 5] = 1;
  }
  while (puVar5 != (undefined4 *)0x0) {
    (**(code **)*puVar5)(1);
    iVar2 = param_1[iVar4 + 5];
    if ((int)param_1[4] <= iVar2) break;
    puVar5 = *(undefined4 **)(param_1[0x106] + iVar2 * 4);
    param_1[iVar4 + 5] = iVar2 + 1;
  }
  AthenaList_Free((int)puVar1);
  if ((undefined4 *)param_1[0x10d] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x10d])(1);
    param_1[0x10d] = 0;
  }
  piVar3 = (int *)param_1[2];
  if (piVar3 != (int *)0x0) {
    (**(code **)(*piVar3 + 8))(piVar3);
    param_1[2] = 0;
  }
  local_4 = 0xffffffff;
  Vec3List_Free(puVar1);
  ExceptionList = pvStack_c;
  return;
}

