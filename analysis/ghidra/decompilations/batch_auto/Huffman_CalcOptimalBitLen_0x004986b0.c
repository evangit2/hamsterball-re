
void __cdecl Huffman_CalcOptimalBitLen(int *param_1)

{
  uint uVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  
  iVar4 = 0;
  uVar1 = param_1[1];
  for (uVar3 = uVar1; uVar3 != 0; uVar3 = uVar3 >> 1) {
    iVar4 = iVar4 + 1;
  }
  iVar2 = *param_1;
  iVar4 = (int)uVar1 >> ((byte)(((iVar2 + -1) * (iVar4 + -1)) / iVar2) & 0x1f);
  while( true ) {
    while( true ) {
      iVar5 = 1;
      iVar6 = 1;
      if (0 < iVar2) {
        iVar7 = iVar2;
        do {
          iVar5 = iVar5 * iVar4;
          iVar6 = iVar6 * (iVar4 + 1);
          iVar7 = iVar7 + -1;
        } while (iVar7 != 0);
      }
      if (iVar5 <= (int)uVar1) break;
      iVar4 = iVar4 + -1;
    }
    if ((int)uVar1 < iVar6) break;
    iVar4 = iVar4 + 1;
  }
  return;
}

