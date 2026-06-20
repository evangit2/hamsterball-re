
void __fastcall Scene_Destroy(undefined4 *param_1)

{
  int *piVar1;
  int iVar2;
  void *_Memory;
  undefined4 *puVar3;
  void *pvStack_c;
  undefined1 *puStack_8;
  uint local_4;
  
  puStack_8 = &LAB_004ccdd9;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_Scene_ScalarDtor_004d8e70;
  piVar1 = (int *)param_1[0x213];
  local_4 = 1;
  if (piVar1 != (int *)0x0) {
    (**(code **)(*piVar1 + 8))(piVar1);
    param_1[0x213] = 0;
  }
  piVar1 = (int *)param_1[0x214];
  if (piVar1 != (int *)0x0) {
    (**(code **)(*piVar1 + 8))(piVar1);
    param_1[0x214] = 0;
  }
  piVar1 = (int *)param_1[0x215];
  if (piVar1 != (int *)0x0) {
    (**(code **)(*piVar1 + 8))(piVar1);
    param_1[0x215] = 0;
  }
  param_1[5] = 0;
  if ((int)param_1[4] < 1) {
    _Memory = (void *)0x0;
  }
  else {
    _Memory = *(void **)param_1[0x106];
    param_1[5] = 1;
  }
  while (_Memory != (void *)0x0) {
    _free(_Memory);
    iVar2 = param_1[5];
    if ((int)param_1[4] <= iVar2) break;
    _Memory = *(void **)(param_1[0x106] + iVar2 * 4);
    param_1[5] = iVar2 + 1;
  }
  AthenaList_Free((int)(param_1 + 3));
  param_1[0x10b] = 0;
  if ((int)param_1[0x10a] < 1) {
    puVar3 = (undefined4 *)0x0;
  }
  else {
    puVar3 = *(undefined4 **)param_1[0x20c];
    param_1[0x10b] = 1;
  }
  while (puVar3 != (undefined4 *)0x0) {
    (**(code **)*puVar3)(1);
    iVar2 = param_1[0x10b];
    if ((int)param_1[0x10a] <= iVar2) break;
    puVar3 = *(undefined4 **)(param_1[0x20c] + iVar2 * 4);
    param_1[0x10b] = iVar2 + 1;
  }
  AthenaList_Free((int)(param_1 + 0x109));
  if ((void *)param_1[0x219] != (void *)0x0) {
    _free((void *)param_1[0x219]);
    param_1[0x219] = 0;
  }
  local_4 = local_4 & 0xffffff00;
  Vec3List_Free(param_1 + 0x109);
  local_4 = 0xffffffff;
  Vec3List_Free(param_1 + 3);
  ExceptionList = pvStack_c;
  return;
}

