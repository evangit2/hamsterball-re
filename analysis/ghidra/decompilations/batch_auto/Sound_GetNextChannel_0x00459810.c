
int __fastcall Sound_GetNextChannel(int param_1)

{
  int iVar1;
  int iVar2;
  
  if ((*(int *)(param_1 + 4) != 0) && (*(int *)(*(int *)(param_1 + 4) + 0x84c) != 0)) {
    iVar1 = *(int *)(param_1 + 0x10);
    if (iVar1 < *(int *)(param_1 + 0xc)) {
      iVar2 = *(int *)(*(int *)(param_1 + 0x414) + iVar1 * 4);
      *(int *)(param_1 + 0x10) = iVar1 + 1;
      if (iVar2 != 0) {
        return iVar2;
      }
    }
    *(undefined4 *)(param_1 + 0x10) = 0;
    if (0 < *(int *)(param_1 + 0xc)) {
      iVar1 = **(int **)(param_1 + 0x414);
      *(undefined4 *)(param_1 + 0x10) = 1;
      return iVar1;
    }
  }
  return 0;
}

