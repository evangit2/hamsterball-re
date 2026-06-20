
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl Color_FitPrincipalAxis(float *param_1,float *param_2,int param_3,float param_4)

{
  float *pfVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  int iVar10;
  float *pfVar11;
  int iVar12;
  uint uVar13;
  uint uVar14;
  float *pfVar15;
  float fVar16;
  float local_c4 [16];
  float *local_84;
  int local_80;
  float local_7c;
  float local_78;
  float local_74;
  float *local_6c;
  undefined4 *local_68;
  float local_64;
  float local_60;
  float local_5c;
  float local_58;
  float local_54;
  float local_50;
  float local_48;
  float local_44;
  float local_40;
  float local_3c;
  float local_34 [6];
  float local_1c;
  float local_14;
  float local_10;
  float local_c;
  undefined4 uStack_8;
  
  fVar9 = param_4;
  if (param_4 == 4.2039e-45) {
    local_6c = (float *)&DAT_004e5a48;
    local_68 = &DAT_004e5a3c;
  }
  else {
    local_6c = (float *)&DAT_004e5a2c;
    local_68 = (undefined4 *)&DAT_004e5a1c;
  }
  local_34[4] = 0.0;
  local_34[5] = 0.0;
  local_14 = DAT_004fab40;
  local_10 = DAT_004fab44;
  local_1c = 0.0;
  local_c = DAT_004fab48;
  uStack_8 = DAT_004fab4c;
  pfVar15 = (float *)(param_3 + 8);
  iVar10 = 0x10;
  local_84 = pfVar15;
  pfVar11 = pfVar15;
  iVar12 = iVar10;
  do {
    if (pfVar11[-2] < local_14) {
      local_14 = pfVar11[-2];
    }
    if (pfVar11[-1] < local_10) {
      local_10 = pfVar11[-1];
    }
    if (*pfVar11 < local_c) {
      local_c = *pfVar11;
    }
    if (local_34[4] < pfVar11[-2]) {
      local_34[4] = pfVar11[-2];
    }
    if (local_34[5] < pfVar11[-1]) {
      local_34[5] = pfVar11[-1];
    }
    if (local_1c < *pfVar11) {
      local_1c = *pfVar11;
    }
    fVar16 = local_1c;
    pfVar11 = pfVar11 + 4;
    iVar12 = iVar12 + -1;
  } while (iVar12 != 0);
  fVar4 = local_34[4] - local_14;
  fVar6 = local_34[5] - local_10;
  fVar7 = local_1c - local_c;
  local_48 = fVar4 * fVar4 + fVar6 * fVar6 + fVar7 * fVar7;
  if (_DAT_004d8f6c <= local_48) {
    fVar8 = 1.0 / local_48;
    local_7c = fVar4 * fVar8;
    local_78 = fVar6 * fVar8;
    local_74 = fVar8 * fVar7;
    local_58 = (local_34[4] + local_14) * _DAT_004cf3f0;
    local_54 = (local_34[5] + local_10) * _DAT_004cf3f0;
    local_50 = (local_1c + local_c) * _DAT_004cf3f0;
    local_34[3] = 0.0;
    local_34[2] = 0.0;
    local_34[1] = 0.0;
    fVar3 = 0.0;
    do {
      local_44 = (pfVar15[-2] - local_58) * fVar4 * fVar8;
      local_40 = (pfVar15[-1] - local_54) * fVar6 * fVar8;
      fVar2 = *pfVar15;
      pfVar15 = pfVar15 + 4;
      iVar10 = iVar10 + -1;
      fVar2 = (fVar2 - local_50) * fVar8 * fVar7;
      fVar5 = local_40 + fVar2 + local_44;
      fVar3 = fVar5 * fVar5 + fVar3;
      fVar5 = (local_40 + local_44) - fVar2;
      local_34[1] = fVar5 * fVar5 + local_34[1];
      fVar5 = (local_44 - local_40) + fVar2;
      local_34[2] = fVar5 * fVar5 + local_34[2];
      fVar2 = (local_44 - local_40) - fVar2;
      local_34[3] = fVar2 * fVar2 + local_34[3];
    } while (iVar10 != 0);
    local_34[0] = fVar3;
    uVar14 = 0;
    uVar13 = 1;
    do {
      pfVar11 = local_34 + uVar13;
      if (fVar3 < *pfVar11) {
        fVar3 = *pfVar11;
        uVar14 = uVar13;
      }
      uVar13 = uVar13 + 1;
    } while (uVar13 < 4);
    fVar4 = local_10;
    if ((uVar14 & 2) != 0) {
      fVar4 = local_34[5];
      local_34[5] = local_10;
    }
    local_10 = fVar4;
    if ((uVar14 & 1) != 0) {
      local_1c = local_c;
      local_c = fVar16;
    }
    if (_DAT_004e5a18 <= local_48) {
      local_60 = (float)((int)param_4 + -1);
      if ((int)param_4 + -1 < 0) {
        local_60 = local_60 + _DAT_004cf558;
      }
      SSE2_SetFPControlWord(pfVar11);
      local_48 = 0.0;
      while( true ) {
        iVar10 = 0x10;
        if (fVar9 != 0.0) {
          pfVar11 = local_c4 + 1;
          iVar12 = (int)local_68 - (int)local_6c;
          pfVar15 = local_6c;
          fVar16 = fVar9;
          do {
            pfVar11[-1] = local_34[4] * *(float *)(iVar12 + (int)pfVar15) + local_14 * *pfVar15;
            *pfVar11 = local_34[5] * *(float *)(iVar12 + (int)pfVar15) + local_10 * *pfVar15;
            fVar4 = *pfVar15;
            pfVar1 = (float *)(iVar12 + (int)pfVar15);
            pfVar15 = pfVar15 + 1;
            pfVar11[1] = local_1c * *pfVar1 + local_c * fVar4;
            pfVar11 = pfVar11 + 4;
            fVar16 = (float)((int)fVar16 + -1);
          } while (fVar16 != 0.0);
        }
        fVar4 = local_34[4] - local_14;
        fVar16 = local_34[5] - local_10;
        fVar7 = local_1c - local_c;
        fVar6 = fVar4 * fVar4 + fVar16 * fVar16 + fVar7 * fVar7;
        if (fVar6 < _DAT_004e5a18) break;
        fVar6 = local_60 / fVar6;
        local_44 = fVar6 * fVar4;
        local_40 = fVar16 * fVar6;
        local_3c = fVar6 * fVar7;
        local_34[2] = 0.0;
        local_34[1] = 0.0;
        local_34[0] = 0.0;
        local_50 = 0.0;
        local_54 = 0.0;
        local_58 = 0.0;
        local_5c = 0.0;
        param_4 = 0.0;
        pfVar11 = local_84;
        do {
          fVar3 = (*pfVar11 - local_c) * fVar6 * fVar7 +
                  (pfVar11[-2] - local_14) * fVar6 * fVar4 +
                  (pfVar11[-1] - local_10) * fVar16 * fVar6;
          local_64 = fVar3;
          if (fVar3 < local_60) {
            local_64 = fVar3 + _DAT_004cf3f0;
            local_80 = (int)ROUND(fVar3 + _DAT_004cf3f0);
          }
          else {
            local_80 = (int)fVar9 + -1;
          }
          pfVar15 = pfVar11 + -2;
          local_78 = local_c4[local_80 * 4 + 1] - pfVar11[-1];
          fVar3 = *pfVar11;
          fVar2 = local_6c[local_80] * _DAT_004cf4d8;
          pfVar11 = pfVar11 + 4;
          iVar10 = iVar10 + -1;
          fVar8 = (float)local_68[local_80] * _DAT_004cf4d8;
          param_4 = fVar2 * local_6c[local_80] + param_4;
          local_58 = fVar2 * (local_c4[local_80 * 4] - *pfVar15) + local_58;
          local_54 = fVar2 * local_78 + local_54;
          local_50 = fVar2 * (local_c4[local_80 * 4 + 2] - fVar3) + local_50;
          local_5c = fVar8 * (float)local_68[local_80] + local_5c;
          local_34[0] = fVar8 * (local_c4[local_80 * 4] - *pfVar15) + local_34[0];
          local_34[1] = local_78 * fVar8 + local_34[1];
          local_34[2] = fVar8 * (local_c4[local_80 * 4 + 2] - fVar3) + local_34[2];
        } while (iVar10 != 0);
        if (_DAT_004cf368 < param_4) {
          fVar16 = _DAT_004d0250 / param_4;
          local_14 = local_58 * fVar16 + local_14;
          local_10 = local_54 * fVar16 + local_10;
          local_c = fVar16 * local_50 + local_c;
        }
        if (_DAT_004cf368 < local_5c) {
          local_5c = _DAT_004d0250 / local_5c;
          local_34[4] = local_34[0] * local_5c + local_34[4];
          local_34[5] = local_34[1] * local_5c + local_34[5];
          local_1c = local_5c * local_34[2] + local_1c;
        }
        if (((((local_58 * local_58 < _DAT_004e5a14) && (local_54 * local_54 < _DAT_004e5a14)) &&
             (local_50 * local_50 < _DAT_004e5a14)) &&
            (((local_34[0] * local_34[0] < _DAT_004e5a14 &&
              (local_34[1] * local_34[1] < _DAT_004e5a14)) &&
             (local_34[2] * local_34[2] < _DAT_004e5a14)))) ||
           (local_48 = (float)((int)local_48 + 1), 7 < (uint)local_48)) break;
      }
      *param_1 = local_14;
      param_1[1] = local_10;
      param_1[2] = local_c;
      *param_2 = local_34[4];
      param_2[1] = local_34[5];
    }
    else {
      *param_1 = local_14;
      param_1[1] = local_10;
      param_1[2] = local_c;
      param_2[1] = local_34[5];
      *param_2 = local_34[4];
    }
  }
  else {
    *param_1 = local_14;
    param_1[1] = local_10;
    param_1[2] = local_c;
    param_2[1] = local_34[5];
    *param_2 = local_34[4];
  }
  param_2[2] = local_1c;
  return;
}

