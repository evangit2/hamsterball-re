
uint __cdecl Audio_ReadSFBZeros(int param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  ushort *puVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  int iVar7;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  uint local_24;
  int local_18;
  int local_14;
  int local_10;
  int local_8;
  
  iVar4 = param_1;
  iVar2 = *(int *)(param_1 + 0x194);
  local_8 = 1 << ((byte)*(undefined4 *)(param_1 + 0x174) & 0x1f);
  if (((*(int *)(param_1 + 0xfc) == 0) || (*(int *)(iVar2 + 0x34) != 0)) ||
     (uVar5 = BitStream_FlushAndReset(), (char)uVar5 != '\0')) {
    piVar1 = (int *)(param_1 + 0x13c);
    local_14 = param_1;
    local_30 = **(undefined4 **)(param_1 + 0x14);
    local_2c = (*(undefined4 **)(param_1 + 0x14))[1];
    local_28 = *(undefined4 *)(param_1 + 0x178);
    uVar6 = *(uint *)(iVar2 + 0x10);
    iVar7 = *(int *)(iVar2 + 0x14);
    local_10 = iVar2 + 0x18;
    param_1 = 0;
    if (0 < *piVar1) {
      do {
        puVar3 = *(ushort **)(param_2 + param_1 * 4);
        if ((iVar7 < 1) &&
           (uVar5 = BitStream_ReadBitsSSE2(&local_30,uVar6,iVar7,1), uVar6 = local_24,
           iVar7 = local_18, (char)uVar5 == '\0')) goto LAB_004b2f72;
        iVar7 = iVar7 + -1;
        if (((int)uVar6 >> ((byte)iVar7 & 0x1f) & 1U) != 0) {
          *puVar3 = *puVar3 | (ushort)local_8;
        }
        param_1 = param_1 + 1;
      } while (param_1 < *(int *)(iVar4 + 0x13c));
    }
    **(undefined4 **)(iVar4 + 0x14) = local_30;
    *(undefined4 *)(*(int *)(iVar4 + 0x14) + 4) = local_2c;
    *(undefined4 *)(iVar4 + 0x178) = local_28;
    *(int *)(iVar2 + 0x34) = *(int *)(iVar2 + 0x34) + -1;
    *(uint *)(iVar2 + 0x10) = uVar6;
    *(int *)(iVar2 + 0x14) = iVar7;
    uVar5 = CONCAT31((int3)(uVar6 >> 8),1);
  }
  else {
LAB_004b2f72:
    uVar5 = uVar5 & 0xffffff00;
  }
  return uVar5;
}

