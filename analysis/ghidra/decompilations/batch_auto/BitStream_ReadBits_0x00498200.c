
uint __cdecl BitStream_ReadBits(uint *param_1,int param_2)

{
  byte bVar1;
  uint uVar2;
  uint uVar3;
  byte *pbVar4;
  char cVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  
  uVar7 = param_1[2];
  uVar2 = *(uint *)(&DAT_004fa740 + param_2 * 4);
  uVar3 = *param_1;
  uVar9 = param_2 + uVar3;
  if ((int)uVar9 < (int)(uVar7 * 8)) {
    pbVar4 = (byte *)param_1[1];
    uVar8 = (uint)(*pbVar4 >> ((byte)*param_1 & 0x1f));
    if (8 < (int)uVar9) {
      cVar5 = (char)uVar3;
      uVar8 = uVar8 | (uint)pbVar4[1] << (8U - cVar5 & 0x1f);
      if ((((0x10 < (int)uVar9) &&
           (uVar8 = uVar8 | (uint)pbVar4[2] << (0x10U - cVar5 & 0x1f), 0x18 < (int)uVar9)) &&
          (uVar8 = uVar8 | (uint)pbVar4[3] << (0x18U - cVar5 & 0x1f), 0x20 < (int)uVar9)) &&
         (uVar3 != 0)) {
        uVar8 = uVar8 | (uint)pbVar4[4] << (0x20U - cVar5 & 0x1f);
      }
    }
    iVar6 = (int)(uVar9 + ((int)uVar9 >> 0x1f & 7U)) >> 3;
    param_1[1] = (uint)(pbVar4 + iVar6);
    *param_1 = uVar9 & 7;
    param_1[2] = uVar7 - iVar6;
    return uVar8 & uVar2;
  }
  if ((int)uVar7 < 0) {
    return 0xffffffff;
  }
  if (uVar9 != 0) {
    iVar6 = Huffman_StreamEnd();
    if (iVar6 != 0) {
      return 0xffffffff;
    }
    uVar7 = (uint)(*(byte *)param_1[1] >> ((byte)*param_1 & 0x1f));
    if (7 < (int)uVar9) {
      param_1[1] = (uint)((byte *)param_1[1] + 1);
      param_1[2] = param_1[2] - 1;
      Huffman_TreeAdvance2();
      if (8 < (int)uVar9) {
        iVar6 = Huffman_StreamEnd();
        if (iVar6 != 0) {
          return 0xffffffff;
        }
        uVar7 = uVar7 | (uint)*(byte *)param_1[1] << (8U - (char)*param_1 & 0x1f);
        if (0xf < (int)uVar9) {
          param_1[1] = (uint)((byte *)param_1[1] + 1);
          param_1[2] = param_1[2] - 1;
          Huffman_TreeAdvance2();
          if (0x10 < (int)uVar9) {
            iVar6 = Huffman_StreamEnd();
            if (iVar6 != 0) {
              return 0xffffffff;
            }
            uVar7 = uVar7 | (uint)*(byte *)param_1[1] << (0x10U - (char)*param_1 & 0x1f);
            if (0x17 < (int)uVar9) {
              param_1[1] = (uint)((byte *)param_1[1] + 1);
              param_1[2] = param_1[2] - 1;
              Huffman_TreeAdvance2();
              if (0x18 < (int)uVar9) {
                iVar6 = Huffman_StreamEnd();
                if (iVar6 != 0) {
                  return 0xffffffff;
                }
                uVar7 = uVar7 | (uint)*(byte *)param_1[1] << (0x18U - (char)*param_1 & 0x1f);
                if (0x1f < (int)uVar9) {
                  param_1[1] = (uint)((byte *)param_1[1] + 1);
                  param_1[2] = param_1[2] - 1;
                  Huffman_TreeAdvance2();
                  if (0x20 < (int)uVar9) {
                    iVar6 = Huffman_StreamEnd();
                    if (iVar6 != 0) {
                      return 0xffffffff;
                    }
                    uVar3 = *param_1;
                    if (uVar3 != 0) {
                      bVar1 = *(byte *)param_1[1];
                      *param_1 = uVar9 & 7;
                      return (uVar7 | (uint)bVar1 << (0x20U - (char)uVar3 & 0x1f)) & uVar2;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  *param_1 = uVar9 & 7;
  return uVar7 & uVar2;
}

