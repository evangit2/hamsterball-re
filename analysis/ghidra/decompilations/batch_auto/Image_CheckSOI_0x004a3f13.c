
uint __cdecl Image_CheckSOI(int *param_1)

{
  byte bVar1;
  undefined4 *puVar2;
  uint uVar3;
  int iVar4;
  byte *pbVar5;
  
  puVar2 = (undefined4 *)param_1[5];
  iVar4 = puVar2[1];
  pbVar5 = (byte *)*puVar2;
  if (iVar4 == 0) {
    uVar3 = (*(code *)puVar2[3])(param_1);
    if ((char)uVar3 != '\0') {
      pbVar5 = (byte *)*puVar2;
      iVar4 = puVar2[1];
      goto LAB_004a3f3a;
    }
LAB_004a3f4f:
    uVar3 = uVar3 & 0xffffff00;
  }
  else {
LAB_004a3f3a:
    bVar1 = *pbVar5;
    iVar4 = iVar4 + -1;
    pbVar5 = pbVar5 + 1;
    if (iVar4 == 0) {
      uVar3 = (*(code *)puVar2[3])(param_1);
      if ((char)uVar3 == '\0') goto LAB_004a3f4f;
      pbVar5 = (byte *)*puVar2;
      iVar4 = puVar2[1];
    }
    uVar3 = (uint)*pbVar5;
    if ((bVar1 != 0xff) || (uVar3 != 0xd8)) {
      *(undefined4 *)(*param_1 + 0x14) = 0x34;
      *(uint *)(*param_1 + 0x18) = (uint)bVar1;
      *(uint *)(*param_1 + 0x1c) = uVar3;
      (**(code **)*param_1)(param_1);
    }
    param_1[0x5e] = uVar3;
    *puVar2 = pbVar5 + 1;
    puVar2[1] = iVar4 + -1;
    uVar3 = CONCAT31((int3)((uint)param_1 >> 8),1);
  }
  return uVar3;
}

