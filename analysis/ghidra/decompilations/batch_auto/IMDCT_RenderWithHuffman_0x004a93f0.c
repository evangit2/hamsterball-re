
undefined4 __cdecl
IMDCT_RenderWithHuffman(int param_1,int *param_2,undefined4 *param_3,int param_4,int param_5)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  int iVar3;
  
  iVar3 = 0;
  if (0 < param_5) {
    puVar1 = param_3;
    do {
      if (*(int *)((param_4 - (int)param_3) + (int)puVar1) != 0) {
        param_3[iVar3] = *puVar1;
        iVar3 = iVar3 + 1;
      }
      puVar1 = puVar1 + 1;
      param_5 = param_5 + -1;
    } while (param_5 != 0);
    if (iVar3 != 0) {
      uVar2 = Font_RenderChannels(param_1,param_2,(int)param_3,iVar3,Huffman_DecodeBlock);
      return uVar2;
    }
  }
  return 0;
}

