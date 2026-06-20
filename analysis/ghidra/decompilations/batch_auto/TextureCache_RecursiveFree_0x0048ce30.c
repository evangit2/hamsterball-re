
void __fastcall TextureCache_RecursiveFree(int param_1)

{
  void *pvVar1;
  
  if ((*(void **)(param_1 + 4) != (void *)0x0) && (*(int *)(param_1 + 0x38) != 0)) {
    _free(*(void **)(param_1 + 4));
  }
  if ((*(void **)(param_1 + 8) != (void *)0x0) && (*(int *)(param_1 + 0x3c) != 0)) {
    _free(*(void **)(param_1 + 8));
  }
  pvVar1 = *(void **)(param_1 + 0x4c);
  if (pvVar1 != (void *)0x0) {
    TextureCache_RecursiveFree((int)pvVar1);
    _free(pvVar1);
  }
  pvVar1 = *(void **)(param_1 + 0x50);
  if (pvVar1 != (void *)0x0) {
    TextureCache_RecursiveFree((int)pvVar1);
    _free(pvVar1);
  }
  return;
}

