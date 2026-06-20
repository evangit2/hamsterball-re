
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl IDCT_8x8_Float(int param_1,int param_2,short *param_3,int param_4,int param_5)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float *pfVar7;
  float *pfVar8;
  undefined1 *puVar9;
  float10 fVar10;
  float10 fVar11;
  float10 fVar12;
  float10 fVar13;
  float10 fVar14;
  float10 fVar15;
  float10 fVar16;
  float10 extraout_ST0;
  float10 extraout_ST1;
  longlong lVar17;
  float local_11c [16];
  float local_dc [8];
  float local_bc [8];
  float local_9c [8];
  float local_7c [8];
  float local_5c [8];
  float local_3c [8];
  float local_1c;
  float local_18;
  int local_14;
  int local_10;
  float local_c;
  float local_8;
  
  fVar10 = (float10)_DAT_004e952c;
  fVar11 = (float10)_DAT_004e9528;
  local_14 = *(int *)(param_1 + 0x11c) + 0x80;
  pfVar8 = *(float **)(param_2 + 0x50);
  pfVar7 = local_11c;
  local_1c = 1.12104e-44;
  do {
    if ((((((param_3[8] == 0 && param_3[0x10] == 0) && param_3[0x18] == 0) && param_3[0x20] == 0) &&
         param_3[0x28] == 0) && param_3[0x30] == 0) && param_3[0x38] == 0) {
      fVar1 = (float)(int)*param_3 * *pfVar8;
      *pfVar7 = fVar1;
      pfVar7[8] = fVar1;
      pfVar7[0x10] = fVar1;
      pfVar7[0x18] = fVar1;
      pfVar7[0x20] = fVar1;
      pfVar7[0x28] = fVar1;
      pfVar7[0x30] = fVar1;
      pfVar7[0x38] = fVar1;
    }
    else {
      fVar2 = (float)(int)param_3[0x30] * pfVar8[0x30];
      fVar1 = (float)((float10)(int)param_3[0x20] * (float10)pfVar8[0x20] +
                     (float10)(int)*param_3 * (float10)*pfVar8);
      fVar12 = (float10)(int)*param_3 * (float10)*pfVar8 -
               (float10)(int)param_3[0x20] * (float10)pfVar8[0x20];
      fVar13 = (float10)fVar2 + (float10)((float)(int)param_3[0x10] * pfVar8[0x10]);
      fVar14 = ((float10)((float)(int)param_3[0x10] * pfVar8[0x10]) - (float10)fVar2) * fVar10 -
               fVar13;
      local_18 = (float)(fVar13 + (float10)fVar1);
      fVar1 = (float)((float10)fVar1 - fVar13);
      fVar2 = (float)(fVar14 + fVar12);
      fVar12 = fVar12 - fVar14;
      fVar13 = (float10)(int)param_3[0x38] * (float10)pfVar8[0x38];
      fVar5 = (float)(int)param_3[0x28] * pfVar8[0x28] + (float)(int)param_3[0x18] * pfVar8[0x18];
      fVar6 = (float)(int)param_3[0x28] * pfVar8[0x28] - (float)(int)param_3[0x18] * pfVar8[0x18];
      fVar3 = (float)(fVar13 + (float10)(int)param_3[8] * (float10)pfVar8[8]);
      fVar13 = (float10)(int)param_3[8] * (float10)pfVar8[8] - fVar13;
      fVar14 = (float10)fVar3 + (float10)fVar5;
      fVar4 = (float)((fVar13 + (float10)fVar6) * fVar11);
      fVar15 = ((float10)fVar4 - (float10)fVar6 * (float10)_DAT_004e9524) - fVar14;
      fVar16 = ((float10)fVar3 - (float10)fVar5) * fVar10 - fVar15;
      local_8 = (float)fVar16;
      local_c = (float)((fVar13 * (float10)_DAT_004e9520 - (float10)fVar4) + fVar16);
      *pfVar7 = (float)(fVar14 + (float10)local_18);
      pfVar7[0x38] = (float)((float10)local_18 - fVar14);
      pfVar7[8] = (float)((float10)fVar2 + fVar15);
      pfVar7[0x30] = (float)((float10)fVar2 - fVar15);
      pfVar7[0x10] = (float)((float10)local_8 + fVar12);
      pfVar7[0x28] = (float)(fVar12 - (float10)local_8);
      pfVar7[0x20] = local_c + fVar1;
      pfVar7[0x18] = fVar1 - local_c;
    }
    pfVar8 = pfVar8 + 1;
    param_3 = param_3 + 1;
    pfVar7 = pfVar7 + 1;
    local_1c = (float)((int)local_1c + -1);
  } while (local_1c != 0.0);
  local_10 = 0;
  pfVar7 = local_11c + 2;
  do {
    puVar9 = (undefined1 *)(*(int *)(param_4 + local_10 * 4) + param_5);
    local_18 = *pfVar7 + pfVar7[4] + pfVar7[2] + pfVar7[-2];
    local_c = (float)((float10)pfVar7[3] + (float10)pfVar7[1]);
    local_8 = pfVar7[5] + pfVar7[-1];
    local_1c = pfVar7[-1] - pfVar7[5];
    fVar11 = ((float10)local_1c + (float10)(pfVar7[3] - pfVar7[1])) * fVar11;
    fVar10 = ((float10)local_8 - (float10)local_c) * fVar10 -
             ((fVar11 - (float10)(pfVar7[3] - pfVar7[1]) * (float10)_DAT_004e9524) -
             ((float10)pfVar7[3] + (float10)pfVar7[1] + (float10)local_8));
    local_8 = (float)fVar10;
    local_c = (float)(((float10)local_1c * (float10)_DAT_004e9520 - fVar11) + fVar10);
    lVar17 = __ftol();
    *puVar9 = *(undefined1 *)(((int)lVar17 + 4 >> 3 & 0x3ffU) + local_14);
    lVar17 = __ftol();
    puVar9[7] = *(undefined1 *)(((int)lVar17 + 4 >> 3 & 0x3ffU) + local_14);
    lVar17 = __ftol();
    puVar9[1] = *(undefined1 *)(((int)lVar17 + 4 >> 3 & 0x3ffU) + local_14);
    lVar17 = __ftol();
    puVar9[6] = *(undefined1 *)(((int)lVar17 + 4 >> 3 & 0x3ffU) + local_14);
    lVar17 = __ftol();
    puVar9[2] = *(undefined1 *)(((int)lVar17 + 4 >> 3 & 0x3ffU) + local_14);
    lVar17 = __ftol();
    puVar9[5] = *(undefined1 *)(((int)lVar17 + 4 >> 3 & 0x3ffU) + local_14);
    lVar17 = __ftol();
    puVar9[4] = *(undefined1 *)(((int)lVar17 + 4 >> 3 & 0x3ffU) + local_14);
    lVar17 = __ftol();
    pfVar7 = pfVar7 + 8;
    local_10 = local_10 + 1;
    puVar9[3] = *(undefined1 *)(((int)lVar17 + 4 >> 3 & 0x3ffU) + local_14);
    fVar11 = extraout_ST0;
    fVar10 = extraout_ST1;
  } while (local_10 < 8);
  return;
}

