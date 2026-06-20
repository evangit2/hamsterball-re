
uint __cdecl MIDI_ReadChunkData(int *param_1)

{
  int iVar1;
  byte bVar2;
  undefined4 *puVar3;
  int *piVar4;
  int *piVar5;
  uint uVar6;
  undefined4 uVar7;
  byte *pbVar8;
  
  piVar4 = param_1;
  puVar3 = (undefined4 *)param_1[5];
  pbVar8 = (byte *)*puVar3;
  piVar5 = (int *)puVar3[1];
  if ((int *)puVar3[1] == (int *)0x0) {
    uVar6 = (*(code *)puVar3[3])(param_1);
    if ((char)uVar6 != '\0') {
      pbVar8 = (byte *)*puVar3;
      piVar5 = (int *)puVar3[1];
      goto LAB_004a3dca;
    }
LAB_004a3de4:
    uVar6 = uVar6 & 0xffffff00;
  }
  else {
LAB_004a3dca:
    param_1 = piVar5;
    param_1 = (int *)((int)param_1 + -1);
    bVar2 = *pbVar8;
    pbVar8 = pbVar8 + 1;
    if (param_1 == (int *)0x0) {
      uVar6 = (*(code *)puVar3[3])(piVar4);
      if ((char)uVar6 == '\0') goto LAB_004a3de4;
      param_1 = (int *)puVar3[1];
      pbVar8 = (byte *)*puVar3;
    }
    iVar1 = (uint)bVar2 * 0x100 + (uint)*pbVar8;
    *(undefined4 *)(*piVar4 + 0x14) = 0x5a;
    *(int *)(*piVar4 + 0x18) = piVar4[0x5e];
    *(int *)(*piVar4 + 0x1c) = iVar1;
    (**(code **)(*piVar4 + 4))(piVar4,1);
    *puVar3 = pbVar8 + 1;
    puVar3[1] = (int)param_1 + -1;
    uVar7 = (**(code **)(piVar4[5] + 0x10))(piVar4,iVar1 + -2);
    uVar6 = CONCAT31((int3)((uint)uVar7 >> 8),1);
  }
  return uVar6;
}

