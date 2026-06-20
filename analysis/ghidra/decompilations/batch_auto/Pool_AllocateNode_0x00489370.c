
void __fastcall Pool_AllocateNode(int param_1)

{
  void *pvVar1;
  
  *(int *)(param_1 + 8) = *(int *)(param_1 + 8) + 1;
  pvVar1 = *(void **)(param_1 + 4);
  if (pvVar1 == (void *)0x0) {
    pvVar1 = _malloc(0x10);
  }
  else {
    *(undefined4 *)(param_1 + 4) = *(undefined4 *)((int)pvVar1 + 0xc);
  }
  *(undefined4 *)((int)pvVar1 + 4) = 0;
  *(undefined4 *)((int)pvVar1 + 8) = 0;
  *(undefined4 *)((int)pvVar1 + 0xc) = 0;
  return;
}

