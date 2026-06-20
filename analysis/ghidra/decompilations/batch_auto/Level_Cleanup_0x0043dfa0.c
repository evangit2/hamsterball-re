
void __fastcall Level_Cleanup(undefined4 *param_1)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  int iVar4;
  undefined4 *puVar5;
  void *pvVar6;
  void *pvStack_c;
  undefined1 *puStack_8;
  int iStack_4;
  
  pvStack_c = ExceptionList;
  puStack_8 = &LAB_004cd0db;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_Level_DeletingDtor_004d8fb0;
  iStack_4 = 5;
  param_1[8] = 0;
  if ((int)param_1[7] < 1) {
    puVar5 = (undefined4 *)0x0;
  }
  else {
    puVar5 = *(undefined4 **)param_1[0x109];
    param_1[8] = 1;
  }
  while (puVar5 != (undefined4 *)0x0) {
    (**(code **)*puVar5)(1);
    iVar1 = param_1[8];
    if ((int)param_1[7] <= iVar1) break;
    puVar5 = *(undefined4 **)(param_1[0x109] + iVar1 * 4);
    param_1[8] = iVar1 + 1;
  }
  AthenaList_Free((int)(param_1 + 6));
  if (*(char *)(param_1 + 0x121) == '\0') {
    _free(*(void **)(param_1[0x120] + 0x440));
    *(undefined4 *)(param_1[0x120] + 0x440) = 0;
    _free(*(void **)(param_1[0x120] + 0x10c8));
    *(undefined4 *)(param_1[0x120] + 0x10c8) = 0;
    iVar4 = AthenaList_NextIndex(param_1[0x120] + 0x478);
    iVar1 = param_1[0x120];
    *(undefined4 *)(iVar1 + 0x480 + iVar4 * 4) = 0;
    if (*(int *)(iVar1 + 0x47c) < 1) {
      puVar5 = (undefined4 *)0x0;
    }
    else {
      puVar5 = (undefined4 *)**(undefined4 **)(iVar1 + 0x884);
      *(undefined4 *)(iVar1 + 0x480 + iVar4 * 4) = 1;
    }
    while (puVar5 != (undefined4 *)0x0) {
      (**(code **)*puVar5)(1);
      iVar1 = param_1[0x120];
      iVar2 = *(int *)(iVar1 + 0x480 + iVar4 * 4);
      if (*(int *)(iVar1 + 0x47c) <= iVar2) break;
      puVar5 = *(undefined4 **)(*(int *)(iVar1 + 0x884) + iVar2 * 4);
      *(int *)(iVar1 + 0x480 + iVar4 * 4) = iVar2 + 1;
    }
    AthenaList_Free(param_1[0x120] + 0x478);
    iVar4 = AthenaList_NextIndex(param_1[0x120] + 0x894);
    iVar1 = param_1[0x120];
    *(undefined4 *)(iVar1 + 0x89c + iVar4 * 4) = 0;
    if (*(int *)(iVar1 + 0x898) < 1) {
      puVar5 = (undefined4 *)0x0;
    }
    else {
      puVar5 = (undefined4 *)**(undefined4 **)(iVar1 + 0xca0);
      *(undefined4 *)(iVar1 + 0x89c + iVar4 * 4) = 1;
    }
    while (puVar5 != (undefined4 *)0x0) {
      _free((void *)*puVar5);
      *puVar5 = 0;
      Matrix4_Identity(puVar5 + 0xb);
      _free(puVar5);
      iVar1 = param_1[0x120];
      iVar2 = *(int *)(iVar1 + 0x89c + iVar4 * 4);
      if (*(int *)(iVar1 + 0x898) <= iVar2) break;
      puVar5 = *(undefined4 **)(*(int *)(iVar1 + 0xca0) + iVar2 * 4);
      *(int *)(iVar1 + 0x89c + iVar4 * 4) = iVar2 + 1;
    }
    AthenaList_Free(param_1[0x120] + 0x894);
    piVar3 = *(int **)(param_1[0x120] + 0x444);
    if (piVar3 != (int *)0x0) {
      (**(code **)(*piVar3 + 8))(piVar3);
      *(undefined4 *)(param_1[0x120] + 0x444) = 0;
    }
    piVar3 = *(int **)(param_1[0x120] + 0x448);
    if (piVar3 != (int *)0x0) {
      (**(code **)(*piVar3 + 8))(piVar3);
      *(undefined4 *)(param_1[0x120] + 0x448) = 0;
    }
    piVar3 = *(int **)(param_1[0x120] + 0x44c);
    if (piVar3 != (int *)0x0) {
      (**(code **)(*piVar3 + 8))(piVar3);
      *(undefined4 *)(param_1[0x120] + 0x44c) = 0;
    }
    iVar4 = AthenaList_NextIndex(param_1[0x120] + 0xcac);
    iVar1 = param_1[0x120];
    *(undefined4 *)(iVar1 + 0xcb4 + iVar4 * 4) = 0;
    if (*(int *)(iVar1 + 0xcb0) < 1) {
      pvVar6 = (void *)0x0;
    }
    else {
      pvVar6 = (void *)**(undefined4 **)(iVar1 + 0x10b8);
      *(undefined4 *)(iVar1 + 0xcb4 + iVar4 * 4) = 1;
    }
    while (pvVar6 != (void *)0x0) {
      MultiBuffer_Free((int)pvVar6);
      _free(pvVar6);
      iVar1 = param_1[0x120];
      iVar2 = *(int *)(iVar1 + 0xcb4 + iVar4 * 4);
      if (*(int *)(iVar1 + 0xcb0) <= iVar2) break;
      pvVar6 = *(void **)(*(int *)(iVar1 + 0x10b8) + iVar2 * 4);
      *(int *)(iVar1 + 0xcb4 + iVar4 * 4) = iVar2 + 1;
    }
    AthenaList_Free(param_1[0x120] + 0xcac);
    pvVar6 = (void *)param_1[0x120];
    if (pvVar6 != (void *)0x0) {
      SceneObject_BaseClear((int)pvVar6);
      _free(pvVar6);
    }
  }
  if ((*(char *)((int)param_1 + 0x431) != '\0') &&
     ((undefined4 *)param_1[0x10d] != (undefined4 *)0x0)) {
    (*(code *)**(undefined4 **)param_1[0x10d])(1);
  }
  param_1[0x10d] = 0;
  iStack_4._0_1_ = 4;
  Vec3List_Free(param_1 + 0x32e);
  iStack_4._0_1_ = 3;
  Vec3List_Free(param_1 + 0x228);
  iStack_4._0_1_ = 2;
  Vec3List_Free(param_1 + 0x122);
  iStack_4._0_1_ = 1;
  Timer_Cleanup(param_1 + 0x10e);
  iStack_4 = (uint)iStack_4._1_3_ << 8;
  Vec3List_Free(param_1 + 6);
  iStack_4 = 0xffffffff;
  MeshBuffer_Cleanup(param_1);
  ExceptionList = pvStack_c;
  return;
}

