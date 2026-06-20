
void __cdecl IMDCT_Destroy(undefined4 *param_1)

{
  int iVar1;
  undefined4 *puVar2;
  
  if (param_1 != (undefined4 *)0x0) {
    if ((void *)param_1[3] != (void *)0x0) {
      _free((void *)param_1[3]);
    }
    if ((void *)param_1[5] != (void *)0x0) {
      _free((void *)param_1[5]);
    }
    puVar2 = param_1;
    for (iVar1 = 6; iVar1 != 0; iVar1 = iVar1 + -1) {
      *puVar2 = 0;
      puVar2 = puVar2 + 1;
    }
    _free(param_1);
  }
  return;
}

