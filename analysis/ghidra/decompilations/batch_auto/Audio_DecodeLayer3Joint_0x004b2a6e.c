
undefined4 __cdecl Audio_DecodeLayer3Joint(uint param_1,int param_2)

{
  uint uVar1;
  undefined4 uVar2;
  uint uVar3;
  int *piVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  uint uVar9;
  int *piVar10;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  uint local_50;
  int local_44;
  uint local_40;
  int local_3c;
  int local_34 [5];
  undefined2 *local_20;
  undefined4 local_1c;
  int local_18;
  int *local_14;
  int local_10;
  int local_c;
  uint local_8;
  
  uVar1 = param_1;
  local_c = *(int *)(param_1 + 0x194);
  local_1c = *(undefined4 *)(param_1 + 0x174);
  if (((*(int *)(param_1 + 0xfc) == 0) || (*(int *)(*(int *)(param_1 + 0x194) + 0x34) != 0)) ||
     (uVar2 = BitStream_FlushAndReset(), (char)uVar2 != '\0')) {
    iVar5 = local_c;
    local_10 = 0;
    iVar7 = *(int *)(param_1 + 0x13c);
    local_40 = param_1;
    local_5c = **(undefined4 **)(param_1 + 0x14);
    local_58 = (*(undefined4 **)(param_1 + 0x14))[1];
    local_54 = *(undefined4 *)(param_1 + 0x178);
    uVar3 = *(uint *)(local_c + 0x10);
    iVar8 = *(int *)(local_c + 0x14);
    local_3c = local_c + 0x18;
    piVar4 = (int *)(local_c + 0x20);
    piVar10 = local_34;
    for (iVar6 = 5; iVar6 != 0; iVar6 = iVar6 + -1) {
      *piVar10 = *piVar4;
      piVar4 = piVar4 + 1;
      piVar10 = piVar10 + 1;
    }
    if (0 < iVar7) {
      local_14 = (int *)(param_1 + 0x140);
      param_1 = uVar3;
      do {
        local_20 = *(undefined2 **)(param_2 + local_10 * 4);
        local_18 = *local_14;
        iVar7 = *(int *)(iVar5 + 0x38 + *(int *)(*(int *)(uVar1 + 0x124 + *local_14 * 4) + 0x14) * 4
                        );
        if (iVar8 < 8) {
          uVar3 = BitStream_ReadBitsSSE2(&local_5c,param_1,iVar8,0);
          if ((char)uVar3 == '\0') goto LAB_004b2c67;
          param_1 = local_50;
          iVar5 = local_c;
          iVar8 = local_44;
          if (7 < local_44) goto LAB_004b2b52;
          piVar4 = (int *)0x1;
LAB_004b2b7a:
          uVar3 = Huffman_DecodeValue(&local_5c,param_1,iVar8,iVar7,piVar4);
          if ((int)uVar3 < 0) goto LAB_004b2c67;
          param_1 = local_50;
          iVar5 = local_c;
          iVar8 = local_44;
          uVar9 = uVar3;
        }
        else {
LAB_004b2b52:
          uVar3 = (int)param_1 >> ((char)iVar8 - 8U & 0x1f) & 0xff;
          iVar6 = *(int *)(iVar7 + 0xd4 + uVar3 * 4);
          if (iVar6 == 0) {
            piVar4 = (int *)0x9;
            goto LAB_004b2b7a;
          }
          uVar9 = (uint)*(byte *)(uVar3 + 0x4d4 + iVar7);
          iVar8 = iVar8 - iVar6;
        }
        uVar3 = 0;
        if (uVar9 != 0) {
          if (iVar8 < (int)uVar9) {
            uVar3 = BitStream_ReadBitsSSE2(&local_5c,param_1,iVar8,uVar9);
            if ((char)uVar3 == '\0') {
LAB_004b2c67:
              return uVar3 & 0xffffff00;
            }
            param_1 = local_50;
            iVar5 = local_c;
            iVar8 = local_44;
          }
          iVar8 = iVar8 - uVar9;
          local_8 = (1 << ((byte)uVar9 & 0x1f)) - 1;
          uVar3 = local_8 & (int)param_1 >> ((byte)iVar8 & 0x1f);
          if ((int)uVar3 < *(int *)(&DAT_004e91a0 + uVar9 * 4)) {
            uVar3 = *(int *)(&DAT_004e91e0 + uVar9 * 4) + uVar3;
          }
        }
        local_14 = local_14 + 1;
        piVar4 = local_34 + local_18 + 1;
        *piVar4 = *piVar4 + uVar3;
        local_10 = local_10 + 1;
        *local_20 = (short)(*piVar4 << ((byte)local_1c & 0x1f));
        uVar3 = param_1;
      } while (local_10 < *(int *)(uVar1 + 0x13c));
    }
    param_1 = uVar3;
    **(undefined4 **)(uVar1 + 0x14) = local_5c;
    *(undefined4 *)(*(int *)(uVar1 + 0x14) + 4) = local_58;
    *(undefined4 *)(uVar1 + 0x178) = local_54;
    *(int *)(iVar5 + 0x34) = *(int *)(iVar5 + 0x34) + -1;
    *(uint *)(iVar5 + 0x10) = param_1;
    *(int *)(iVar5 + 0x14) = iVar8;
    piVar4 = local_34;
    piVar10 = (int *)(iVar5 + 0x20);
    for (iVar7 = 5; iVar7 != 0; iVar7 = iVar7 + -1) {
      *piVar10 = *piVar4;
      piVar4 = piVar4 + 1;
      piVar10 = piVar10 + 1;
    }
    uVar2 = CONCAT31((int3)((uint)iVar5 >> 8),1);
  }
  return uVar2;
}

