
undefined4 __cdecl Vorbis_LookupMappingValue(int param_1,undefined4 *param_2)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  uint local_18 [6];
  
  iVar1 = *(int *)(param_1 + 0x1c);
  Huffman_StreamInit(local_18,(undefined4 *)*param_2);
  uVar2 = BitStream_ReadBits(local_18,1);
  if (uVar2 != 0) {
    return 0xffffff79;
  }
  iVar4 = 0;
  for (iVar3 = *(int *)(iVar1 + 8); 1 < iVar3; iVar3 = iVar3 >> 1) {
    iVar4 = iVar4 + 1;
  }
  uVar2 = BitStream_ReadBits(local_18,iVar4);
  if (uVar2 != 0xffffffff) {
    return *(undefined4 *)(iVar1 + **(int **)(iVar1 + 0x20 + uVar2 * 4) * 4);
  }
  return 0xffffff78;
}

