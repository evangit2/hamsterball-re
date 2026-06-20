
void __fastcall SoundDevice_dtor(undefined4 *param_1)

{
  undefined4 *puVar1;
  int iVar2;
  int *piVar3;
  int iVar4;
  undefined4 *puVar5;
  void *_Memory;
  void *pvStack_c;
  undefined1 *puStack_8;
  uint local_4;
  
  puStack_8 = &LAB_004cd2f9;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_SoundDevice_DeletingDtor_004d911c;
  local_4 = 1;
  RegKey_Open(*(int *)(param_1[0x20d] + 0x54));
  RegKey_WriteDWORD(*(void **)(param_1[0x20d] + 0x54),"Sound Volume",param_1[0x20e]);
  RegKey_Close(*(int *)(param_1[0x20d] + 0x54));
  puVar1 = param_1 + 1;
  iVar4 = AthenaList_NextIndex((int)puVar1);
  param_1[iVar4 + 3] = 0;
  if ((int)param_1[2] < 1) {
    puVar5 = (undefined4 *)0x0;
  }
  else {
    puVar5 = *(undefined4 **)param_1[0x104];
    param_1[iVar4 + 3] = 1;
  }
  while (puVar5 != (undefined4 *)0x0) {
    (**(code **)*puVar5)(1);
    iVar2 = param_1[iVar4 + 3];
    if ((int)param_1[2] <= iVar2) break;
    puVar5 = *(undefined4 **)(param_1[0x104] + iVar2 * 4);
    param_1[iVar4 + 3] = iVar2 + 1;
  }
  AthenaList_Free((int)puVar1);
  puVar5 = param_1 + 0x107;
  iVar4 = AthenaList_NextIndex((int)puVar5);
  param_1[iVar4 + 0x109] = 0;
  if ((int)param_1[0x108] < 1) {
    _Memory = (void *)0x0;
  }
  else {
    _Memory = *(void **)param_1[0x20a];
    param_1[iVar4 + 0x109] = 1;
  }
  while (_Memory != (void *)0x0) {
    _free(_Memory);
    iVar2 = param_1[iVar4 + 0x109];
    if ((int)param_1[0x108] <= iVar2) break;
    _Memory = *(void **)(param_1[0x20a] + iVar2 * 4);
    param_1[iVar4 + 0x109] = iVar2 + 1;
  }
  AthenaList_Free((int)puVar5);
  piVar3 = (int *)param_1[0x213];
  if (piVar3 != (int *)0x0) {
    (**(code **)(*piVar3 + 8))(piVar3);
    param_1[0x213] = 0;
  }
  local_4 = local_4 & 0xffffff00;
  Vec3List_Free(puVar5);
  local_4 = 0xffffffff;
  Vec3List_Free(puVar1);
  ExceptionList = pvStack_c;
  return;
}

