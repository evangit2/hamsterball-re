
void __fastcall MenuTag_dtor(undefined4 *param_1)

{
  undefined4 *puVar1;
  int iVar2;
  int iVar3;
  void *_Memory;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cdd0b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *param_1 = &PTR_MenuTag_DeletingDtor_004da170;
  puVar1 = param_1 + 2;
  local_4 = 0;
  iVar3 = AthenaList_NextIndex((int)puVar1);
  param_1[iVar3 + 4] = 0;
  if ((int)param_1[3] < 1) {
    _Memory = (void *)0x0;
  }
  else {
    _Memory = *(void **)param_1[0x105];
    param_1[iVar3 + 4] = 1;
  }
  while (_Memory != (void *)0x0) {
    _free(_Memory);
    iVar2 = param_1[iVar3 + 4];
    if ((int)param_1[3] <= iVar2) break;
    _Memory = *(void **)(param_1[0x105] + iVar2 * 4);
    param_1[iVar3 + 4] = iVar2 + 1;
  }
  AthenaList_Free((int)puVar1);
  local_4 = 0xffffffff;
  Vec3List_Free(puVar1);
  ExceptionList = local_c;
  return;
}

