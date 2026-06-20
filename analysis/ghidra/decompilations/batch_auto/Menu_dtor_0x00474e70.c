
void __fastcall Menu_dtor(undefined4 *param_1)

{
  undefined4 *puVar1;
  int iVar2;
  int iVar3;
  void *_Memory;
  void *local_c;
  undefined1 *puStack_8;
  int local_4;
  
  puStack_8 = &LAB_004cdcf6;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *param_1 = &PTR_Menu_DeletingDtor_004da0a0;
  puVar1 = param_1 + 0x21f;
  local_4 = 9;
  iVar3 = AthenaList_NextIndex((int)puVar1);
  param_1[iVar3 + 0x221] = 0;
  if ((int)param_1[0x220] < 1) {
    _Memory = (void *)0x0;
  }
  else {
    _Memory = *(void **)param_1[0x322];
    param_1[iVar3 + 0x221] = 1;
  }
  while (_Memory != (void *)0x0) {
    if (0xf < *(uint *)((int)_Memory + 0x34)) {
      _free(*(void **)((int)_Memory + 0x20));
    }
    *(undefined4 *)((int)_Memory + 0x34) = 0xf;
    *(undefined4 *)((int)_Memory + 0x30) = 0;
    *(undefined1 *)((int)_Memory + 0x20) = 0;
    if (0xf < *(uint *)((int)_Memory + 0x18)) {
      _free(*(void **)((int)_Memory + 4));
    }
    *(undefined4 *)((int)_Memory + 0x18) = 0xf;
    *(undefined4 *)((int)_Memory + 0x14) = 0;
    *(undefined1 *)((int)_Memory + 4) = 0;
    _free(_Memory);
    iVar2 = param_1[iVar3 + 0x221];
    if ((int)param_1[0x220] <= iVar2) break;
    _Memory = *(void **)(param_1[0x322] + iVar2 * 4);
    param_1[iVar3 + 0x221] = iVar2 + 1;
  }
  AthenaList_Free((int)puVar1);
  local_4._0_1_ = 8;
  Vec3List_Free(param_1 + 0xa4f);
  local_4._0_1_ = 7;
  Vec3List_Free(param_1 + 0x949);
  local_4._0_1_ = 6;
  Vec3List_Free(param_1 + 0x843);
  local_4._0_1_ = 5;
  Vec3List_Free(param_1 + 0x73d);
  local_4._0_1_ = 4;
  Vec3List_Free(param_1 + 0x637);
  local_4._0_1_ = 3;
  Vec3List_Free(param_1 + 0x531);
  local_4._0_1_ = 2;
  Vec3List_Free(param_1 + 0x42b);
  local_4._0_1_ = 1;
  Vec3List_Free(param_1 + 0x325);
  local_4 = (uint)local_4._1_3_ << 8;
  Vec3List_Free(puVar1);
  local_4 = 0xffffffff;
  SceneObject_dtor(param_1);
  ExceptionList = local_c;
  return;
}

