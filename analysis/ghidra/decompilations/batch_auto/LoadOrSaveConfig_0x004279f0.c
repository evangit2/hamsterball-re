
void __fastcall LoadOrSaveConfig(undefined4 *param_1)

{
  int iVar1;
  int *piVar2;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cae73;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_App_LoadOrSaveConfig_DeletingDtor_004d2660;
  local_4 = 1;
  if ((undefined4 *)param_1[0x243] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x243])(1);
  }
  if ((undefined4 *)param_1[0x244] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x244])(1);
  }
  (**(code **)(*(int *)param_1[0x8c] + 8))("DATA\\HS.CFG");
  if ((undefined4 *)param_1[0x88] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x88])(1);
  }
  param_1[0x88] = 0;
  if ((undefined4 *)param_1[0xc6] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0xc6])(1);
  }
  param_1[0xc6] = 0;
  if ((undefined4 *)param_1[199] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[199])(1);
  }
  param_1[199] = 0;
  if ((undefined4 *)param_1[200] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[200])(1);
  }
  param_1[200] = 0;
  if ((undefined4 *)param_1[0xc9] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0xc9])(1);
  }
  param_1[0xc9] = 0;
  if ((undefined4 *)param_1[0xca] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0xca])(1);
  }
  param_1[0xca] = 0;
  if ((undefined4 *)param_1[0x91] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x91])(1);
  }
  param_1[0x91] = 0;
  if ((undefined4 *)param_1[0x92] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x92])(1);
  }
  param_1[0x92] = 0;
  if ((undefined4 *)param_1[0x93] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x93])(1);
  }
  param_1[0x93] = 0;
  if ((undefined4 *)param_1[0x2c9] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x2c9])(1);
  }
  param_1[0x2c9] = 0;
  piVar2 = param_1 + 0x94;
  iVar1 = 4;
  do {
    if ((undefined4 *)*piVar2 != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)*piVar2)(1);
    }
    *piVar2 = 0;
    piVar2 = piVar2 + 1;
    iVar1 = iVar1 + -1;
  } while (iVar1 != 0);
  if ((undefined4 *)param_1[0x98] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x98])(1);
  }
  param_1[0x98] = 0;
  if ((undefined4 *)param_1[0x99] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x99])(1);
  }
  param_1[0x99] = 0;
  if ((undefined4 *)param_1[0x9a] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x9a])(1);
  }
  param_1[0x9a] = 0;
  if ((undefined4 *)param_1[0x9b] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x9b])(1);
  }
  param_1[0x9b] = 0;
  if ((undefined4 *)param_1[0x9c] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x9c])(1);
  }
  param_1[0x9c] = 0;
  if ((undefined4 *)param_1[0x9d] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x9d])(1);
  }
  param_1[0x9d] = 0;
  if ((undefined4 *)param_1[0x8c] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x8c])(1);
  }
  param_1[0x8c] = 0;
  if ((undefined4 *)param_1[0xcb] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0xcb])(1);
    param_1[0xcb] = 0;
  }
  if ((undefined4 *)param_1[0xcf] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0xcf])(1);
    param_1[0xcf] = 0;
  }
  if ((undefined4 *)param_1[0xd0] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0xd0])(1);
    param_1[0xd0] = 0;
  }
  if ((undefined4 *)param_1[0xd1] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0xd1])(1);
    param_1[0xd1] = 0;
  }
  if ((undefined4 *)param_1[0xd2] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0xd2])(1);
    param_1[0xd2] = 0;
  }
  if ((undefined4 *)param_1[0xd3] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0xd3])(1);
    param_1[0xd3] = 0;
  }
  if ((undefined4 *)param_1[0xd4] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0xd4])(1);
    param_1[0xd4] = 0;
  }
  if ((undefined4 *)param_1[0xd5] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0xd5])(1);
    param_1[0xd5] = 0;
  }
  if ((undefined4 *)param_1[0xd6] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0xd6])(1);
    param_1[0xd6] = 0;
  }
  if ((undefined4 *)param_1[0xd7] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0xd7])(1);
    param_1[0xd7] = 0;
  }
  if ((undefined4 *)param_1[0xd8] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0xd8])(1);
    param_1[0xd8] = 0;
  }
  if ((undefined4 *)param_1[0xd9] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0xd9])(1);
    param_1[0xd9] = 0;
  }
  if ((undefined4 *)param_1[0xe4] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0xe4])(1);
    param_1[0xe4] = 0;
  }
  if ((undefined4 *)param_1[0xe5] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0xe5])(1);
    param_1[0xe5] = 0;
  }
  if ((undefined4 *)param_1[0xe6] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0xe6])(1);
    param_1[0xe6] = 0;
  }
  if ((undefined4 *)param_1[0xe7] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0xe7])(1);
    param_1[0xe7] = 0;
  }
  if ((undefined4 *)param_1[0xe8] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0xe8])(1);
    param_1[0xe8] = 0;
  }
  if ((undefined4 *)param_1[0xe9] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0xe9])(1);
    param_1[0xe9] = 0;
  }
  if ((undefined4 *)param_1[0xda] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0xda])(1);
    param_1[0xda] = 0;
  }
  if ((undefined4 *)param_1[0xdb] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0xdb])(1);
    param_1[0xdb] = 0;
  }
  if ((undefined4 *)param_1[0xdc] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0xdc])(1);
    param_1[0xdc] = 0;
  }
  if ((undefined4 *)param_1[0x169] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x169])(1);
    param_1[0x169] = 0;
  }
  if ((undefined4 *)param_1[0x167] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x167])(1);
    param_1[0x167] = 0;
  }
  if ((undefined4 *)param_1[0x168] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x168])(1);
    param_1[0x168] = 0;
  }
  if ((undefined4 *)param_1[0x165] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x165])(1);
    param_1[0x165] = 0;
  }
  if ((undefined4 *)param_1[0x166] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x166])(1);
    param_1[0x166] = 0;
  }
  if ((undefined4 *)param_1[0x164] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x164])(1);
    param_1[0x164] = 0;
  }
  if ((undefined4 *)param_1[0x163] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x163])(1);
    param_1[0x163] = 0;
  }
  if ((undefined4 *)param_1[0x15e] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x15e])(1);
    param_1[0x15e] = 0;
  }
  if ((undefined4 *)param_1[0x15d] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x15d])(1);
    param_1[0x15d] = 0;
  }
  if ((undefined4 *)param_1[0x15c] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x15c])(1);
    param_1[0x15c] = 0;
  }
  if ((undefined4 *)param_1[0x14c] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x14c])(1);
    param_1[0x14c] = 0;
  }
  if ((undefined4 *)param_1[0x161] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x161])(1);
    param_1[0x161] = 0;
  }
  if ((undefined4 *)param_1[0x162] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x162])(1);
    param_1[0x162] = 0;
  }
  if ((undefined4 *)param_1[0x16a] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x16a])(1);
    param_1[0x16a] = 0;
  }
  if ((undefined4 *)param_1[0x16b] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x16b])(1);
    param_1[0x16b] = 0;
  }
  if ((undefined4 *)param_1[0x16c] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x16c])(1);
    param_1[0x16c] = 0;
  }
  if ((undefined4 *)param_1[0x16f] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x16f])(1);
    param_1[0x16f] = 0;
  }
  if ((undefined4 *)param_1[0x170] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x170])(1);
    param_1[0x170] = 0;
  }
  if ((undefined4 *)param_1[0x171] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x171])(1);
    param_1[0x171] = 0;
  }
  if ((undefined4 *)param_1[0x16d] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x16d])(1);
    param_1[0x16d] = 0;
  }
  if ((undefined4 *)param_1[0x16e] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x16e])(1);
    param_1[0x16e] = 0;
  }
  if ((undefined4 *)param_1[0x172] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x172])(1);
    param_1[0x172] = 0;
  }
  if (*(char *)(param_1 + 0x80) == '\0') {
    ShellExecuteA((HWND)0x0,"open","http://www.raptisoft.com",(LPCSTR)0x0,(LPCSTR)0x0,1);
  }
  puStack_8 = (undefined1 *)((uint)puStack_8 & 0xffffff00);
  _eh_vector_destructor_iterator_(param_1 + 0x173,0xa0,4,StdString3_Clear);
  puStack_8 = (undefined1 *)0xffffffff;
  App_Shutdown(param_1);
  ExceptionList = param_1;
  return;
}

