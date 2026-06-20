
void __cdecl ShortArray_FindMatch3(int param_1,short param_2)

{
  uint uVar1;
  
  uVar1 = 0;
  do {
    if (*(short *)(param_1 + uVar1 * 2) == param_2) {
      return;
    }
    uVar1 = uVar1 + 1;
  } while (uVar1 < 3);
  return;
}

