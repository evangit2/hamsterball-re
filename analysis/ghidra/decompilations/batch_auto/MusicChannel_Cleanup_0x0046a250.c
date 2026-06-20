
void __fastcall MusicChannel_Cleanup(undefined4 *param_1)

{
  undefined4 *puVar1;
  int iVar2;
  int iVar3;
  undefined4 *_Memory;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cd48b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *param_1 = &PTR_MusicChannel_DeletingDtor_004d91d8;
  puVar1 = param_1 + 3;
  local_4 = 0;
  iVar3 = AthenaList_NextIndex((int)puVar1);
  param_1[iVar3 + 5] = 0;
  if ((int)param_1[4] < 1) {
    _Memory = (undefined4 *)0x0;
  }
  else {
    _Memory = *(undefined4 **)param_1[0x106];
    param_1[iVar3 + 5] = 1;
  }
  while (_Memory != (undefined4 *)0x0) {
    _free((void *)*_Memory);
    _free(_Memory);
    iVar2 = param_1[iVar3 + 5];
    if ((int)param_1[4] <= iVar2) break;
    _Memory = *(undefined4 **)(param_1[0x106] + iVar2 * 4);
    param_1[iVar3 + 5] = iVar2 + 1;
  }
  AthenaList_Free((int)puVar1);
  local_4 = 0xffffffff;
  Vec3List_Free(puVar1);
  ExceptionList = local_c;
  return;
}

