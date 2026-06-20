
/* WARNING: Function: __chkstk replaced with injection: alloca_probe */
/* WARNING: Unable to track spacebase fully for stack */

undefined4 __cdecl
Huffman_DecodeBlock(int *param_1,int *param_2,undefined4 param_3,int param_4,int param_5)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  int *piVar4;
  byte bVar5;
  int *piVar6;
  int *piVar7;
  int aiStack_24 [3];
  
  piVar6 = param_1;
  iVar1 = param_4 / *param_1;
  aiStack_24[2] = 0x499558;
  piVar4 = (int *)(&stack0xffffffe8 + iVar1 * -8);
  piVar7 = (int *)(&stack0xffffffe8 + iVar1 * -8);
  aiStack_24[2 - iVar1] = 0x499568;
  param_4 = 0;
  bVar5 = (byte)(param_5 - param_1[3]);
  if (param_5 - param_1[3] < 0) {
    if (0 < iVar1) {
      do {
        aiStack_24[iVar1 * -2 + 2] = param_3;
        aiStack_24[iVar1 * -2 + 1] = (int)param_1;
        aiStack_24[iVar1 * -2] = 0x49964a;
        uVar2 = Huffman_DecodeSymbol(aiStack_24[iVar1 * -2 + 1],(uint *)aiStack_24[iVar1 * -2 + 2]);
        piVar7[iVar1] = uVar2;
        if (uVar2 == 0xffffffff) {
          return 0xffffffff;
        }
        param_4 = param_4 + 1;
        *piVar7 = param_1[4] + *param_1 * uVar2 * 4;
        piVar7 = piVar7 + 1;
      } while (param_4 < iVar1);
    }
    param_4 = 0;
    if (0 < *param_1) {
      param_1 = param_2;
      do {
        iVar3 = 0;
        if (0 < iVar1) {
          piVar4 = param_1;
          do {
            *piVar4 = *piVar4 + (*(int *)(*(int *)(&stack0xffffffe8 + iVar3 * 4 + iVar1 * -8) +
                                         param_4 * 4) << (-bVar5 & 0x1f));
            iVar3 = iVar3 + 1;
            piVar4 = piVar4 + 1;
          } while (iVar3 < iVar1);
        }
        param_1 = param_1 + iVar1;
        param_4 = param_4 + 1;
      } while (param_4 < *piVar6);
    }
  }
  else {
    if (0 < iVar1) {
      do {
        aiStack_24[iVar1 * -2 + 2] = param_3;
        aiStack_24[iVar1 * -2 + 1] = (int)param_1;
        aiStack_24[iVar1 * -2] = 0x49959b;
        uVar2 = Huffman_DecodeSymbol(aiStack_24[iVar1 * -2 + 1],(uint *)aiStack_24[iVar1 * -2 + 2]);
        piVar4[iVar1] = uVar2;
        if (uVar2 == 0xffffffff) {
          return 0xffffffff;
        }
        param_4 = param_4 + 1;
        *piVar4 = param_1[4] + *param_1 * uVar2 * 4;
        piVar4 = piVar4 + 1;
      } while (param_4 < iVar1);
    }
    param_4 = 0;
    if (0 < *param_1) {
      do {
        iVar3 = 0;
        piVar6 = param_2;
        if (0 < iVar1) {
          do {
            *piVar6 = *piVar6 + (*(int *)(*(int *)(&stack0xffffffe8 + iVar3 * 4 + iVar1 * -8) +
                                         param_4 * 4) >> (bVar5 & 0x1f));
            iVar3 = iVar3 + 1;
            piVar6 = piVar6 + 1;
          } while (iVar3 < iVar1);
        }
        param_2 = param_2 + iVar1;
        param_4 = param_4 + 1;
      } while (param_4 < *param_1);
      return 0;
    }
  }
  return 0;
}

