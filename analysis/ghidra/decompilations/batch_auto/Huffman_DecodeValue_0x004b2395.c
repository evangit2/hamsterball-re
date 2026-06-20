
uint __cdecl
Huffman_DecodeValue(undefined4 *param_1,uint param_2,int param_3,int param_4,int *param_5)

{
  undefined4 uVar1;
  int iVar2;
  uint uVar3;
  int *local_8;
  
  local_8 = param_5;
  if (param_3 < (int)param_5) {
    uVar1 = BitStream_ReadBitsSSE2(param_1,param_2,param_3,(int)param_5);
    if ((char)uVar1 == '\0') {
      return 0xffffffff;
    }
    param_2 = param_1[3];
    param_3 = param_1[6];
  }
  iVar2 = param_3 - (int)param_5;
  uVar3 = (int)param_2 >> ((byte)iVar2 & 0x1f) & (1 << ((byte)param_5 & 0x1f)) - 1U;
  param_5 = (int *)(param_4 + 0x44 + (int)param_5 * 4);
  if (*param_5 < (int)uVar3) {
    do {
      if (iVar2 < 1) {
        uVar1 = BitStream_ReadBitsSSE2(param_1,param_2,iVar2,1);
        if ((char)uVar1 == '\0') {
          return 0xffffffff;
        }
        param_2 = param_1[3];
        iVar2 = param_1[6];
      }
      param_5 = param_5 + 1;
      iVar2 = iVar2 + -1;
      uVar3 = uVar3 << 1 | (int)param_2 >> ((byte)iVar2 & 0x1f) & 1U;
      local_8 = (int *)((int)local_8 + 1);
    } while (*param_5 < (int)uVar3);
  }
  param_1[6] = iVar2;
  param_1[3] = param_2;
  if ((int)local_8 < 0x11) {
    uVar3 = (uint)*(byte *)((*(int *)(param_4 + 0x8c + (int)local_8 * 4) -
                            *(int *)(param_4 + (int)local_8 * 4)) + *(int *)(param_4 + 0xd0) + 0x11
                           + uVar3);
  }
  else {
    *(undefined4 *)(*(int *)param_1[7] + 0x14) = 0x72;
    (**(code **)(*(int *)param_1[7] + 4))((int *)param_1[7],0xffffffff);
    uVar3 = 0;
  }
  return uVar3;
}

