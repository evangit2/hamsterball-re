
uint MIDI_ParseInstrument(void)

{
  byte bVar1;
  uint *puVar2;
  uint uVar3;
  int *unaff_EBX;
  byte *pbVar4;
  int *piVar5;
  byte *pbVar6;
  int iVar7;
  undefined4 *puVar8;
  byte local_134 [256];
  uint local_34;
  int *local_30;
  uint local_2c;
  uint local_28;
  uint local_24;
  uint local_20;
  byte local_1c;
  int local_18;
  int *local_14;
  int local_10;
  byte *local_c;
  int local_8;
  
  piVar5 = (int *)unaff_EBX[5];
  local_8 = piVar5[1];
  pbVar6 = (byte *)*piVar5;
  local_30 = piVar5;
  if (local_8 == 0) {
    uVar3 = (*(code *)piVar5[3])();
    if ((char)uVar3 != '\0') {
      pbVar6 = (byte *)*piVar5;
      local_8 = piVar5[1];
      local_c = pbVar6;
      goto LAB_004a3844;
    }
LAB_004a3a9c:
    uVar3 = uVar3 & 0xffffff00;
  }
  else {
LAB_004a3844:
    local_8 = local_8 + -1;
    local_10 = (uint)*pbVar6 << 8;
    pbVar6 = pbVar6 + 1;
    if (local_8 == 0) {
      uVar3 = (*(code *)piVar5[3])();
      if ((char)uVar3 == '\0') goto LAB_004a3a9c;
      pbVar6 = (byte *)*piVar5;
      local_8 = piVar5[1];
      local_c = pbVar6;
    }
    local_8 = local_8 + -1;
    local_10 = local_10 + (uint)*pbVar6 + -2;
    pbVar6 = pbVar6 + 1;
    while (0 < local_10) {
      if (local_8 == 0) {
        uVar3 = (*(code *)piVar5[3])();
        if ((char)uVar3 == '\0') goto LAB_004a3a9c;
        pbVar6 = (byte *)*piVar5;
        local_8 = piVar5[1];
        local_c = pbVar6;
      }
      local_34 = (uint)*pbVar6;
      local_8 = local_8 + -1;
      *(undefined4 *)(*unaff_EBX + 0x14) = 0x4f;
      *(uint *)(*unaff_EBX + 0x18) = local_34;
      pbVar6 = pbVar6 + 1;
      (**(code **)(*unaff_EBX + 4))();
      local_2c = local_2c & 0xffffff00;
      local_18 = 0;
      local_14 = (int *)0x1;
      do {
        if (local_8 == 0) {
          uVar3 = (*(code *)piVar5[3])();
          if ((char)uVar3 == '\0') goto LAB_004a3a9c;
          pbVar6 = (byte *)*piVar5;
          local_8 = piVar5[1];
        }
        bVar1 = *pbVar6;
        local_8 = local_8 + -1;
        *(byte *)((int)&local_2c + (int)local_14) = bVar1;
        local_18 = local_18 + (uint)bVar1;
        pbVar6 = pbVar6 + 1;
        local_14 = (int *)((int)local_14 + 1);
        local_c = pbVar6;
      } while ((int)local_14 < 0x11);
      iVar7 = *unaff_EBX;
      *(uint *)(iVar7 + 0x18) = local_2c >> 8 & 0xff;
      *(uint *)(iVar7 + 0x1c) = local_2c >> 0x10 & 0xff;
      *(uint *)(iVar7 + 0x20) = local_2c >> 0x18;
      local_10 = local_10 + -0x11;
      *(uint *)(iVar7 + 0x24) = local_28 & 0xff;
      *(uint *)(iVar7 + 0x28) = local_28 >> 8 & 0xff;
      *(uint *)(iVar7 + 0x2c) = local_28 >> 0x10 & 0xff;
      *(uint *)(iVar7 + 0x30) = local_28 >> 0x18;
      *(uint *)(iVar7 + 0x34) = local_24 & 0xff;
      *(undefined4 *)(*unaff_EBX + 0x14) = 0x55;
      (**(code **)(*unaff_EBX + 4))();
      iVar7 = *unaff_EBX;
      *(uint *)(iVar7 + 0x18) = local_24 >> 8 & 0xff;
      *(uint *)(iVar7 + 0x1c) = local_24 >> 0x10 & 0xff;
      *(uint *)(iVar7 + 0x20) = local_24 >> 0x18;
      *(uint *)(iVar7 + 0x24) = local_20 & 0xff;
      *(uint *)(iVar7 + 0x28) = local_20 >> 8 & 0xff;
      *(uint *)(iVar7 + 0x2c) = local_20 >> 0x10 & 0xff;
      *(uint *)(iVar7 + 0x30) = local_20 >> 0x18;
      *(uint *)(iVar7 + 0x34) = (uint)local_1c;
      *(undefined4 *)(*unaff_EBX + 0x14) = 0x55;
      (**(code **)(*unaff_EBX + 4))();
      if ((0x100 < local_18) || (local_10 < local_18)) {
        *(undefined4 *)(*unaff_EBX + 0x14) = 0x1c;
        (**(code **)*unaff_EBX)();
      }
      iVar7 = 0;
      if (0 < local_18) {
        do {
          if (local_8 == 0) {
            uVar3 = (*(code *)piVar5[3])();
            if ((char)uVar3 == '\0') goto LAB_004a3a9c;
            local_c = (byte *)*piVar5;
            local_8 = piVar5[1];
          }
          local_8 = local_8 + -1;
          bVar1 = *local_c;
          local_c = local_c + 1;
          local_134[iVar7] = bVar1;
          iVar7 = iVar7 + 1;
        } while (iVar7 < local_18);
      }
      local_10 = local_10 - local_18;
      if ((local_34 & 0x10) == 0) {
        piVar5 = unaff_EBX + local_34 + 0x28;
        local_14 = piVar5;
        uVar3 = local_34;
      }
      else {
        piVar5 = unaff_EBX + local_34 + 0x1c;
        local_14 = piVar5;
        uVar3 = local_34 - 0x10;
      }
      local_14 = piVar5;
      if (((int)uVar3 < 0) || (3 < (int)uVar3)) {
        *(undefined4 *)(*unaff_EBX + 0x14) = 0x1d;
        *(uint *)(*unaff_EBX + 0x18) = uVar3;
        (**(code **)*unaff_EBX)();
      }
      if (*piVar5 == 0) {
        iVar7 = Audio_CodecAllocStr256((int)unaff_EBX);
        *piVar5 = iVar7;
      }
      puVar2 = (uint *)*piVar5;
      *puVar2 = local_2c;
      puVar2[1] = local_28;
      puVar2[2] = local_24;
      puVar2[3] = local_20;
      *(byte *)(puVar2 + 4) = local_1c;
      pbVar4 = local_134;
      puVar8 = (undefined4 *)(*local_14 + 0x11);
      for (iVar7 = 0x40; piVar5 = local_30, pbVar6 = local_c, iVar7 != 0; iVar7 = iVar7 + -1) {
        *puVar8 = *(undefined4 *)pbVar4;
        pbVar4 = pbVar4 + 4;
        puVar8 = puVar8 + 1;
      }
    }
    piVar5[1] = local_8;
    *piVar5 = (int)pbVar6;
    uVar3 = CONCAT31((int3)((uint)local_8 >> 8),1);
  }
  return uVar3;
}

