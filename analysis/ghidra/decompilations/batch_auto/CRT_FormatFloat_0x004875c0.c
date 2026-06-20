
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void CRT_FormatFloat(double *param_1,char param_2)

{
  char *pcVar1;
  char cVar2;
  undefined2 *puVar3;
  double dVar4;
  ushort uVar5;
  short sVar6;
  undefined4 uVar7;
  undefined2 *puVar8;
  uint uVar9;
  int iVar10;
  int iVar11;
  uint uVar12;
  double *pdVar13;
  char extraout_DL;
  char cVar14;
  int iVar15;
  char *pcVar16;
  ulonglong uVar17;
  uint local_3c;
  int local_38;
  double local_30;
  char local_28;
  char local_27;
  char local_26 [6];
  char local_20 [28];
  
  local_30 = *param_1;
  pcVar16 = &local_28;
  if (*(int *)((int)param_1 + 0x24) < 0) {
    *(undefined4 *)((int)param_1 + 0x24) = 6;
  }
  else if ((*(int *)((int)param_1 + 0x24) == 0) && ((param_2 == 'g' || (param_2 == 'G')))) {
    *(undefined4 *)((int)param_1 + 0x24) = 1;
  }
  uVar7 = CRT_ClassifyFP((short *)&local_3c,(ushort *)param_1);
  sVar6 = (short)uVar7;
  if (sVar6 < 1) {
    if (sVar6 == 0) {
      sVar6 = 0;
      local_3c = 0;
    }
    else {
      if (local_30 < _DAT_004cf6a8) {
        local_30 = -local_30;
      }
      local_3c = ((short)local_3c * 0x7597) / 100000 - 4;
      sVar6 = (short)local_3c;
      if (sVar6 < 0) {
        uVar9 = 3U - (int)sVar6 & 0xfffffffc;
        local_3c = -uVar9;
        if (0 < (int)uVar9) {
          pdVar13 = (double *)&DAT_004db4e8;
          do {
            if ((uVar9 & 1) != 0) {
              local_30 = local_30 * *pdVar13;
            }
            uVar9 = (int)uVar9 >> 1;
            pdVar13 = pdVar13 + 1;
          } while (0 < (int)uVar9);
        }
      }
      else if (0 < sVar6) {
        local_3c = local_3c & 0xfffffffc;
        uVar5 = (ushort)local_3c;
        dVar4 = _DAT_004cf3c8;
        if (0 < (short)uVar5) {
          pdVar13 = (double *)&DAT_004db4e8;
          do {
            if ((uVar5 & 1) != 0) {
              dVar4 = dVar4 * *pdVar13;
            }
            uVar5 = (short)uVar5 >> 1;
            pdVar13 = pdVar13 + 1;
          } while (0 < (short)uVar5);
        }
        local_30 = local_30 / dVar4;
      }
      if (param_2 == 'f') {
        iVar10 = (short)local_3c + 10;
      }
      else {
        iVar10 = 6;
      }
      iVar10 = *(int *)((int)param_1 + 0x24) + iVar10;
      if (0x13 < iVar10) {
        iVar10 = 0x13;
      }
      local_28 = '0';
      pcVar16 = &local_27;
      while ((0 < iVar10 && (_DAT_004cf6a8 < local_30))) {
        uVar17 = __ftol2();
        iVar11 = (int)uVar17;
        iVar10 = iVar10 + -8;
        if (0 < iVar10) {
          local_30 = (local_30 - (double)iVar11) * _DAT_004db4e0;
        }
        pcVar16 = pcVar16 + 8;
        iVar15 = 8;
        local_38 = iVar11;
        while ((0 < iVar11 && (iVar15 = iVar15 + -1, -1 < iVar15))) {
          iVar11 = CRT_DivCeil(iVar11,10);
          pcVar16 = pcVar16 + -1;
          *pcVar16 = extraout_DL + '0';
        }
        if (-1 < iVar15 + -1) {
          do {
            pcVar16 = pcVar16 + -1;
            iVar15 = iVar15 + -1;
            *pcVar16 = '0';
          } while (iVar15 != 0);
        }
        pcVar16 = pcVar16 + 8;
      }
      uVar9 = (int)pcVar16 - (int)&local_27;
      local_3c = local_3c + 7;
      pcVar16 = &local_27;
      while (local_27 == '0') {
        pcVar1 = pcVar16 + 1;
        uVar9 = uVar9 - 1;
        local_3c = local_3c - 1;
        pcVar16 = pcVar16 + 1;
        local_27 = *pcVar1;
      }
      if (param_2 == 'f') {
        sVar6 = (short)local_3c + 1;
      }
      else if ((param_2 == 'e') || (param_2 == 'E')) {
        sVar6 = 1;
      }
      else {
        sVar6 = 0;
      }
      uVar5 = *(short *)((int)param_1 + 0x24) + sVar6;
      uVar12 = (uint)uVar5;
      if ((int)uVar9 < (int)(short)uVar5) {
        uVar12 = uVar9;
      }
      sVar6 = (short)uVar12;
      if (0 < sVar6) {
        iVar10 = (int)sVar6;
        if (((int)uVar9 <= iVar10) || (cVar14 = '9', pcVar16[iVar10] < '5')) {
          cVar14 = '0';
        }
        iVar10 = iVar10 + -1;
        cVar2 = pcVar16[iVar10];
        while (sVar6 = (short)uVar12, cVar2 == cVar14) {
          uVar12 = uVar12 - 1;
          iVar10 = iVar10 + -1;
          cVar2 = pcVar16[iVar10];
        }
        if (cVar14 == '\t') {
          pcVar16[iVar10] = pcVar16[iVar10] + '\x01';
        }
        if (iVar10 < 0) {
          pcVar16 = pcVar16 + -1;
          sVar6 = sVar6 + 1;
          local_3c = local_3c + 1;
        }
      }
    }
    CRT_FormatFloatOutput((int)param_1,param_2,(undefined4 *)pcVar16,sVar6,local_3c);
    return;
  }
  puVar8 = &DAT_004db534;
  if (sVar6 != 2) {
    puVar8 = &DAT_004db530;
  }
  puVar3 = *(undefined2 **)(param_1 + 1);
  *(undefined4 *)((int)param_1 + 0x14) = 3;
  *puVar3 = *puVar8;
  *(undefined1 *)(puVar3 + 1) = *(undefined1 *)(puVar8 + 1);
  return;
}

