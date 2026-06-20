
void __cdecl Audio_CodecComputeScaling(int *param_1)

{
  int *piVar1;
  int *piVar2;
  int iVar3;
  int *piVar4;
  int iVar5;
  uint uVar6;
  int local_8;
  
  piVar2 = param_1;
  if (param_1[4] != 0xca) {
    *(undefined4 *)(*param_1 + 0x14) = 0x12;
    *(int *)(*param_1 + 0x18) = param_1[4];
    (**(code **)*param_1)(param_1);
  }
  iVar3 = param_1[0xb];
  uVar6 = param_1[0xc];
  if (uVar6 < (uint)(iVar3 << 3)) {
    if (uVar6 < (uint)(iVar3 << 2)) {
      if (uVar6 < (uint)(iVar3 * 2)) {
        param_1[0x17] = param_1[6];
        iVar3 = param_1[7];
        param_1[0x45] = 8;
      }
      else {
        iVar3 = DivCeil(param_1[6],2);
        param_1[0x17] = iVar3;
        iVar3 = DivCeil(param_1[7],2);
        param_1[0x45] = 4;
      }
    }
    else {
      iVar3 = DivCeil(param_1[6],4);
      param_1[0x17] = iVar3;
      iVar3 = DivCeil(param_1[7],4);
      param_1[0x45] = 2;
    }
  }
  else {
    iVar3 = DivCeil(param_1[6],8);
    param_1[0x17] = iVar3;
    iVar3 = DivCeil(param_1[7],8);
    param_1[0x45] = 1;
  }
  local_8 = 0;
  param_1[0x18] = iVar3;
  if (0 < param_1[8]) {
    piVar4 = (int *)(param_1[0x31] + 0xc);
    do {
      piVar1 = (int *)piVar2[0x45];
      param_1 = piVar1;
      if ((int)piVar1 < 8) {
        do {
          iVar3 = piVar4[-1] * (int)param_1 * 2;
          if ((iVar3 - piVar2[0x43] * (int)piVar1 != 0 && piVar2[0x43] * (int)piVar1 <= iVar3) ||
             (iVar3 = *piVar4 * (int)param_1 * 2,
             iVar3 - piVar2[0x44] * (int)piVar1 != 0 && piVar2[0x44] * (int)piVar1 <= iVar3)) break;
          param_1 = (int *)((int)param_1 * 2);
        } while ((int)param_1 < 8);
      }
      local_8 = local_8 + 1;
      piVar4[6] = (int)param_1;
      piVar4 = piVar4 + 0x15;
    } while (local_8 < piVar2[8]);
  }
  iVar3 = 0;
  if (0 < piVar2[8]) {
    piVar4 = (int *)(piVar2[0x31] + 0x24);
    do {
      iVar5 = DivCeil(piVar4[-7] * *piVar4 * piVar2[6],piVar2[0x43] << 3);
      piVar4[1] = iVar5;
      iVar5 = DivCeil(piVar4[-6] * piVar2[7] * *piVar4,piVar2[0x44] << 3);
      piVar4[2] = iVar5;
      iVar3 = iVar3 + 1;
      piVar4 = piVar4 + 0x15;
    } while (iVar3 < piVar2[8]);
  }
  iVar3 = piVar2[10];
  iVar5 = 1;
  if (iVar3 == 1) {
LAB_004acc79:
    piVar2[0x19] = iVar5;
  }
  else {
    if (1 < iVar3) {
      iVar5 = 3;
      if (iVar3 < 4) goto LAB_004acc79;
      if (iVar3 < 6) {
        piVar2[0x19] = 4;
        goto LAB_004acc7c;
      }
    }
    piVar2[0x19] = piVar2[8];
  }
LAB_004acc7c:
  if (*(char *)((int)piVar2 + 0x4a) == '\0') {
    iVar3 = piVar2[0x19];
  }
  else {
    iVar3 = 1;
  }
  piVar2[0x1a] = iVar3;
  uVar6 = Audio_CodecValidateConfig(iVar5,(int)piVar2);
  if ((char)uVar6 == '\0') {
    piVar2[0x1b] = 1;
  }
  else {
    piVar2[0x1b] = piVar2[0x44];
  }
  return;
}

