
longlong __cdecl BitStream_ReadValue(int param_1,int param_2)

{
  int iVar1;
  longlong lVar2;
  int iVar3;
  longlong lVar4;
  
  if (*(int *)(param_1 + 0x40) < 2) {
    return -0x83;
  }
  if ((*(int *)(param_1 + 4) != 0) && (iVar1 = *(int *)(param_1 + 0x1c), param_2 < iVar1)) {
    if (-1 < param_2) {
      return CONCAT44(*(undefined4 *)(*(int *)(param_1 + 0x2c) + 0xc + param_2 * 0x10),
                      *(undefined4 *)(*(int *)(param_1 + 0x2c) + 8 + param_2 * 0x10));
    }
    lVar4 = 0;
    iVar3 = 0;
    lVar2 = 0;
    if (0 < iVar1) {
      do {
        lVar4 = BitStream_ReadValue(param_1,iVar3);
        lVar4 = lVar4 + lVar2;
        iVar3 = iVar3 + 1;
        lVar2 = lVar4;
      } while (iVar3 < iVar1);
    }
    return lVar4;
  }
  return -0x83;
}

