
void __fastcall FontList_Dtor(undefined4 *param_1)

{
  undefined4 *puVar1;
  int iVar2;
  int iVar3;
  undefined4 *puVar4;
  int *piVar5;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004ccd2b;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_FontList_ScalarDtor_004d8e30;
  puVar1 = param_1 + 2;
  local_4 = 0;
  iVar3 = AthenaList_NextIndex((int)puVar1);
  param_1[iVar3 + 4] = 0;
  if ((int)param_1[3] < 1) {
    puVar4 = (undefined4 *)0x0;
  }
  else {
    puVar4 = *(undefined4 **)param_1[0x105];
    param_1[iVar3 + 4] = 1;
  }
  while (puVar4 != (undefined4 *)0x0) {
    Texture_RemoveRef((void *)param_1[1],puVar4);
    iVar2 = param_1[iVar3 + 4];
    if ((int)param_1[3] <= iVar2) break;
    puVar4 = *(undefined4 **)(param_1[0x105] + iVar2 * 4);
    param_1[iVar3 + 4] = iVar2 + 1;
  }
  AthenaList_Free((int)puVar1);
  piVar5 = param_1 + 0x10f;
  iVar3 = 0x100;
  do {
    if ((undefined4 *)*piVar5 != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)*piVar5)(1);
    }
    *piVar5 = 0;
    piVar5 = piVar5 + 5;
    iVar3 = iVar3 + -1;
  } while (iVar3 != 0);
  local_4 = 0xffffffff;
  Vec3List_Free(puVar1);
  ExceptionList = pvStack_c;
  return;
}

