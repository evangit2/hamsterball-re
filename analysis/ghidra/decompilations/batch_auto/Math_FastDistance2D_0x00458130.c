
uint Math_FastDistance2D(int param_1,int param_2,int param_3,int param_4)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  iVar2 = param_3 - param_1;
  iVar1 = param_4 - param_2;
  if (iVar2 < 0) {
    iVar2 = -iVar2;
  }
  if (iVar1 < 0) {
    iVar1 = -iVar1;
  }
  iVar3 = iVar2;
  if (iVar2 < iVar1) {
    iVar3 = iVar1;
    iVar1 = iVar2;
  }
  return (uint)(iVar1 * 0x66 + iVar3 * 0xf6) >> 8;
}

