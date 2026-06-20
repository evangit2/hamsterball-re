
void __cdecl Font_RenderDithered(int param_1,int param_2,undefined4 *param_3,uint param_4)

{
  uint uVar1;
  byte bVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  byte *pbVar9;
  int iVar10;
  char *pcVar11;
  uint local_1c;
  int *local_8;
  
  iVar3 = *(int *)(param_1 + 100);
  iVar4 = *(int *)(param_1 + 0x1a4);
  uVar5 = *(uint *)(param_1 + 0x5c);
  if (0 < (int)param_4) {
    iVar10 = param_2 - (int)param_3;
    local_1c = param_4;
    do {
      Mem_Zero((undefined4 *)*param_3,uVar5);
      iVar6 = *(int *)(iVar4 + 0x30);
      param_1 = 0;
      if (0 < iVar3) {
        local_8 = (int *)(iVar4 + 0x34);
        do {
          param_4 = 0;
          pcVar11 = (char *)*param_3;
          pbVar9 = (byte *)(*(int *)(iVar10 + (int)param_3) + param_1);
          iVar7 = *(int *)(*(int *)(iVar4 + 0x18) + param_1 * 4);
          iVar8 = *local_8;
          for (uVar1 = uVar5; uVar1 != 0; uVar1 = uVar1 - 1) {
            bVar2 = *pbVar9;
            pbVar9 = pbVar9 + iVar3;
            *pcVar11 = *pcVar11 +
                       *(char *)(*(int *)(iVar8 + iVar6 * 0x40 + param_4 * 4) + (uint)bVar2 + iVar7)
            ;
            pcVar11 = pcVar11 + 1;
            param_4 = param_4 + 1 & 0xf;
          }
          param_1 = param_1 + 1;
          local_8 = local_8 + 1;
        } while (param_1 < iVar3);
      }
      param_3 = param_3 + 1;
      local_1c = local_1c - 1;
      *(uint *)(iVar4 + 0x30) = iVar6 + 1U & 0xf;
    } while (local_1c != 0);
  }
  return;
}

