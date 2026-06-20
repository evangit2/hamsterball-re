
undefined4 __cdecl Audio_DecodeLayer3Mono(uint param_1,int *param_2)

{
  int iVar1;
  uint uVar2;
  undefined4 uVar3;
  int iVar4;
  int *piVar5;
  uint uVar6;
  uint uVar7;
  int iVar8;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  uint local_40;
  int local_34;
  uint local_30;
  int local_2c;
  int local_24;
  undefined4 local_20;
  int local_1c;
  int local_18;
  int local_14;
  uint local_10;
  int local_c;
  int local_8;
  
  uVar2 = param_1;
  local_1c = *(int *)(param_1 + 0x16c);
  iVar1 = *(int *)(param_1 + 0x194);
  local_20 = *(undefined4 *)(param_1 + 0x174);
  if (((*(int *)(param_1 + 0xfc) == 0) || (*(int *)(iVar1 + 0x34) != 0)) ||
     (uVar3 = BitStream_FlushAndReset(), (char)uVar3 != '\0')) {
    if (*(int *)(iVar1 + 0x20) == 0) {
      local_30 = param_1;
      local_4c = **(undefined4 **)(param_1 + 0x14);
      local_48 = (*(undefined4 **)(param_1 + 0x14))[1];
      local_44 = *(undefined4 *)(param_1 + 0x178);
      uVar7 = *(uint *)(iVar1 + 0x10);
      iVar4 = *(int *)(iVar1 + 0x14);
      local_2c = iVar1 + 0x18;
      local_24 = *param_2;
      local_18 = *(int *)(iVar1 + 0x48);
      local_c = 0;
      local_10 = uVar7;
      for (local_8 = *(int *)(param_1 + 0x168); local_8 <= local_1c; local_8 = local_8 + 1) {
        if (iVar4 < 8) {
          param_1 = BitStream_ReadBitsSSE2(&local_4c,uVar7,iVar4,0);
          if ((char)param_1 == '\0') goto LAB_004b2e1a;
          local_10 = local_40;
          iVar4 = local_34;
          uVar7 = local_40;
          if (7 < local_34) goto LAB_004b2d30;
          piVar5 = (int *)0x1;
LAB_004b2d60:
          param_1 = Huffman_DecodeValue(&local_4c,uVar7,iVar4,local_18,piVar5);
          if ((int)param_1 < 0) goto LAB_004b2e1a;
          local_10 = local_40;
          iVar4 = local_34;
          uVar7 = local_40;
        }
        else {
LAB_004b2d30:
          uVar6 = (int)uVar7 >> ((char)iVar4 - 8U & 0x1f) & 0xff;
          iVar8 = *(int *)(local_18 + 0xd4 + uVar6 * 4);
          if (iVar8 == 0) {
            piVar5 = (int *)0x9;
            goto LAB_004b2d60;
          }
          iVar4 = iVar4 - iVar8;
          param_1 = (uint)*(byte *)(uVar6 + 0x4d4 + local_18);
        }
        local_14 = (int)param_1 >> 4;
        uVar6 = param_1 & 0xf;
        if (uVar6 == 0) {
          if (local_14 != 0xf) {
            iVar8 = 1 << ((byte)local_14 & 0x1f);
            local_c = iVar8;
            if (local_14 != 0) {
              if ((iVar4 < local_14) &&
                 (param_1 = BitStream_ReadBitsSSE2(&local_4c,uVar7,iVar4,local_14), iVar4 = local_34
                 , uVar7 = local_40, (char)param_1 == '\0')) goto LAB_004b2e1a;
              iVar4 = iVar4 - local_14;
              local_c = local_c + ((int)uVar7 >> ((byte)iVar4 & 0x1f) & iVar8 - 1U);
            }
            local_c = local_c + -1;
            break;
          }
          local_8 = local_8 + 0xf;
        }
        else {
          local_8 = local_8 + local_14;
          if (iVar4 < (int)uVar6) {
            param_1 = BitStream_ReadBitsSSE2(&local_4c,uVar7,iVar4,uVar6);
            if ((char)param_1 == '\0') {
LAB_004b2e1a:
              return param_1 & 0xffffff00;
            }
            local_10 = local_40;
            iVar4 = local_34;
            uVar7 = local_40;
          }
          iVar4 = iVar4 - uVar6;
          uVar7 = (1 << (sbyte)uVar6) - 1U & (int)uVar7 >> ((byte)iVar4 & 0x1f);
          if ((int)uVar7 < *(int *)(&DAT_004e91a0 + uVar6 * 4)) {
            uVar7 = *(int *)(&DAT_004e91e0 + uVar6 * 4) + uVar7;
          }
          *(short *)(local_24 + (&DAT_004e65e0)[local_8] * 2) =
               (short)(uVar7 << ((byte)local_20 & 0x1f));
          uVar7 = local_10;
        }
      }
      **(undefined4 **)(uVar2 + 0x14) = local_4c;
      *(undefined4 *)(*(int *)(uVar2 + 0x14) + 4) = local_48;
      *(undefined4 *)(uVar2 + 0x178) = local_44;
      *(int *)(iVar1 + 0x14) = iVar4;
      *(uint *)(iVar1 + 0x10) = uVar7;
    }
    else {
      local_c = *(int *)(iVar1 + 0x20) + -1;
    }
    *(int *)(iVar1 + 0x34) = *(int *)(iVar1 + 0x34) + -1;
    *(int *)(iVar1 + 0x20) = local_c;
    uVar3 = CONCAT31((int3)((uint)local_c >> 8),1);
  }
  return uVar3;
}

