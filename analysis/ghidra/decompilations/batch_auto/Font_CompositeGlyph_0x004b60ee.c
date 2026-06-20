
void __cdecl Font_CompositeGlyph(int param_1,int param_2,undefined4 *param_3,int param_4)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  undefined1 *puVar5;
  byte *pbVar6;
  int iVar7;
  int iVar8;
  int local_c;
  
  iVar2 = *(int *)(*(int *)(param_1 + 0x1a4) + 0x18);
  iVar3 = *(int *)(param_1 + 0x5c);
  iVar4 = *(int *)(param_1 + 100);
  if (0 < param_4) {
    iVar8 = param_2 - (int)param_3;
    local_c = param_4;
    do {
      pbVar6 = *(byte **)(iVar8 + (int)param_3);
      puVar5 = (undefined1 *)*param_3;
      for (iVar1 = iVar3; iVar1 != 0; iVar1 = iVar1 + -1) {
        param_1 = 0;
        param_1._0_1_ = 0;
        iVar7 = 0;
        if (0 < iVar4) {
          do {
            param_1 = param_1 + (uint)*(byte *)(*(int *)(iVar2 + iVar7 * 4) + (uint)*pbVar6);
            pbVar6 = pbVar6 + 1;
            iVar7 = iVar7 + 1;
          } while (iVar7 < iVar4);
        }
        *puVar5 = (undefined1)param_1;
        puVar5 = puVar5 + 1;
      }
      param_3 = param_3 + 1;
      local_c = local_c + -1;
    } while (local_c != 0);
  }
  return;
}

