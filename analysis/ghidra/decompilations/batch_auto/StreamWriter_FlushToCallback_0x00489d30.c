
void __fastcall StreamWriter_FlushToCallback(undefined4 param_1,int param_2,int param_3)

{
  int iVar1;
  int iVar2;
  
  for (; param_2 != 0; param_2 = *(int *)(param_2 + 0xc)) {
    iVar2 = *(int *)(param_2 + 8);
    if (param_3 < *(int *)(param_2 + 8)) {
      iVar2 = param_3;
    }
    iVar1 = 0;
    if (0 < iVar2) {
      do {
        iVar1 = iVar1 + 1;
      } while (iVar1 < iVar2);
    }
    param_3 = param_3 - iVar1;
  }
  return;
}

