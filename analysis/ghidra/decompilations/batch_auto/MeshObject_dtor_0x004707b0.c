
void __fastcall MeshObject_dtor(undefined4 *param_1)

{
  int *piVar1;
  undefined4 *puVar2;
  int iVar3;
  int iVar4;
  void *pvStack_c;
  undefined1 *puStack_8;
  int local_4;
  
  puStack_8 = &LAB_004cd864;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &MeshWorld_vtable;
  local_4 = 2;
  if (((*(char *)(param_1 + 0x116) == '\x01') && (param_1[10] != 0)) &&
     (iVar3 = 0, 0 < (int)param_1[9])) {
    iVar4 = 0;
    do {
      puVar2 = *(undefined4 **)(iVar4 + 0x48 + param_1[10]);
      if (puVar2 != (undefined4 *)0x0) {
        Texture_RemoveRef((void *)param_1[1],puVar2);
      }
      iVar3 = iVar3 + 1;
      iVar4 = iVar4 + 0x50;
    } while (iVar3 < (int)param_1[9]);
  }
  puVar2 = (undefined4 *)param_1[10];
  if (puVar2 != (undefined4 *)0x0) {
    if (puVar2[-1] == 0) {
      _free(puVar2 + -1);
    }
    else {
      (**(code **)*puVar2)(3);
    }
    param_1[10] = 0;
  }
  param_1[0xd] = 0;
  if ((int)param_1[0xc] < 1) {
    puVar2 = (undefined4 *)0x0;
  }
  else {
    puVar2 = *(undefined4 **)param_1[0x10e];
    param_1[0xd] = 1;
  }
  while (puVar2 != (undefined4 *)0x0) {
    (**(code **)*puVar2)(1);
    iVar3 = param_1[0xd];
    if ((int)param_1[0xc] <= iVar3) break;
    puVar2 = *(undefined4 **)(param_1[0x10e] + iVar3 * 4);
    param_1[0xd] = iVar3 + 1;
  }
  AthenaList_Free((int)(param_1 + 0xb));
  _free((void *)param_1[0x112]);
  piVar1 = (int *)param_1[0x113];
  param_1[0x112] = 0;
  if (piVar1 != (int *)0x0) {
    (**(code **)(*piVar1 + 8))(piVar1);
  }
  piVar1 = (int *)param_1[0x114];
  if (piVar1 != (int *)0x0) {
    (**(code **)(*piVar1 + 8))(piVar1);
  }
  piVar1 = (int *)param_1[0x115];
  if (piVar1 != (int *)0x0) {
    (**(code **)(*piVar1 + 8))(piVar1);
    param_1[0x115] = 0;
  }
  local_4._0_1_ = 1;
  NoOp();
  local_4 = (uint)local_4._1_3_ << 8;
  Vec3List_Free(param_1 + 0xb);
  if (0xf < (uint)param_1[8]) {
    _free((void *)param_1[3]);
  }
  param_1[7] = 0;
  param_1[8] = 0xf;
  *(undefined1 *)(param_1 + 3) = 0;
  ExceptionList = pvStack_c;
  return;
}

