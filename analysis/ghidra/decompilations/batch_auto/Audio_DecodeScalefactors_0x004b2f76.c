
undefined4 __cdecl Audio_DecodeScalefactors(int *param_1,int *param_2)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  undefined4 uVar4;
  int *piVar5;
  uint uVar6;
  int iVar7;
  int aiStack_158 [64];
  undefined4 local_58;
  undefined4 local_54;
  int local_50;
  int *local_4c;
  int local_40;
  int *local_3c;
  int local_38;
  uint local_30;
  uint local_2c;
  short *local_28;
  int local_24;
  int local_20;
  uint local_1c;
  int local_18;
  int local_14;
  int local_10;
  uint local_c;
  int local_8;
  
  piVar3 = param_1;
  local_14 = param_1[0x5b];
  iVar1 = param_1[0x65];
  local_2c = 1 << ((byte)param_1[0x5d] & 0x1f);
  local_30 = -1 << ((byte)param_1[0x5d] & 0x1f);
  if (((param_1[0x3f] == 0) || (*(int *)(iVar1 + 0x34) != 0)) ||
     (uVar4 = BitStream_FlushAndReset(), (char)uVar4 != '\0')) {
    local_1c = 0;
    local_3c = param_1;
    local_58 = *(undefined4 *)param_1[5];
    local_54 = ((undefined4 *)param_1[5])[1];
    local_50 = param_1[0x5e];
    local_38 = iVar1 + 0x18;
    iVar7 = *(int *)(iVar1 + 0x14);
    local_18 = *param_2;
    local_10 = *(int *)(iVar1 + 0x20);
    local_20 = *(int *)(iVar1 + 0x48);
    local_8 = param_1[0x5a];
    param_1 = *(int **)(iVar1 + 0x10);
    if (local_10 == 0) {
      for (; local_8 <= local_14; local_8 = local_8 + 1) {
        if (iVar7 < 8) {
          uVar4 = BitStream_ReadBitsSSE2(&local_58,(uint)param_1,iVar7,0);
          if ((char)uVar4 == '\0') goto LAB_004b32dc;
          param_1 = local_4c;
          iVar7 = local_40;
          if (7 < local_40) goto LAB_004b304f;
          piVar5 = (int *)0x1;
LAB_004b307c:
          local_c = Huffman_DecodeValue(&local_58,(uint)param_1,iVar7,local_20,piVar5);
          if ((int)local_c < 0) goto LAB_004b32dc;
          param_1 = local_4c;
          iVar7 = local_40;
        }
        else {
LAB_004b304f:
          uVar6 = (int)param_1 >> ((char)iVar7 - 8U & 0x1f) & 0xff;
          iVar2 = *(int *)(local_20 + 0xd4 + uVar6 * 4);
          if (iVar2 == 0) {
            piVar5 = (int *)0x9;
            goto LAB_004b307c;
          }
          local_c = (uint)*(byte *)(uVar6 + 0x4d4 + local_20);
          iVar7 = iVar7 - iVar2;
        }
        local_24 = (int)local_c >> 4;
        local_c = local_c & 0xf;
        if (local_c == 0) {
          uVar6 = 0;
          if (local_24 != 0xf) {
            local_20 = 1 << ((byte)local_24 & 0x1f);
            local_10 = local_20;
            if (local_24 == 0) goto LAB_004b3214;
            if (iVar7 < local_24) {
              uVar4 = BitStream_ReadBitsSSE2(&local_58,(uint)param_1,iVar7,local_24);
              if ((char)uVar4 == '\0') goto LAB_004b32dc;
              param_1 = local_4c;
              iVar7 = local_40;
            }
            iVar7 = iVar7 - local_24;
            local_10 = local_10 + ((int)param_1 >> ((byte)iVar7 & 0x1f) & local_20 - 1U);
            goto LAB_004b3214;
          }
        }
        else {
          if (local_c != 1) {
            *(undefined4 *)(*piVar3 + 0x14) = 0x72;
            (**(code **)(*piVar3 + 4))(piVar3,0xffffffff);
          }
          if (iVar7 < 1) {
            uVar4 = BitStream_ReadBitsSSE2(&local_58,(uint)param_1,iVar7,1);
            if ((char)uVar4 == '\0') goto LAB_004b32dc;
            param_1 = local_4c;
            iVar7 = local_40;
          }
          iVar7 = iVar7 + -1;
          uVar6 = local_2c;
          if (((int)param_1 >> ((byte)iVar7 & 0x1f) & 1U) == 0) {
            uVar6 = local_30;
          }
        }
        do {
          local_c = uVar6;
          local_28 = (short *)(local_18 + (&DAT_004e65e0)[local_8] * 2);
          if (*local_28 == 0) {
            local_24 = local_24 + -1;
            if (local_24 < 0) break;
          }
          else {
            if (iVar7 < 1) {
              uVar4 = BitStream_ReadBitsSSE2(&local_58,(uint)param_1,iVar7,1);
              if ((char)uVar4 == '\0') goto LAB_004b32dc;
              param_1 = local_4c;
              iVar7 = local_40;
            }
            iVar7 = iVar7 + -1;
            if ((((int)param_1 >> ((byte)iVar7 & 0x1f) & 1U) != 0) &&
               ((local_2c & (int)*local_28) == 0)) {
              uVar6 = local_2c;
              if (*local_28 < 0) {
                uVar6 = local_30;
              }
              *local_28 = *local_28 + (short)uVar6;
            }
          }
          local_8 = local_8 + 1;
          uVar6 = local_c;
        } while (local_8 <= local_14);
        if (local_c != 0) {
          iVar2 = (&DAT_004e65e0)[local_8];
          *(short *)(local_18 + iVar2 * 2) = (short)local_c;
          piVar5 = aiStack_158 + local_1c;
          local_1c = local_1c + 1;
          *piVar5 = iVar2;
        }
      }
    }
    else {
LAB_004b3214:
      if (local_10 != 0) {
        for (; local_8 <= local_14; local_8 = local_8 + 1) {
          local_28 = (short *)(local_18 + (&DAT_004e65e0)[local_8] * 2);
          if (*local_28 != 0) {
            if (iVar7 < 1) {
              uVar4 = BitStream_ReadBitsSSE2(&local_58,(uint)param_1,iVar7,1);
              if ((char)uVar4 == '\0') {
LAB_004b32dc:
                for (; 0 < (int)local_1c; local_1c = local_1c - 1) {
                  *(undefined2 *)(local_18 + aiStack_158[local_1c - 1] * 2) = 0;
                }
                return local_1c & 0xffffff00;
              }
              param_1 = local_4c;
              iVar7 = local_40;
            }
            iVar7 = iVar7 + -1;
            if ((((int)param_1 >> ((byte)iVar7 & 0x1f) & 1U) != 0) &&
               ((local_2c & (int)*local_28) == 0)) {
              uVar6 = local_2c;
              if (*local_28 < 0) {
                uVar6 = local_30;
              }
              *local_28 = *local_28 + (short)uVar6;
            }
          }
        }
        local_10 = local_10 + -1;
      }
    }
    *(undefined4 *)piVar3[5] = local_58;
    *(undefined4 *)(piVar3[5] + 4) = local_54;
    piVar3[0x5e] = local_50;
    *(int *)(iVar1 + 0x34) = *(int *)(iVar1 + 0x34) + -1;
    *(int **)(iVar1 + 0x10) = param_1;
    *(int *)(iVar1 + 0x20) = local_10;
    *(int *)(iVar1 + 0x14) = iVar7;
    uVar4 = CONCAT31((int3)((uint)local_10 >> 8),1);
  }
  return uVar4;
}

