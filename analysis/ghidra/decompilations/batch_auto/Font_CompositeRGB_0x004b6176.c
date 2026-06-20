
void __cdecl Font_CompositeRGB(int param_1,int param_2,int *param_3,int param_4)

{
  int iVar1;
  byte bVar2;
  int *piVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  byte *pbVar9;
  byte *pbVar10;
  byte *pbVar11;
  char *pcVar12;
  
  piVar3 = *(int **)(*(int *)(param_1 + 0x1a4) + 0x18);
  iVar4 = *piVar3;
  iVar5 = piVar3[1];
  iVar6 = piVar3[2];
  iVar7 = *(int *)(param_1 + 0x5c);
  if (0 < param_4) {
    iVar8 = param_2 - (int)param_3;
    do {
      pbVar11 = *(byte **)(iVar8 + (int)param_3);
      pcVar12 = (char *)*param_3;
      for (iVar1 = iVar7; iVar1 != 0; iVar1 = iVar1 + -1) {
        bVar2 = *pbVar11;
        pbVar9 = pbVar11 + 1;
        pbVar10 = pbVar11 + 2;
        pbVar11 = pbVar11 + 3;
        *pcVar12 = *(char *)((uint)bVar2 + iVar4) + *(char *)((uint)*pbVar9 + iVar5) +
                   *(char *)((uint)*pbVar10 + iVar6);
        pcVar12 = pcVar12 + 1;
      }
      param_3 = param_3 + 1;
      param_4 = param_4 + -1;
    } while (param_4 != 0);
  }
  return;
}

