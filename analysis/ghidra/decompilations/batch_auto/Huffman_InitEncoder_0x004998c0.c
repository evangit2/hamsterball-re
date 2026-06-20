
undefined4 __cdecl
Huffman_InitEncoder(int *param_1,int param_2,int param_3,int param_4,uint *param_5,int param_6,
                   int param_7)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  byte bVar6;
  int iVar7;
  int iVar8;
  
  iVar8 = 0;
  bVar6 = (byte)(param_7 - param_1[3]);
  if (param_7 - param_1[3] < 0) {
    iVar4 = param_3 + param_6;
    if (param_3 < iVar4) {
      do {
        uVar5 = Huffman_DecodeSymbol((int)param_1,param_5);
        if (uVar5 == 0xffffffff) {
          return 0xffffffff;
        }
        iVar1 = *param_1;
        iVar2 = param_1[4];
        iVar7 = 0;
        if (0 < iVar1) {
          do {
            iVar3 = *(int *)(param_2 + iVar8 * 4);
            *(int *)(iVar3 + param_3 * 4) =
                 *(int *)(iVar3 + param_3 * 4) +
                 (*(int *)(iVar2 + iVar1 * uVar5 * 4 + iVar7 * 4) << (-bVar6 & 0x1f));
            iVar8 = iVar8 + 1;
            if (iVar8 == param_4) {
              iVar8 = 0;
              param_3 = param_3 + 1;
            }
            iVar7 = iVar7 + 1;
          } while (iVar7 < *param_1);
        }
      } while (param_3 < iVar4);
      return 0;
    }
  }
  else {
    iVar4 = param_6 + param_3;
    while (param_3 < iVar4) {
      uVar5 = Huffman_DecodeSymbol((int)param_1,param_5);
      if (uVar5 == 0xffffffff) {
        return 0xffffffff;
      }
      iVar1 = *param_1;
      iVar2 = param_1[4];
      iVar7 = 0;
      if (0 < iVar1) {
        do {
          iVar3 = *(int *)(param_2 + iVar8 * 4);
          *(int *)(iVar3 + param_3 * 4) =
               *(int *)(iVar3 + param_3 * 4) +
               (*(int *)(iVar2 + iVar1 * uVar5 * 4 + iVar7 * 4) >> (bVar6 & 0x1f));
          iVar8 = iVar8 + 1;
          if (iVar8 == param_4) {
            iVar8 = 0;
            param_3 = param_3 + 1;
          }
          iVar7 = iVar7 + 1;
        } while (iVar7 < *param_1);
      }
    }
  }
  return 0;
}

