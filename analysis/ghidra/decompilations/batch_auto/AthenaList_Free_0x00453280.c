
void __fastcall AthenaList_Free(int param_1)

{
  if (*(void **)(param_1 + 0x40c) != (void *)0x0) {
    _free(*(void **)(param_1 + 0x40c));
  }
  *(undefined4 *)(param_1 + 0x40c) = 0;
  *(undefined4 *)(param_1 + 4) = 0;
  return;
}

