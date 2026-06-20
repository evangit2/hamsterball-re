
void CRT_FormatFloatOutput(int param_1,char param_2,undefined4 *param_3,short param_4,int param_5)

{
  int *piVar1;
  short sVar2;
  undefined4 *puVar3;
  uint uVar4;
  int iVar5;
  char extraout_DL;
  short sVar6;
  uint uVar7;
  int iVar8;
  undefined4 *puVar9;
  char *pcVar10;
  char *pcVar11;
  char local_d [13];
  
  puVar3 = (undefined4 *)CRT_GetLocalePtr();
  local_d[0] = *(char *)*puVar3;
  if (param_4 < 1) {
    param_4 = 1;
    param_3 = &DAT_004d435c;
  }
  sVar2 = (short)param_5;
  if (((param_2 == 'f') || (param_2 == 'F')) ||
     (((param_2 == 'g' || (param_2 == 'G')) &&
      ((-5 < sVar2 && ((int)sVar2 < *(int *)(param_1 + 0x24))))))) {
    sVar2 = sVar2 + 1;
    if ((param_2 != 'f') && (param_2 != 'F')) {
      if (((*(byte *)(param_1 + 0x30) & 8) == 0) && ((int)param_4 < *(int *)(param_1 + 0x24))) {
        *(int *)(param_1 + 0x24) = (int)param_4;
      }
      iVar8 = *(int *)(param_1 + 0x24) - (int)sVar2;
      *(int *)(param_1 + 0x24) = iVar8;
      if (iVar8 < 0) {
        *(undefined4 *)(param_1 + 0x24) = 0;
      }
    }
    if (sVar2 < 1) {
      *(undefined1 *)(*(int *)(param_1 + 8) + *(int *)(param_1 + 0x14)) = 0x30;
      iVar8 = *(int *)(param_1 + 0x14) + 1;
      *(int *)(param_1 + 0x14) = iVar8;
      if ((0 < *(int *)(param_1 + 0x24)) || ((*(byte *)(param_1 + 0x30) & 8) != 0)) {
        *(char *)(iVar8 + *(int *)(param_1 + 8)) = local_d[0];
        iVar8 = *(int *)(param_1 + 0x14) + 1;
        *(int *)(param_1 + 0x14) = iVar8;
      }
      if (*(int *)(param_1 + 0x24) < -(int)sVar2) {
        sVar2 = -(short)*(int *)(param_1 + 0x24);
      }
      *(int *)(param_1 + 0x18) = -(int)sVar2;
      iVar5 = *(int *)(param_1 + 0x24) + (int)sVar2;
      *(int *)(param_1 + 0x24) = iVar5;
      if (iVar5 < param_4) {
        param_4 = (short)iVar5;
      }
      uVar4 = (uint)param_4;
      *(uint *)(param_1 + 0x1c) = uVar4;
      puVar3 = (undefined4 *)(*(int *)(param_1 + 8) + iVar8);
      for (uVar7 = uVar4 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
        *puVar3 = *param_3;
        param_3 = param_3 + 1;
        puVar3 = puVar3 + 1;
      }
      for (uVar7 = uVar4 & 3; uVar7 != 0; uVar7 = uVar7 - 1) {
        *(undefined1 *)puVar3 = *(undefined1 *)param_3;
        param_3 = (undefined4 *)((int)param_3 + 1);
        puVar3 = (undefined4 *)((int)puVar3 + 1);
      }
      *(uint *)(param_1 + 0x20) = *(int *)(param_1 + 0x24) - uVar4;
    }
    else {
      puVar3 = (undefined4 *)(*(int *)(param_1 + 8) + *(int *)(param_1 + 0x14));
      if (param_4 < sVar2) {
        uVar7 = (uint)param_4;
        for (uVar4 = uVar7 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
          *puVar3 = *param_3;
          param_3 = param_3 + 1;
          puVar3 = puVar3 + 1;
        }
        for (uVar4 = uVar7 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
          *(undefined1 *)puVar3 = *(undefined1 *)param_3;
          param_3 = (undefined4 *)((int)param_3 + 1);
          puVar3 = (undefined4 *)((int)puVar3 + 1);
        }
        iVar8 = *(int *)(param_1 + 0x14) + uVar7;
        *(uint *)(param_1 + 0x18) = (int)sVar2 - uVar7;
        *(int *)(param_1 + 0x14) = iVar8;
        if ((0 < *(int *)(param_1 + 0x24)) || ((*(byte *)(param_1 + 0x30) & 8) != 0)) {
          *(char *)(iVar8 + *(int *)(param_1 + 8)) = local_d[0];
          *(int *)(param_1 + 0x1c) = *(int *)(param_1 + 0x1c) + 1;
        }
        *(undefined4 *)(param_1 + 0x20) = *(undefined4 *)(param_1 + 0x24);
      }
      else {
        puVar9 = param_3;
        for (uVar4 = (uint)(int)sVar2 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
          *puVar3 = *puVar9;
          puVar9 = puVar9 + 1;
          puVar3 = puVar3 + 1;
        }
        for (uVar4 = (int)sVar2 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
          *(undefined1 *)puVar3 = *(undefined1 *)puVar9;
          puVar9 = (undefined4 *)((int)puVar9 + 1);
          puVar3 = (undefined4 *)((int)puVar3 + 1);
        }
        sVar6 = param_4 - sVar2;
        *(int *)(param_1 + 0x14) = *(int *)(param_1 + 0x14) + (int)sVar2;
        if ((0 < *(int *)(param_1 + 0x24)) || ((*(byte *)(param_1 + 0x30) & 8) != 0)) {
          *(char *)(*(int *)(param_1 + 8) + *(int *)(param_1 + 0x14)) = local_d[0];
          *(int *)(param_1 + 0x14) = *(int *)(param_1 + 0x14) + 1;
        }
        if (*(int *)(param_1 + 0x24) < (int)sVar6) {
          sVar6 = (short)*(int *)(param_1 + 0x24);
        }
        uVar4 = (uint)sVar6;
        puVar3 = (undefined4 *)((int)sVar2 + (int)param_3);
        puVar9 = (undefined4 *)(*(int *)(param_1 + 8) + *(int *)(param_1 + 0x14));
        for (uVar7 = uVar4 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
          *puVar9 = *puVar3;
          puVar3 = puVar3 + 1;
          puVar9 = puVar9 + 1;
        }
        for (uVar7 = uVar4 & 3; uVar7 != 0; uVar7 = uVar7 - 1) {
          *(undefined1 *)puVar9 = *(undefined1 *)puVar3;
          puVar3 = (undefined4 *)((int)puVar3 + 1);
          puVar9 = (undefined4 *)((int)puVar9 + 1);
        }
        *(uint *)(param_1 + 0x14) = *(int *)(param_1 + 0x14) + uVar4;
        *(uint *)(param_1 + 0x18) = *(int *)(param_1 + 0x24) - uVar4;
      }
    }
  }
  else {
    if ((param_2 == 'g') || (param_2 == 'G')) {
      if (((int)param_4 < *(int *)(param_1 + 0x24)) && ((*(byte *)(param_1 + 0x30) & 8) == 0)) {
        *(int *)(param_1 + 0x24) = (int)param_4;
      }
      piVar1 = (int *)(param_1 + 0x24);
      *piVar1 = *piVar1 + -1;
      if (*piVar1 < 0) {
        *(undefined4 *)(param_1 + 0x24) = 0;
      }
      param_2 = ((param_2 != 'g') - 1U & 0x20) + 0x45;
    }
    else if (param_2 == 'a') {
      param_2 = 'p';
    }
    else if (param_2 == 'A') {
      param_2 = 'P';
    }
    *(undefined1 *)(*(int *)(param_1 + 8) + *(int *)(param_1 + 0x14)) = *(undefined1 *)param_3;
    *(int *)(param_1 + 0x14) = *(int *)(param_1 + 0x14) + 1;
    if ((0 < *(int *)(param_1 + 0x24)) || ((*(byte *)(param_1 + 0x30) & 8) != 0)) {
      *(char *)(*(int *)(param_1 + 0x14) + *(int *)(param_1 + 8)) = local_d[0];
      *(int *)(param_1 + 0x14) = *(int *)(param_1 + 0x14) + 1;
    }
    iVar8 = *(int *)(param_1 + 0x24);
    if (0 < iVar8) {
      sVar6 = param_4 + -1;
      if (iVar8 < sVar6) {
        sVar6 = (short)iVar8;
      }
      uVar7 = (uint)sVar6;
      puVar3 = (undefined4 *)((int)param_3 + 1);
      puVar9 = (undefined4 *)(*(int *)(param_1 + 8) + *(int *)(param_1 + 0x14));
      for (uVar4 = uVar7 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
        *puVar9 = *puVar3;
        puVar3 = puVar3 + 1;
        puVar9 = puVar9 + 1;
      }
      for (uVar4 = uVar7 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
        *(undefined1 *)puVar9 = *(undefined1 *)puVar3;
        puVar3 = (undefined4 *)((int)puVar3 + 1);
        puVar9 = (undefined4 *)((int)puVar9 + 1);
      }
      *(uint *)(param_1 + 0x14) = *(int *)(param_1 + 0x14) + uVar7;
      *(uint *)(param_1 + 0x18) = *(int *)(param_1 + 0x24) - uVar7;
    }
    pcVar10 = (char *)(*(int *)(param_1 + 8) + *(int *)(param_1 + 0x14));
    *pcVar10 = param_2;
    if (sVar2 < 0) {
      pcVar10[1] = '-';
      param_5 = -param_5;
    }
    else {
      pcVar10[1] = '+';
    }
    pcVar11 = pcVar10 + 2;
    iVar8 = 0;
    if (0 < (short)param_5) {
      do {
        param_5 = CRT_DivCeil((int)(short)param_5,10);
        local_d[iVar8 + 1] = extraout_DL;
        iVar8 = iVar8 + 1;
      } while (0 < (short)param_5);
      if (iVar8 == 1) {
        if ((param_2 != 'e') && (param_2 != 'E')) goto LAB_00487221;
        *pcVar11 = '0';
        pcVar11 = pcVar10 + 3;
        goto LAB_00487221;
      }
      if (iVar8 != 0) {
        for (; 0 < iVar8; iVar8 = iVar8 + -1) {
LAB_00487221:
          *pcVar11 = local_d[iVar8] + '0';
          pcVar11 = pcVar11 + 1;
        }
        *(char **)(param_1 + 0x1c) = pcVar11 + (-*(int *)(param_1 + 0x14) - *(int *)(param_1 + 8));
        goto LAB_004873d3;
      }
    }
    *pcVar11 = '0';
    *(char **)(param_1 + 0x1c) = pcVar10 + ((3 - *(int *)(param_1 + 8)) - *(int *)(param_1 + 0x14));
  }
LAB_004873d3:
  if (((byte)*(undefined4 *)(param_1 + 0x30) & 0x14) == 0x10) {
    iVar8 = *(int *)(param_1 + 0x20) + *(int *)(param_1 + 0x1c) + *(int *)(param_1 + 0x18) +
            *(int *)(param_1 + 0xc) + *(int *)(param_1 + 0x14);
    if (iVar8 < *(int *)(param_1 + 0x28)) {
      *(int *)(param_1 + 0x10) = *(int *)(param_1 + 0x28) - iVar8;
    }
  }
  return;
}

