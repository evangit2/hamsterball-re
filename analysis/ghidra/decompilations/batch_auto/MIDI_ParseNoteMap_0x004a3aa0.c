
uint MIDI_ParseNoteMap(void)

{
  int *piVar1;
  byte bVar2;
  undefined4 *puVar3;
  ushort uVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  ushort *puVar8;
  int *unaff_EBX;
  byte *pbVar9;
  byte *pbVar10;
  int *local_14;
  int local_10;
  int local_c;
  int local_8;
  
  puVar3 = (undefined4 *)unaff_EBX[5];
  local_8 = puVar3[1];
  pbVar9 = (byte *)*puVar3;
  if (local_8 == 0) {
    uVar5 = (*(code *)puVar3[3])();
    if ((char)uVar5 != '\0') {
      local_8 = puVar3[1];
      pbVar9 = (byte *)*puVar3;
      goto LAB_004a3acc;
    }
LAB_004a3cc5:
    uVar5 = uVar5 & 0xffffff00;
  }
  else {
LAB_004a3acc:
    local_8 = local_8 + -1;
    bVar2 = *pbVar9;
    pbVar9 = pbVar9 + 1;
    if (local_8 == 0) {
      uVar5 = (*(code *)puVar3[3])();
      if ((char)uVar5 == '\0') goto LAB_004a3cc5;
      local_8 = puVar3[1];
      pbVar9 = (byte *)*puVar3;
    }
    local_8 = local_8 + -1;
    pbVar10 = pbVar9 + 1;
    iVar7 = (uint)bVar2 * 0x100 + (uint)*pbVar9 + -2;
    while (local_c = iVar7, 0 < local_c) {
      if (local_8 == 0) {
        uVar5 = (*(code *)puVar3[3])();
        if ((char)uVar5 == '\0') goto LAB_004a3cc5;
        local_8 = puVar3[1];
        pbVar10 = (byte *)*puVar3;
      }
      bVar2 = *pbVar10;
      local_8 = local_8 + -1;
      *(undefined4 *)(*unaff_EBX + 0x14) = 0x50;
      uVar5 = bVar2 & 0xf;
      *(uint *)(*unaff_EBX + 0x18) = uVar5;
      iVar6 = (int)(uint)bVar2 >> 4;
      *(int *)(*unaff_EBX + 0x1c) = iVar6;
      pbVar10 = pbVar10 + 1;
      (**(code **)(*unaff_EBX + 4))();
      if (3 < uVar5) {
        *(undefined4 *)(*unaff_EBX + 0x14) = 0x1e;
        *(uint *)(*unaff_EBX + 0x18) = uVar5;
        (**(code **)*unaff_EBX)();
      }
      piVar1 = unaff_EBX + uVar5 + 0x24;
      if (*piVar1 == 0) {
        iVar7 = Audio_CodecAllocStr128((int)unaff_EBX);
        *piVar1 = iVar7;
      }
      iVar7 = *piVar1;
      local_14 = &DAT_004e65e0;
      do {
        if (iVar6 == 0) {
          if (local_8 == 0) {
            uVar5 = (*(code *)puVar3[3])();
            if ((char)uVar5 == '\0') goto LAB_004a3cc5;
            local_8 = puVar3[1];
            pbVar10 = (byte *)*puVar3;
          }
          uVar4 = (ushort)*pbVar10;
        }
        else {
          if (local_8 == 0) {
            uVar5 = (*(code *)puVar3[3])();
            if ((char)uVar5 == '\0') goto LAB_004a3cc5;
            local_8 = puVar3[1];
            pbVar10 = (byte *)*puVar3;
          }
          local_8 = local_8 + -1;
          bVar2 = *pbVar10;
          pbVar10 = pbVar10 + 1;
          if (local_8 == 0) {
            uVar5 = (*(code *)puVar3[3])();
            if ((char)uVar5 == '\0') goto LAB_004a3cc5;
            local_8 = puVar3[1];
            pbVar10 = (byte *)*puVar3;
          }
          uVar4 = (ushort)bVar2 * 0x100 + (ushort)*pbVar10;
        }
        local_8 = local_8 + -1;
        piVar1 = local_14 + 1;
        pbVar10 = pbVar10 + 1;
        *(ushort *)(iVar7 + *local_14 * 2) = uVar4;
        local_14 = piVar1;
      } while ((int)piVar1 < 0x4e66e0);
      if (1 < *(int *)(*unaff_EBX + 0x68)) {
        puVar8 = (ushort *)(iVar7 + 4);
        local_10 = 8;
        do {
          iVar7 = *unaff_EBX;
          *(uint *)(iVar7 + 0x18) = (uint)puVar8[-2];
          *(uint *)(iVar7 + 0x1c) = (uint)puVar8[-1];
          *(uint *)(iVar7 + 0x20) = (uint)*puVar8;
          *(uint *)(iVar7 + 0x24) = (uint)puVar8[1];
          *(uint *)(iVar7 + 0x28) = (uint)puVar8[2];
          *(uint *)(iVar7 + 0x2c) = (uint)puVar8[3];
          *(uint *)(iVar7 + 0x30) = (uint)puVar8[4];
          *(uint *)(iVar7 + 0x34) = (uint)puVar8[5];
          *(undefined4 *)(*unaff_EBX + 0x14) = 0x5c;
          (**(code **)(*unaff_EBX + 4))();
          puVar8 = puVar8 + 8;
          local_10 = local_10 + -1;
        } while (local_10 != 0);
      }
      iVar7 = local_c + -0x41;
      if (iVar6 != 0) {
        iVar7 = local_c + -0x81;
      }
    }
    puVar3[1] = local_8;
    *puVar3 = pbVar10;
    uVar5 = CONCAT31((int3)((uint)local_8 >> 8),1);
  }
  return uVar5;
}

