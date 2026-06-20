
void __fastcall UIList_Cleanup(undefined4 *param_1)

{
  undefined4 *puVar1;
  int iVar2;
  int iVar3;
  undefined4 *_Memory;
  void *pvStack_c;
  undefined1 *puStack_8;
  uint local_4;
  
  puStack_8 = &LAB_004cc4b1;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_UIList_DeletingDtor_004d6a70;
  puVar1 = param_1 + 0x223;
  local_4 = 1;
  iVar3 = AthenaList_NextIndex((int)puVar1);
  param_1[iVar3 + 0x225] = 0;
  if ((int)param_1[0x224] < 1) {
    _Memory = (undefined4 *)0x0;
  }
  else {
    _Memory = *(undefined4 **)param_1[0x326];
    param_1[iVar3 + 0x225] = 1;
  }
  while (_Memory != (undefined4 *)0x0) {
    if ((undefined4 *)_Memory[7] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)_Memory[7])(1);
    }
    _free((void *)*_Memory);
    _free((void *)_Memory[1]);
    local_4._0_1_ = 2;
    Vec3List_Free(_Memory + 10);
    local_4 = CONCAT31(local_4._1_3_,1);
    Matrix_Identity(_Memory + 2);
    _free(_Memory);
    iVar2 = param_1[iVar3 + 0x225];
    if ((int)param_1[0x224] <= iVar2) break;
    _Memory = *(undefined4 **)(param_1[0x326] + iVar2 * 4);
    param_1[iVar3 + 0x225] = iVar2 + 1;
  }
  AthenaList_Free((int)puVar1);
  if ((undefined4 *)param_1[0x329] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x329])(1);
  }
  if ((undefined4 *)param_1[0x32a] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x32a])(1);
  }
  local_4 = local_4 & 0xffffff00;
  Vec3List_Free(puVar1);
  local_4 = 0xffffffff;
  SceneObject_dtor(param_1);
  ExceptionList = pvStack_c;
  return;
}

