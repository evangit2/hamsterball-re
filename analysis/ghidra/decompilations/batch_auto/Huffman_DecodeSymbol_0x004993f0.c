
uint __cdecl Huffman_DecodeSymbol(int param_1,uint *param_2)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  
  iVar6 = *(int *)(param_1 + 0x28);
  uVar2 = Huffman_ReadBits(param_2,*(int *)(param_1 + 0x24));
  if ((int)uVar2 < 0) {
    iVar7 = *(int *)(param_1 + 8);
    uVar2 = 0;
  }
  else {
    uVar3 = *(uint *)(*(int *)(param_1 + 0x20) + uVar2 * 4);
    if (-1 < (int)uVar3) {
      BitStream_Advance(param_2,(int)*(char *)(*(int *)(param_1 + 0x1c) + -1 + uVar3));
      return uVar3 - 1;
    }
    uVar2 = (int)uVar3 >> 0xf & 0x7fff;
    iVar7 = *(int *)(param_1 + 8) - (uVar3 & 0x7fff);
  }
  uVar3 = Huffman_ReadBits(param_2,iVar6);
  do {
    if (-1 < (int)uVar3) {
LAB_00499492:
      uVar3 = BitUtils_ReverseBits32();
      iVar4 = iVar7 - uVar2;
      if (1 < iVar4) {
        do {
          uVar5 = iVar4 >> 1;
          uVar1 = (uint)(uVar3 < *(uint *)(*(int *)(param_1 + 0x14) + (uVar5 + uVar2) * 4));
          iVar7 = iVar7 - (-uVar1 & uVar5);
          uVar2 = uVar2 + (uVar1 - 1 & uVar5);
          iVar4 = iVar7 - uVar2;
        } while (1 < iVar4);
      }
      iVar7 = (int)*(char *)(*(int *)(param_1 + 0x1c) + uVar2);
      if (iVar6 < iVar7) {
        BitStream_Advance(param_2,iVar6);
        return 0xffffffff;
      }
      BitStream_Advance(param_2,iVar7);
      return uVar2;
    }
    if (iVar6 < 2) {
      if ((int)uVar3 < 0) {
        return 0xffffffff;
      }
      goto LAB_00499492;
    }
    iVar6 = iVar6 + -1;
    uVar3 = Huffman_ReadBits(param_2,iVar6);
  } while( true );
}

