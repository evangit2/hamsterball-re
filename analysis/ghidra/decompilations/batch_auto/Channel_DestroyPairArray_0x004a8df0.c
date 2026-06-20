
void __cdecl Channel_DestroyPairArray(undefined4 *param_1)

{
  void *_Memory;
  int iVar1;
  undefined4 *puVar2;
  
  if (param_1 != (undefined4 *)0x0) {
    iVar1 = 0;
    if (0 < (int)param_1[2]) {
      do {
        _Memory = *(void **)(param_1[6] + iVar1 * 4);
        if (_Memory != (void *)0x0) {
          _free(_Memory);
        }
        iVar1 = iVar1 + 1;
      } while (iVar1 < (int)param_1[2]);
    }
    _free((void *)param_1[6]);
    iVar1 = 0;
    if (0 < (int)param_1[7]) {
      do {
        _free(*(void **)(param_1[8] + iVar1 * 4));
        iVar1 = iVar1 + 1;
      } while (iVar1 < (int)param_1[7]);
    }
    _free((void *)param_1[8]);
    puVar2 = param_1;
    for (iVar1 = 9; iVar1 != 0; iVar1 = iVar1 + -1) {
      *puVar2 = 0;
      puVar2 = puVar2 + 1;
    }
    _free(param_1);
  }
  return;
}

