
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall Matrix_SolveGaussElim(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  float fVar3;
  float fVar4;
  int iVar5;
  int iVar6;
  float fVar7;
  int iVar8;
  int iVar9;
  float *pfVar10;
  int iVar11;
  int iVar12;
  float *pfVar13;
  float *pfVar14;
  int local_30;
  int local_2c;
  int local_28;
  int local_24;
  float *local_20;
  int local_18;
  int local_14;
  float *local_10;
  int local_c;
  float *local_8;
  
  iVar5 = *(int *)(param_1 + 0x20);
  iVar1 = iVar5 + -1;
  if (0 < iVar1) {
    local_10 = (float *)(iVar1 * 4);
    local_18 = iVar5 * 4;
    local_14 = (iVar1 + iVar5) * 4;
    local_28 = 0;
    iVar12 = 0;
    local_2c = iVar5;
    do {
      iVar9 = iVar12 + 1;
      local_8 = (float *)iVar12;
      if (iVar9 < iVar1) {
        local_c = local_28;
        local_24 = local_18;
        local_20 = (float *)(*(int *)(param_1 + 4) + (local_2c + iVar12) * 4);
        local_30 = iVar9;
        do {
          if (ABS(*(float *)(local_c + iVar12 * 4 + *(int *)(param_1 + 4))) < ABS(*local_20)) {
            local_8 = (float *)local_30;
            local_c = local_24;
          }
          local_30 = local_30 + 1;
          local_24 = local_24 + iVar5 * 4;
          local_20 = local_20 + iVar5;
        } while (local_30 < iVar1);
      }
      if (0 < iVar5) {
        iVar8 = (int)local_8 * iVar5 * 4;
        iVar11 = local_28;
        local_c = iVar5;
        do {
          iVar6 = *(int *)(param_1 + 4);
          uVar2 = *(undefined4 *)(iVar11 + iVar6);
          *(undefined4 *)(iVar11 + iVar6) = *(undefined4 *)(iVar8 + iVar6);
          *(undefined4 *)(iVar8 + iVar6) = uVar2;
          iVar8 = iVar8 + 4;
          iVar11 = iVar11 + 4;
          local_c = local_c + -1;
        } while (local_c != 0);
      }
      if (iVar9 < iVar1) {
        local_20 = (float *)local_14;
        iVar11 = (local_2c + iVar12) * 4;
        local_24 = iVar1 - iVar9;
        do {
          fVar7 = *(float *)(iVar11 + *(int *)(param_1 + 4)) /
                  *(float *)((iVar5 * 4 + 4) * iVar12 + *(int *)(param_1 + 4));
          if ((fVar7 != _DAT_004cf368) && (iVar12 <= iVar1)) {
            iVar8 = (iVar1 - iVar12) + 1;
            pfVar10 = local_20;
            pfVar13 = local_10;
            do {
              iVar6 = *(int *)(param_1 + 4);
              pfVar14 = (float *)((int)pfVar13 + iVar6);
              pfVar13 = (float *)((int)pfVar13 + -4);
              iVar8 = iVar8 + -1;
              *(float *)((int)pfVar10 + iVar6) = *(float *)((int)pfVar10 + iVar6) - fVar7 * *pfVar14
              ;
              pfVar10 = (float *)((int)pfVar10 + -4);
            } while (iVar8 != 0);
          }
          iVar11 = iVar11 + iVar5 * 4;
          local_20 = (float *)((int)local_20 + iVar5 * 4);
          local_24 = local_24 + -1;
        } while (local_24 != 0);
      }
      local_2c = local_2c + iVar5;
      iVar12 = iVar5 * 4;
      local_18 = local_18 + iVar12;
      local_14 = local_14 + iVar12;
      local_28 = local_28 + iVar12;
      local_10 = (float *)((int)local_10 + iVar12);
      iVar12 = iVar9;
    } while (iVar9 < iVar1);
  }
  iVar12 = iVar5 + -2;
  if (-1 < iVar12) {
    local_14 = (iVar5 * 4 + 4) * iVar12;
    iVar9 = iVar12 * iVar5;
    iVar8 = (iVar9 + iVar1) * 4;
    iVar11 = (iVar1 - iVar12) + -1;
    do {
      local_28 = iVar12 + 1;
      fVar7 = _DAT_004cf368;
      if (3 < iVar11) {
        local_8 = (float *)(iVar12 + 4);
        iVar6 = iVar12 + 1 + iVar9;
        pfVar10 = (float *)(*(int *)(param_1 + 0x14) + 0xc + iVar12 * 4);
        pfVar13 = (float *)(*(int *)(param_1 + 4) + 8 + iVar6 * 4);
        pfVar14 = (float *)(*(int *)(param_1 + 4) + 4 + iVar6 * 4);
        do {
          local_28 = local_28 + 4;
          local_8 = (float *)((int)local_8 + 4);
          fVar7 = pfVar13[1] * pfVar10[1] +
                  *pfVar10 * *pfVar13 + pfVar10[-1] * *pfVar14 + pfVar13[-2] * pfVar10[-2] + fVar7;
          pfVar10 = pfVar10 + 4;
          pfVar13 = pfVar13 + 4;
          pfVar14 = pfVar14 + 4;
        } while ((int)local_8 < iVar1);
      }
      if (local_28 < iVar1) {
        local_10 = (float *)(*(int *)(param_1 + 0x14) + local_28 * 4);
        local_8 = (float *)(*(int *)(param_1 + 4) + (iVar9 + local_28) * 4);
        local_28 = iVar1 - local_28;
        do {
          fVar3 = *local_10;
          fVar4 = *local_8;
          local_8 = local_8 + 1;
          local_10 = local_10 + 1;
          fVar7 = fVar3 * fVar4 + fVar7;
          local_28 = local_28 + -1;
        } while (local_28 != 0);
      }
      iVar12 = iVar12 + -1;
      iVar11 = iVar11 + 1;
      *(float *)(*(int *)(param_1 + 0x14) + 4 + iVar12 * 4) =
           (*(float *)(iVar8 + *(int *)(param_1 + 4)) - fVar7) /
           *(float *)(*(int *)(param_1 + 4) + local_14);
      local_14 = local_14 + iVar5 * -4 + -4;
      iVar9 = iVar9 - iVar5;
      iVar8 = iVar8 + iVar5 * -4;
    } while (-1 < iVar12);
  }
  return;
}

