
void __cdecl
Huffman_CompressBlock
          (undefined4 *param_1,int *param_2,int param_3,int param_4,int param_5,int param_6)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int *piVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  undefined4 *puVar10;
  int local_10 [4];
  
  local_10[1] = param_2[1];
  local_10[0] = *param_2;
  iVar5 = *(int *)(param_3 + param_5 * 4);
  iVar1 = *(int *)(param_3 + param_4 * 4);
  iVar2 = *(int *)(param_3 + param_6 * 4);
  iVar8 = (int)(iVar5 + (iVar5 >> 0x1f & 3U)) >> 2;
  iVar6 = iVar8 - ((int)(iVar1 + (iVar1 >> 0x1f & 3U)) >> 2);
  iVar8 = (iVar5 / 2 - ((int)(iVar2 + (iVar2 >> 0x1f & 3U)) >> 2)) + iVar8;
  iVar7 = iVar1 / 2 + iVar6;
  iVar1 = iVar2 / 2 + iVar8;
  iVar3 = iVar6;
  iVar9 = 0;
  puVar10 = param_1;
  if (0 < iVar6) {
    for (; iVar9 = iVar6, iVar3 != 0; iVar3 = iVar3 + -1) {
      *puVar10 = 0;
      puVar10 = puVar10 + 1;
    }
  }
  if (iVar9 < iVar7) {
    piVar4 = (int *)local_10[param_4];
    do {
      local_10[2] = (int)((longlong)*piVar4 * (longlong)(int)param_1[iVar9]);
      param_1[iVar9] =
           (int)((ulonglong)((longlong)*piVar4 * (longlong)(int)param_1[iVar9]) >> 0x20) * 2;
      iVar9 = iVar9 + 1;
      piVar4 = piVar4 + 1;
    } while (iVar9 < iVar7);
  }
  if (iVar8 < iVar1) {
    piVar4 = (int *)(local_10[param_6] + -4 + (iVar2 / 2) * 4);
    do {
      param_1[iVar8] =
           (int)((ulonglong)((longlong)*piVar4 * (longlong)(int)param_1[iVar8]) >> 0x20) * 2;
      iVar8 = iVar8 + 1;
      piVar4 = piVar4 + -1;
    } while (iVar8 < iVar1);
  }
  if (iVar8 < iVar5) {
    puVar10 = param_1 + iVar8;
    for (iVar5 = iVar5 - iVar8; iVar5 != 0; iVar5 = iVar5 + -1) {
      *puVar10 = 0;
      puVar10 = puVar10 + 1;
    }
  }
  return;
}

