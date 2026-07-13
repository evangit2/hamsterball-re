
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FlagWaver_UpdateVertices(int param_1)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float *pfVar7;
  float *pfVar8;
  float *pfVar9;
  uint uVar10;
  int iVar11;
  undefined4 *puVar12;
  undefined4 *unaff_EDI;
  int iVar13;
  undefined1 auVar14 [10];
  float10 fVar15;
  float local_5c;
  float local_58;
  float local_54;
  float local_50;
  float local_4c;
  float local_48;
  float local_44;
  float local_40;
  float local_3c;
  float local_38;
  float local_34;
  float local_2c;
  float local_28;
  float local_20;
  float local_1c;
  float local_10;
  float local_4;
  
  if ((*(int *)(param_1 + 0xc) != 0) &&
     (iVar11 = *(int *)(*(int *)(param_1 + 4) + 0x7c4), *(int *)(param_1 + 0x2c) != iVar11)) {
    *(int *)(param_1 + 0x2c) = iVar11;
    local_5c = 0.0;
    if (-1 < *(int *)(param_1 + 8)) {
      iVar11 = 0;
      iVar13 = 0;
      do {
        auVar14 = Wave_Sin(&PTR_PTR_004f7188,
                           ((local_5c / *(float *)(param_1 + 0x10)) * _DAT_004cf44c -
                           *(float *)(param_1 + 0x28)) * *(float *)(param_1 + 0x20));
        local_58 = (float)(float10)auVar14;
        auVar14 = Wave_Cos(&PTR_PTR_004f7188,
                           ((local_5c / *(float *)(param_1 + 0x10)) * _DAT_004cf44c -
                           *(float *)(param_1 + 0x28)) * *(float *)(param_1 + 0x20));
        fVar15 = ((float10)local_5c / (float10)*(float *)(param_1 + 0x10)) *
                 (float10)*(float *)(param_1 + 0x1c);
        *(float *)(iVar11 + *(int *)(param_1 + 0xc)) = (float)((float10)local_58 * fVar15);
        *(undefined4 *)(iVar11 + 4 + *(int *)(param_1 + 0xc)) = 0;
        *(float *)(iVar11 + 8 + *(int *)(param_1 + 0xc)) = local_5c;
        *(undefined4 *)(iVar11 + 0xc + *(int *)(param_1 + 0xc)) = 0;
        *(undefined4 *)(iVar11 + 0x10 + *(int *)(param_1 + 0xc)) = 0;
        *(undefined4 *)(iVar11 + 0x14 + *(int *)(param_1 + 0xc)) = 0;
        *(float *)(iVar11 + 0x18 + *(int *)(param_1 + 0xc)) = local_5c / *(float *)(param_1 + 0x10);
        *(undefined4 *)(iVar11 + 0x1c + *(int *)(param_1 + 0xc)) = 0;
        *(float *)(iVar11 + 0x20 + *(int *)(param_1 + 0xc)) = (float)(fVar15 * (float10)auVar14);
        *(float *)(iVar11 + 0x24 + *(int *)(param_1 + 0xc)) = -*(float *)(param_1 + 0x14);
        *(float *)(iVar11 + 0x28 + *(int *)(param_1 + 0xc)) = local_5c;
        *(undefined4 *)(iVar11 + 0x2c + *(int *)(param_1 + 0xc)) = 0;
        *(undefined4 *)(iVar11 + 0x30 + *(int *)(param_1 + 0xc)) = 0;
        *(undefined4 *)(iVar11 + 0x34 + *(int *)(param_1 + 0xc)) = 0;
        *(float *)(iVar11 + 0x38 + *(int *)(param_1 + 0xc)) = local_5c / *(float *)(param_1 + 0x10);
        *(undefined4 *)(iVar11 + 0x3c + *(int *)(param_1 + 0xc)) = 0x3f800000;
        if (0 < iVar11) {
          iVar2 = *(int *)(param_1 + 0xc);
          local_44 = *(float *)(iVar11 + -0x3c + iVar2);
          local_48 = *(float *)(iVar11 + -0x40 + iVar2);
          pfVar7 = (float *)(iVar11 + iVar2);
          local_40 = pfVar7[-0xe];
          local_2c = pfVar7[1];
          local_28 = pfVar7[2];
          local_54 = *pfVar7 - local_48;
          local_50 = local_2c - local_44;
          local_4c = local_28 - local_40;
          local_10 = local_4c;
          Vec3_NormalizeAndScale(&local_54,*(float *)(param_1 + 0x18));
          *pfVar7 = local_48 + local_54;
          *(float *)(iVar11 + 8 + *(int *)(param_1 + 0xc)) = local_40 + local_4c;
          iVar2 = *(int *)(param_1 + 0xc);
          local_38 = *(float *)(iVar11 + -0x1c + iVar2);
          local_3c = *(float *)(iVar11 + -0x20 + iVar2);
          iVar2 = iVar11 + iVar2;
          local_34 = *(float *)(iVar2 + -0x18);
          local_20 = *(float *)(iVar2 + 0x24);
          local_1c = *(float *)(iVar2 + 0x28);
          local_54 = *(float *)(iVar2 + 0x20) - local_3c;
          local_50 = local_20 - local_38;
          local_4c = local_1c - local_34;
          local_4 = local_4c;
          Vec3_NormalizeAndScale(&local_54,*(float *)(param_1 + 0x18));
          *(float *)(iVar2 + 0x20) = local_3c + local_54;
          *(float *)(iVar11 + 0x28 + *(int *)(param_1 + 0xc)) = local_34 + local_4c;
        }
        if (*(char *)(param_1 + 0x80) != '\0') {
          fVar4 = *(float *)(param_1 + 0x14) * _DAT_004cf3f0;
          fVar5 = ((float)_DAT_004cf3c8 - local_5c / *(float *)(param_1 + 0x10)) * fVar4;
          *(float *)(iVar11 + 4 + *(int *)(param_1 + 0xc)) =
               (fVar5 + fVar4) - *(float *)(param_1 + 0x14);
          *(float *)(iVar11 + 0x24 + *(int *)(param_1 + 0xc)) =
               (fVar4 - fVar5) - *(float *)(param_1 + 0x14);
        }
        local_5c = local_5c + *(float *)(param_1 + 0x18);
        iVar13 = iVar13 + 1;
        iVar11 = iVar11 + 0x40;
      } while (iVar13 <= *(int *)(param_1 + 8));
    }
    pfVar7 = operator_new(*(int *)(param_1 + 8) * 0xc);
    iVar11 = 0;
    if (0 < *(int *)(param_1 + 8)) {
      iVar13 = 0;
      pfVar9 = pfVar7;
      do {
        pfVar8 = (float *)(*(int *)(param_1 + 0xc) + iVar13);
        local_4 = pfVar8[2];
        fVar4 = *pfVar8 - *(float *)(*(int *)(param_1 + 0xc) + 0x40 + iVar13);
        local_54 = local_4 - pfVar8[0x12];
        local_50 = (fVar4 - local_54) * _DAT_004cf3f0 + _DAT_004cf4dc;
        fVar6 = fVar4 * fVar4 + local_54 * local_54 + local_50 * local_50;
        fVar5 = _DAT_004cf368;
        if ((fVar6 < _DAT_004cf368 == (fVar6 == _DAT_004cf368)) &&
           (fVar5 = SQRT(fVar6), _DAT_004cf368 < fVar5)) {
          fVar5 = _DAT_004cf310 / fVar5;
        }
        local_54 = fVar5 * local_54;
        local_50 = local_50 * fVar5;
        if (pfVar9 != &local_54) {
          pfVar9[2] = fVar5 * fVar4;
          *pfVar9 = local_54;
          pfVar9[1] = local_50;
        }
        iVar11 = iVar11 + 1;
        iVar13 = iVar13 + 0x40;
        *pfVar9 = -*pfVar9;
        pfVar9 = pfVar9 + 3;
      } while (iVar11 < *(int *)(param_1 + 8));
    }
    iVar11 = 1;
    *(float *)(*(int *)(param_1 + 0xc) + 0xc) = *pfVar7 + *(float *)(*(int *)(param_1 + 0xc) + 0xc);
    pfVar9 = pfVar7 + 1;
    *(float *)(*(int *)(param_1 + 0xc) + 0x10) =
         *(float *)(*(int *)(param_1 + 0xc) + 0x10) + *pfVar9;
    *(float *)(*(int *)(param_1 + 0xc) + 0x14) =
         *(float *)(*(int *)(param_1 + 0xc) + 0x14) + pfVar7[2];
    *(float *)(*(int *)(param_1 + 0xc) + 0x2c) =
         *pfVar7 + *(float *)(*(int *)(param_1 + 0xc) + 0x2c);
    *(float *)(*(int *)(param_1 + 0xc) + 0x30) =
         *(float *)(*(int *)(param_1 + 0xc) + 0x30) + *pfVar9;
    *(float *)(*(int *)(param_1 + 0xc) + 0x34) =
         *(float *)(*(int *)(param_1 + 0xc) + 0x34) + pfVar7[2];
    if (1 < *(int *)(param_1 + 8)) {
      iVar13 = 0x40;
      do {
        iVar11 = iVar11 + 1;
        *(float *)(iVar13 + 0xc + *(int *)(param_1 + 0xc)) =
             pfVar9[-1] + *(float *)(iVar13 + 0xc + *(int *)(param_1 + 0xc));
        *(float *)(iVar13 + 0x10 + *(int *)(param_1 + 0xc)) =
             *(float *)(iVar13 + 0x10 + *(int *)(param_1 + 0xc)) + *pfVar9;
        pfVar8 = (float *)(iVar13 + 0x14 + *(int *)(param_1 + 0xc));
        *pfVar8 = pfVar9[1] + *pfVar8;
        *(float *)(iVar13 + 0x2c + *(int *)(param_1 + 0xc)) =
             *(float *)(iVar13 + 0x2c + *(int *)(param_1 + 0xc)) + pfVar9[-1];
        *(float *)(iVar13 + 0x30 + *(int *)(param_1 + 0xc)) =
             *(float *)(iVar13 + 0x30 + *(int *)(param_1 + 0xc)) + *pfVar9;
        iVar2 = iVar13 + 0x34;
        iVar1 = iVar13 + 0x34;
        iVar13 = iVar13 + 0x40;
        *(float *)(iVar1 + *(int *)(param_1 + 0xc)) =
             *(float *)(iVar2 + *(int *)(param_1 + 0xc)) + pfVar9[1];
        pfVar9 = pfVar9 + 3;
      } while (iVar11 < *(int *)(param_1 + 8));
    }
    pfVar9 = (float *)(*(int *)(param_1 + 8) * 0x40 + 0xc + *(int *)(param_1 + 0xc));
    *pfVar9 = pfVar7[*(int *)(param_1 + 8) * 3 + -3] + *pfVar9;
    pfVar9 = (float *)(*(int *)(param_1 + 8) * 0x40 + 0x10 + *(int *)(param_1 + 0xc));
    *pfVar9 = pfVar7[*(int *)(param_1 + 8) * 3 + -2] + *pfVar9;
    pfVar9 = (float *)(*(int *)(param_1 + 8) * 0x40 + 0x14 + *(int *)(param_1 + 0xc));
    *pfVar9 = pfVar7[*(int *)(param_1 + 8) * 3 + -1] + *pfVar9;
    pfVar9 = (float *)(*(int *)(param_1 + 8) * 0x40 + 0x2c + *(int *)(param_1 + 0xc));
    *pfVar9 = pfVar7[*(int *)(param_1 + 8) * 3 + -3] + *pfVar9;
    pfVar9 = (float *)(*(int *)(param_1 + 8) * 0x40 + 0x30 + *(int *)(param_1 + 0xc));
    *pfVar9 = pfVar7[*(int *)(param_1 + 8) * 3 + -2] + *pfVar9;
    pfVar9 = (float *)(*(int *)(param_1 + 8) * 0x40 + 0x34 + *(int *)(param_1 + 0xc));
    iVar11 = 0;
    *pfVar9 = pfVar7[*(int *)(param_1 + 8) * 3 + -1] + *pfVar9;
    if (-1 < *(int *)(param_1 + 8)) {
      iVar13 = 0;
      do {
        iVar11 = iVar11 + 1;
        *(float *)(iVar13 + 0xc + *(int *)(param_1 + 0xc)) =
             *(float *)(iVar13 + 0xc + *(int *)(param_1 + 0xc)) * _DAT_004cf3f0;
        *(float *)(iVar13 + 0x10 + *(int *)(param_1 + 0xc)) =
             *(float *)(iVar13 + 0x10 + *(int *)(param_1 + 0xc)) * _DAT_004cf3f0;
        *(float *)(iVar13 + 0x14 + *(int *)(param_1 + 0xc)) =
             *(float *)(iVar13 + 0x14 + *(int *)(param_1 + 0xc)) * _DAT_004cf3f0;
        *(float *)(iVar13 + 0x2c + *(int *)(param_1 + 0xc)) =
             *(float *)(iVar13 + 0x2c + *(int *)(param_1 + 0xc)) * _DAT_004cf3f0;
        *(float *)(iVar13 + 0x30 + *(int *)(param_1 + 0xc)) =
             *(float *)(iVar13 + 0x30 + *(int *)(param_1 + 0xc)) * _DAT_004cf3f0;
        iVar2 = iVar13 + 0x34;
        iVar1 = iVar13 + 0x34;
        iVar13 = iVar13 + 0x40;
        *(float *)(iVar1 + *(int *)(param_1 + 0xc)) =
             *(float *)(iVar2 + *(int *)(param_1 + 0xc)) * _DAT_004cf3f0;
      } while (iVar11 <= *(int *)(param_1 + 8));
    }
    piVar3 = *(int **)(param_1 + 0x84);
    if (piVar3 != (int *)0x0) {
      (**(code **)(*piVar3 + 0x2c))(piVar3,0,0,&local_58,0);
      puVar12 = *(undefined4 **)(param_1 + 0xc);
      for (uVar10 = (uint)(*(int *)(param_1 + 0x88) << 5) >> 2; uVar10 != 0; uVar10 = uVar10 - 1) {
        *unaff_EDI = *puVar12;
        puVar12 = puVar12 + 1;
        unaff_EDI = unaff_EDI + 1;
      }
      for (iVar11 = 0; iVar11 != 0; iVar11 = iVar11 + -1) {
        *(undefined1 *)unaff_EDI = *(undefined1 *)puVar12;
        puVar12 = (undefined4 *)((int)puVar12 + 1);
        unaff_EDI = (undefined4 *)((int)unaff_EDI + 1);
      }
      (**(code **)(**(int **)(param_1 + 0x84) + 0x30))(*(int **)(param_1 + 0x84));
    }
    _free(pfVar7);
  }
  return;
}

