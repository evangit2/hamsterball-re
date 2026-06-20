
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int __cdecl DXT1_CompressColorBlock(char *param_1,int param_2)

{
  char *pcVar1;
  float fVar2;
  float fVar3;
  char cVar4;
  uint uVar5;
  char cVar6;
  int iVar7;
  undefined4 extraout_ECX;
  uint uVar8;
  float *pfVar9;
  float fVar10;
  uint uVar11;
  float *pfVar12;
  float local_a4 [32];
  int local_24;
  float local_20;
  float local_1c;
  float local_18;
  undefined *local_14;
  float *local_10;
  float *local_c;
  float local_8;
  
  pfVar9 = (float *)(param_2 + 0xc);
  local_10 = (float *)*pfVar9;
  pfVar12 = local_a4;
  local_c = local_10;
  for (iVar7 = 0x10; iVar7 != 0; iVar7 = iVar7 + -1) {
    *pfVar12 = 0.0;
    pfVar12 = pfVar12 + 1;
  }
  SSE2_SetFPControlWord(0);
  pcVar1 = param_1;
  uVar8 = 0;
  pfVar12 = pfVar9;
  do {
    local_18 = local_a4[uVar8] + *pfVar12;
    local_14 = (undefined *)(int)ROUND(local_18 * _DAT_004cf304 + _DAT_004cf3f0);
    local_8 = (float)local_14;
    fVar10 = (float)(int)local_14 * _DAT_004db358;
    local_a4[uVar8 + 0x10] = fVar10;
    if ((float)local_c <= fVar10) {
      if ((float)local_10 < fVar10) {
        local_10 = (float *)fVar10;
      }
    }
    else {
      local_c = (float *)fVar10;
    }
    fVar10 = local_18 - fVar10;
    uVar11 = uVar8 & 3;
    if (uVar11 != 3) {
      local_a4[uVar8 + 1] = _DAT_004e5a78 * fVar10 + local_a4[uVar8 + 1];
    }
    if (uVar8 < 0xc) {
      if (uVar11 != 0) {
        local_a4[uVar8 + 3] = _DAT_004e5a74 * fVar10 + local_a4[uVar8 + 3];
      }
      local_a4[uVar8 + 4] = _DAT_004e5a70 * fVar10 + local_a4[uVar8 + 4];
      if (uVar11 != 3) {
        local_a4[uVar8 + 5] = fVar10 * _DAT_004db564 + local_a4[uVar8 + 5];
      }
    }
    uVar8 = uVar8 + 1;
    pfVar12 = pfVar12 + 4;
  } while (uVar8 < 0x10);
  local_8 = DAT_00535280;
  iVar7 = DXT_CompressBlock1((ushort *)(param_1 + 8),0.0);
  if (iVar7 < 0) {
    return iVar7;
  }
  if ((float)local_c == 1.0) {
    *pcVar1 = -1;
    pcVar1[1] = -1;
LAB_004a831c:
    pcVar1[2] = '\0';
    pcVar1[3] = '\0';
    pcVar1[4] = '\0';
    pcVar1[5] = '\0';
    pcVar1[6] = '\0';
    pcVar1[7] = '\0';
  }
  else {
    if (((float)local_c == 0.0) || ((float)local_10 == 1.0)) {
      param_2 = 6;
      uVar8 = 6;
    }
    else {
      uVar8 = 8;
      param_2 = 8;
    }
    Color_FitEndpoints(&local_8,(float *)&param_1,(int)(local_a4 + 0x10),uVar8);
    SSE2_SetFPControlWord(extraout_ECX);
    uVar11 = (uint)ROUND(local_8 * _DAT_004cf304 + _DAT_004cf3f0);
    local_8 = (float)(int)ROUND((float)param_1 * _DAT_004cf304 + _DAT_004cf3f0);
    fVar10 = (float)(uVar11 & 0xff) * _DAT_004db358;
    local_a4[0x19] = (float)((uint)local_8 & 0xff) * _DAT_004db358;
    cVar4 = (char)uVar11;
    cVar6 = SUB41(local_8,0);
    if (uVar8 == 8) {
      if (cVar4 == cVar6) {
        *pcVar1 = cVar4;
        pcVar1[1] = cVar6;
        goto LAB_004a831c;
      }
LAB_004a8391:
      pcVar1[1] = cVar4;
      *pcVar1 = cVar6;
      uVar11 = 1;
      do {
        fVar2 = (float)(int)(7 - uVar11);
        if ((int)(7 - uVar11) < 0) {
          fVar2 = fVar2 + _DAT_004cf558;
        }
        fVar3 = (float)(int)uVar11;
        if ((int)uVar11 < 0) {
          fVar3 = fVar3 + _DAT_004cf558;
        }
        uVar5 = uVar11 + 1;
        local_a4[uVar11 + 0x19] = (fVar3 * fVar10 + fVar2 * local_a4[0x19]) * _DAT_004d0630;
        uVar11 = uVar5;
      } while (uVar5 < 7);
      local_14 = &DAT_004e5a7c;
      local_a4[0x18] = local_a4[0x19];
      local_a4[0x19] = fVar10;
    }
    else {
      if (uVar8 != 6) goto LAB_004a8391;
      *pcVar1 = cVar4;
      pcVar1[1] = cVar6;
      uVar11 = 1;
      do {
        fVar2 = (float)(int)(5 - uVar11);
        if ((int)(5 - uVar11) < 0) {
          fVar2 = fVar2 + _DAT_004cf558;
        }
        fVar3 = (float)(int)uVar11;
        if ((int)uVar11 < 0) {
          fVar3 = fVar3 + _DAT_004cf558;
        }
        uVar5 = uVar11 + 1;
        local_a4[uVar11 + 0x19] = (fVar3 * local_a4[0x19] + fVar2 * fVar10) * _DAT_004cf374;
        uVar11 = uVar5;
      } while (uVar5 < 5);
      local_14 = &DAT_004e5a9c;
      local_a4[0x1e] = 0.0;
      local_a4[0x1f] = 1.0;
      local_a4[0x18] = fVar10;
    }
    param_1 = (char *)(uVar8 - 1);
    local_1c = (float)(int)param_1;
    if ((int)param_1 < 0) {
      local_1c = local_1c + _DAT_004cf558;
    }
    if (local_a4[0x18] == local_a4[0x19]) {
      local_18 = 0.0;
    }
    else {
      local_18 = local_1c / (local_a4[0x19] - local_a4[0x18]);
    }
    pfVar12 = local_a4;
    for (iVar7 = 0x10; iVar7 != 0; iVar7 = iVar7 + -1) {
      *pfVar12 = 0.0;
      pfVar12 = pfVar12 + 1;
    }
    SSE2_SetFPControlWord(0);
    local_c = pfVar9;
    fVar10 = 0.0;
    do {
      uVar8 = 0;
      param_1._2_1_ = '\0';
      local_8 = (float)((int)fVar10 + 8U);
      if ((uint)fVar10 < (int)fVar10 + 8U) {
        local_10 = local_c;
        do {
          fVar2 = local_a4[(int)fVar10] + *local_10;
          fVar3 = (fVar2 - local_a4[0x18]) * local_18;
          if (fVar3 < _DAT_004cf368 == (fVar3 == _DAT_004cf368)) {
            if (fVar3 < local_1c) {
              local_20 = fVar3 + _DAT_004cf3f0;
              local_24 = (int)ROUND(fVar3 + _DAT_004cf3f0);
              iVar7 = *(int *)(local_14 + local_24 * 4);
            }
            else if ((param_2 == 6) &&
                    (fVar3 = (local_a4[0x19] + _DAT_004cf310) * _DAT_004cf3f0,
                    fVar3 < fVar2 != (fVar3 == fVar2))) {
              iVar7 = 7;
            }
            else {
              iVar7 = 1;
            }
          }
          else if ((param_2 != 6) || (local_a4[0x18] * _DAT_004cf3f0 < fVar2)) {
            iVar7 = 0;
          }
          else {
            iVar7 = 6;
          }
          fVar2 = fVar2 - local_a4[iVar7 + 0x18];
          uVar8 = uVar8 >> 3 | iVar7 << 0x15;
          uVar11 = (uint)fVar10 & 3;
          if (uVar11 != 3) {
            local_a4[(int)fVar10 + 1] = _DAT_004e5a78 * fVar2 + local_a4[(int)fVar10 + 1];
          }
          if ((uint)fVar10 < 0xc) {
            if (uVar11 != 0) {
              local_a4[(int)fVar10 + 3] = _DAT_004e5a74 * fVar2 + local_a4[(int)fVar10 + 3];
            }
            local_a4[(int)fVar10 + 4] = _DAT_004e5a70 * fVar2 + local_a4[(int)fVar10 + 4];
            if (uVar11 != 3) {
              local_a4[(int)fVar10 + 5] = fVar2 * _DAT_004db564 + local_a4[(int)fVar10 + 5];
            }
          }
          local_10 = local_10 + 4;
          fVar10 = (float)((int)fVar10 + 1);
        } while ((uint)fVar10 < (uint)local_8);
        param_1._2_1_ = (char)(uVar8 >> 0x10);
      }
      local_c = local_c + 0x20;
      pcVar1[4] = param_1._2_1_;
      pcVar1[2] = (char)uVar8;
      pcVar1[3] = (char)(uVar8 >> 8);
      fVar10 = local_8;
      pcVar1 = pcVar1 + 3;
    } while ((uint)local_8 < 0x10);
  }
  return 0;
}

