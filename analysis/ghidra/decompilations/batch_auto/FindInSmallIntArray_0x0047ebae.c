
void __cdecl FindInSmallIntArray(int param_1,int param_2)

{
  uint uVar1;
  
  uVar1 = 0;
  do {
    if (*(int *)(param_1 + uVar1 * 4) == param_2) {
      return;
    }
    uVar1 = uVar1 + 1;
  } while (uVar1 < 3);
  return;
}

