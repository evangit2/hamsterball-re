
undefined4 __cdecl Audio_DequantizeGranule(int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  uint uVar4;
  int iVar5;
  int *piVar6;
  uint uVar7;
  int iVar8;
  uint uVar9;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  uint local_4c;
  int local_40;
  int local_3c;
  int local_38;
  int local_30 [4];
  uint local_20;
  undefined2 *local_1c;
  int local_18;
  uint *local_14;
  int local_10;
  int local_c;
  uint local_8;
  
  iVar1 = *(int *)(param_1 + 0x194);
  if (((*(int *)(param_1 + 0xfc) == 0) || (*(int *)(iVar1 + 0x30) != 0)) ||
     (uVar3 = Audio_ResetBitReservoir(param_1), (char)uVar3 != '\0')) {
    local_58 = **(undefined4 **)(param_1 + 0x14);
    local_54 = (*(undefined4 **)(param_1 + 0x14))[1];
    local_10 = 0;
    local_38 = iVar1 + 0x18;
    local_30[0] = *(int *)(iVar1 + 0x20);
    local_30[1] = *(undefined4 *)(iVar1 + 0x24);
    local_50 = *(undefined4 *)(param_1 + 0x178);
    local_30[2] = *(undefined4 *)(iVar1 + 0x28);
    uVar7 = *(uint *)(iVar1 + 0x10);
    iVar5 = *(int *)(iVar1 + 0x14);
    local_3c = param_1;
    local_30[3] = *(undefined4 *)(iVar1 + 0x2c);
    if (0 < *(int *)(param_1 + 0x13c)) {
      local_14 = (uint *)(param_1 + 0x140);
      do {
        local_1c = *(undefined2 **)(param_2 + local_10 * 4);
        iVar8 = *(int *)(param_1 + 0x124 + *local_14 * 4);
        local_8 = *(int *)(iVar1 + 0x34 + *(int *)(iVar8 + 0x14) * 4);
        local_20 = *local_14;
        local_18 = iVar8;
        local_c = *(int *)(iVar1 + 0x44 + *(int *)(iVar8 + 0x18) * 4);
        if (iVar5 < 8) {
          uVar4 = BitStream_ReadBitsSSE2(&local_58,uVar7,iVar5,0);
          if ((char)uVar4 == '\0') goto LAB_004b28a6;
          iVar5 = local_40;
          uVar7 = local_4c;
          if (7 < local_40) goto LAB_004b25b9;
          piVar6 = (int *)0x1;
LAB_004b25e6:
          uVar4 = Huffman_DecodeValue(&local_58,uVar7,iVar5,local_8,piVar6);
          iVar5 = local_40;
          uVar7 = local_4c;
          uVar9 = uVar4;
          if ((int)uVar4 < 0) goto LAB_004b28a6;
        }
        else {
LAB_004b25b9:
          uVar4 = (int)uVar7 >> ((char)iVar5 - 8U & 0x1f) & 0xff;
          iVar2 = *(int *)(local_8 + 0xd4 + uVar4 * 4);
          if (iVar2 == 0) {
            piVar6 = (int *)0x9;
            goto LAB_004b25e6;
          }
          iVar5 = iVar5 - iVar2;
          uVar9 = (uint)*(byte *)(uVar4 + 0x4d4 + local_8);
        }
        uVar4 = 0;
        if (uVar9 != 0) {
          if ((iVar5 < (int)uVar9) &&
             (uVar4 = BitStream_ReadBitsSSE2(&local_58,uVar7,iVar5,uVar9), iVar5 = local_40,
             uVar7 = local_4c, (char)uVar4 == '\0')) {
LAB_004b28a6:
            return uVar4 & 0xffffff00;
          }
          iVar5 = iVar5 - uVar9;
          local_8 = (1 << ((byte)uVar9 & 0x1f)) - 1;
          uVar4 = local_8 & (int)uVar7 >> ((byte)iVar5 & 0x1f);
          iVar8 = local_18;
          if ((int)uVar4 < *(int *)(&DAT_004e90b8 + uVar9 * 4)) {
            uVar4 = *(int *)(&DAT_004e90f8 + uVar9 * 4) + uVar4;
          }
        }
        if (*(char *)(iVar8 + 0x30) == '\0') {
LAB_004b2795:
          local_8 = 1;
          do {
            if (iVar5 < 8) {
              uVar4 = BitStream_ReadBitsSSE2(&local_58,uVar7,iVar5,0);
              if ((char)uVar4 == '\0') goto LAB_004b28a6;
              iVar5 = local_40;
              uVar7 = local_4c;
              if (7 < local_40) goto LAB_004b27cc;
              piVar6 = (int *)0x1;
LAB_004b27f6:
              uVar4 = Huffman_DecodeValue(&local_58,uVar7,iVar5,local_c,piVar6);
              iVar5 = local_40;
              uVar7 = local_4c;
              if ((int)uVar4 < 0) goto LAB_004b28a6;
            }
            else {
LAB_004b27cc:
              uVar4 = (int)uVar7 >> ((char)iVar5 - 8U & 0x1f) & 0xff;
              iVar8 = *(int *)(local_c + 0xd4 + uVar4 * 4);
              if (iVar8 == 0) {
                piVar6 = (int *)0x9;
                goto LAB_004b27f6;
              }
              uVar4 = (uint)*(byte *)(uVar4 + 0x4d4 + local_c);
              iVar5 = iVar5 - iVar8;
            }
            uVar9 = uVar4 & 0xf;
            if (uVar9 == 0) {
              if ((int)uVar4 >> 4 != 0xf) break;
              local_8 = local_8 + 0xf;
            }
            else {
              local_8 = local_8 + ((int)uVar4 >> 4);
              if ((iVar5 < (int)uVar9) &&
                 (uVar4 = BitStream_ReadBitsSSE2(&local_58,uVar7,iVar5,uVar9), iVar5 = local_40,
                 uVar7 = local_4c, (char)uVar4 == '\0')) goto LAB_004b28a6;
              iVar5 = iVar5 - uVar9;
            }
            local_8 = local_8 + 1;
          } while ((int)local_8 < 0x40);
        }
        else {
          piVar6 = local_30 + local_20;
          *piVar6 = *piVar6 + uVar4;
          *local_1c = (short)*piVar6;
          if (*(int *)(iVar8 + 0x24) < 2) goto LAB_004b2795;
          local_8 = 1;
          do {
            if (iVar5 < 8) {
              uVar4 = BitStream_ReadBitsSSE2(&local_58,uVar7,iVar5,0);
              if ((char)uVar4 == '\0') goto LAB_004b28a6;
              iVar5 = local_40;
              uVar7 = local_4c;
              if (7 < local_40) goto LAB_004b26bd;
              piVar6 = (int *)0x1;
LAB_004b26e7:
              uVar4 = Huffman_DecodeValue(&local_58,uVar7,iVar5,local_c,piVar6);
              iVar5 = local_40;
              uVar7 = local_4c;
              if ((int)uVar4 < 0) goto LAB_004b28a6;
            }
            else {
LAB_004b26bd:
              uVar4 = (int)uVar7 >> ((char)iVar5 - 8U & 0x1f) & 0xff;
              iVar8 = *(int *)(local_c + 0xd4 + uVar4 * 4);
              if (iVar8 == 0) {
                piVar6 = (int *)0x9;
                goto LAB_004b26e7;
              }
              uVar4 = (uint)*(byte *)(uVar4 + 0x4d4 + local_c);
              iVar5 = iVar5 - iVar8;
            }
            uVar9 = uVar4 & 0xf;
            local_20 = uVar9;
            if (uVar9 == 0) {
              if ((int)uVar4 >> 4 != 0xf) break;
              local_8 = local_8 + 0xf;
            }
            else {
              local_8 = local_8 + ((int)uVar4 >> 4);
              if ((iVar5 < (int)uVar9) &&
                 (uVar4 = BitStream_ReadBitsSSE2(&local_58,uVar7,iVar5,uVar9), iVar5 = local_40,
                 uVar7 = local_4c, (char)uVar4 == '\0')) goto LAB_004b28a6;
              iVar5 = iVar5 - uVar9;
              uVar4 = (1 << (sbyte)uVar9) - 1U & (int)uVar7 >> ((byte)iVar5 & 0x1f);
              if ((int)uVar4 < *(int *)(&DAT_004e90b8 + local_20 * 4)) {
                uVar4 = *(int *)(&DAT_004e90f8 + local_20 * 4) + uVar4;
              }
              local_1c[(&DAT_004e65e0)[local_8]] = (short)uVar4;
            }
            local_8 = local_8 + 1;
          } while ((int)local_8 < 0x40);
        }
        local_10 = local_10 + 1;
        local_14 = local_14 + 1;
      } while (local_10 < *(int *)(param_1 + 0x13c));
    }
    **(undefined4 **)(param_1 + 0x14) = local_58;
    *(undefined4 *)(*(int *)(param_1 + 0x14) + 4) = local_54;
    *(undefined4 *)(param_1 + 0x178) = local_50;
    *(int *)(iVar1 + 0x30) = *(int *)(iVar1 + 0x30) + -1;
    *(int *)(iVar1 + 0x20) = local_30[0];
    *(int *)(iVar1 + 0x24) = local_30[1];
    *(int *)(iVar1 + 0x28) = local_30[2];
    *(int *)(iVar1 + 0x14) = iVar5;
    *(uint *)(iVar1 + 0x10) = uVar7;
    *(int *)(iVar1 + 0x2c) = local_30[3];
    uVar3 = CONCAT31((int3)((uint)iVar5 >> 8),1);
  }
  return uVar3;
}

