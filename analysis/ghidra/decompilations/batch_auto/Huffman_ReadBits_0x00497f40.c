
uint __cdecl Huffman_ReadBits(uint *param_1,int param_2)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  char cVar4;
  uint uVar5;
  byte *pbVar6;
  int iVar7;
  undefined4 *puVar8;
  int iVar9;
  
  uVar1 = *(uint *)(&DAT_004fa740 + param_2 * 4);
  uVar2 = *param_1;
  uVar3 = param_1[2];
  iVar7 = param_2 + uVar2;
  pbVar6 = (byte *)param_1[1];
  cVar4 = (char)uVar2;
  if (iVar7 < (int)(uVar3 * 8)) {
    uVar5 = (uint)(*pbVar6 >> ((byte)*param_1 & 0x1f));
    if ((((8 < iVar7) && (uVar5 = uVar5 | (uint)pbVar6[1] << (8U - cVar4 & 0x1f), 0x10 < iVar7)) &&
        (uVar5 = uVar5 | (uint)pbVar6[2] << (0x10U - cVar4 & 0x1f), 0x18 < iVar7)) &&
       ((uVar5 = uVar5 | (uint)pbVar6[3] << (0x18U - cVar4 & 0x1f), 0x20 < iVar7 && (uVar2 != 0))))
    {
      return (uVar5 | (uint)pbVar6[4] << (0x20U - cVar4 & 0x1f)) & uVar1;
    }
  }
  else {
    puVar8 = (undefined4 *)param_1[3];
    if ((int)uVar3 < 0) {
      return 0xffffffff;
    }
    uVar5 = uVar2;
    if (iVar7 != 0) {
      while (uVar3 == 0) {
        puVar8 = (undefined4 *)puVar8[3];
        if (puVar8 == (undefined4 *)0x0) {
          return 0xffffffff;
        }
        pbVar6 = (byte *)(puVar8[1] + *(int *)*puVar8);
        uVar3 = puVar8[2];
      }
      uVar5 = (uint)(*pbVar6 >> ((byte)*param_1 & 0x1f));
      pbVar6 = pbVar6 + 1;
      if (8 < iVar7) {
        iVar9 = uVar3 - 1;
        while (iVar9 == 0) {
          puVar8 = (undefined4 *)puVar8[3];
          if (puVar8 == (undefined4 *)0x0) {
            return 0xffffffff;
          }
          pbVar6 = (byte *)(puVar8[1] + *(int *)*puVar8);
          iVar9 = puVar8[2];
        }
        uVar5 = uVar5 | (uint)*pbVar6 << (8U - cVar4 & 0x1f);
        pbVar6 = pbVar6 + 1;
        if (0x10 < iVar7) {
          iVar9 = iVar9 + -1;
          while (iVar9 == 0) {
            puVar8 = (undefined4 *)puVar8[3];
            if (puVar8 == (undefined4 *)0x0) {
              return 0xffffffff;
            }
            pbVar6 = (byte *)(puVar8[1] + *(int *)*puVar8);
            iVar9 = puVar8[2];
          }
          uVar5 = uVar5 | (uint)*pbVar6 << (0x10U - cVar4 & 0x1f);
          pbVar6 = pbVar6 + 1;
          if (0x18 < iVar7) {
            iVar9 = iVar9 + -1;
            while (iVar9 == 0) {
              puVar8 = (undefined4 *)puVar8[3];
              if (puVar8 == (undefined4 *)0x0) {
                return 0xffffffff;
              }
              pbVar6 = (byte *)(puVar8[1] + *(int *)*puVar8);
              iVar9 = puVar8[2];
            }
            uVar5 = uVar5 | (uint)*pbVar6 << (0x18U - cVar4 & 0x1f);
            pbVar6 = pbVar6 + 1;
            if ((0x20 < iVar7) && (uVar2 != 0)) {
              if (iVar9 == 1) {
                do {
                  puVar8 = (undefined4 *)puVar8[3];
                  if (puVar8 == (undefined4 *)0x0) {
                    return 0xffffffff;
                  }
                  pbVar6 = (byte *)(puVar8[1] + *(int *)*puVar8);
                } while (puVar8[2] == 0);
              }
              return (uVar5 | (uint)*pbVar6 << (0x20U - cVar4 & 0x1f)) & uVar1;
            }
          }
        }
      }
    }
  }
  return uVar5 & uVar1;
}

