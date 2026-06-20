
void __cdecl Channel_FreePairArray(undefined4 *param_1)

{
  int iVar1;
  undefined4 *puVar2;
  
  if (param_1 != (undefined4 *)0x0) {
    iVar1 = 0;
    if (0 < *(int *)param_1[1]) {
      do {
        (**(code **)(*(int *)(param_1[4] + iVar1 * 4) + 0xc))
                  (*(undefined4 *)(param_1[2] + iVar1 * 4));
        (**(code **)(*(int *)(param_1[5] + iVar1 * 4) + 0xc))
                  (*(undefined4 *)(param_1[3] + iVar1 * 4));
        iVar1 = iVar1 + 1;
      } while (iVar1 < *(int *)param_1[1]);
    }
    _free((void *)param_1[4]);
    _free((void *)param_1[5]);
    _free((void *)param_1[2]);
    _free((void *)param_1[3]);
    puVar2 = param_1;
    for (iVar1 = 8; iVar1 != 0; iVar1 = iVar1 + -1) {
      *puVar2 = 0;
      puVar2 = puVar2 + 1;
    }
    _free(param_1);
  }
  return;
}

