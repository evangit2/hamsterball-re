
uint __cdecl Huffman_DecodeAndRemap(int param_1,uint *param_2)

{
  uint uVar1;
  
  uVar1 = Huffman_DecodeSymbol(param_1,param_2);
  if (-1 < (int)uVar1) {
    uVar1 = *(uint *)(*(int *)(param_1 + 0x18) + uVar1 * 4);
  }
  return uVar1;
}

