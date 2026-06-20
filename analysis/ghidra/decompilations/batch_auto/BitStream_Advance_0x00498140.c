
void __cdecl BitStream_Advance(uint *param_1,int param_2)

{
  uint uVar1;
  int iVar2;
  
  uVar1 = param_2 + *param_1;
  *param_1 = uVar1 & 7;
  iVar2 = (int)(uVar1 + ((int)uVar1 >> 0x1f & 7U)) >> 3;
  param_1[1] = param_1[1] + iVar2;
  uVar1 = param_1[2] - iVar2;
  param_1[2] = uVar1;
  if ((int)uVar1 < 1) {
    Huffman_TreeAdvance();
    return;
  }
  return;
}

