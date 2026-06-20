
void __fastcall MusicDevice_dtor(undefined4 *param_1)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cdb7b;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_MusicDevice_DeletingDtor_004da09c;
  local_4 = 0;
  RegKey_Open(*(int *)(param_1[1] + 0x54));
  RegKey_WriteDWORD(*(void **)(param_1[1] + 0x54),"Music Volume",param_1[2]);
  RegKey_Close(*(int *)(param_1[1] + 0x54));
  iVar2 = AthenaList_NextIndex((int)(param_1 + 3));
  param_1[iVar2 + 5] = 0;
  if ((int)param_1[4] < 1) {
    puVar3 = (undefined4 *)0x0;
  }
  else {
    puVar3 = *(undefined4 **)param_1[0x106];
    param_1[iVar2 + 5] = 1;
  }
  while (puVar3 != (undefined4 *)0x0) {
    (**(code **)*puVar3)(1);
    iVar1 = param_1[iVar2 + 5];
    if ((int)param_1[4] <= iVar1) break;
    puVar3 = *(undefined4 **)(param_1[0x106] + iVar1 * 4);
    param_1[iVar2 + 5] = iVar1 + 1;
  }
  BASS_Stop();
  BASS_Free();
  local_4 = 0xffffffff;
  Vec3List_Free(param_1 + 3);
  ExceptionList = pvStack_c;
  return;
}

