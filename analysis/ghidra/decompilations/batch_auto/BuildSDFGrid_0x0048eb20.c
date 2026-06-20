
/* WARNING: Removing unreachable block (ram,0x0048eb95) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int * __cdecl BuildSDFGrid(uint param_1,int param_2,int param_3)

{
  int *piVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  int *piVar10;
  int iVar11;
  int iVar12;
  int iVar13;
  int iVar14;
  uint uVar15;
  int iVar16;
  int iVar17;
  undefined4 unaff_EDI;
  uint uVar18;
  longlong lVar19;
  float local_24;
  uint local_1c;
  float local_18;
  uint local_10;
  float local_c;
  float local_8;
  
  uVar18 = 0;
  if ((param_1 != 0) && (param_2 != 0)) {
    fVar2 = (float)param_2;
    if (param_2 < 0) {
      fVar2 = fVar2 + _DAT_004cf558;
    }
    fVar3 = (float)(int)param_1;
    if ((int)param_1 < 0) {
      fVar3 = fVar3 + _DAT_004cf558;
    }
    fVar4 = fVar2 / fVar3;
    uVar15 = 0x10;
    fVar5 = _DAT_004cf3f0 / fVar4;
    if (param_1 != 0) {
      do {
        lVar19 = __ftol();
        uVar18 = uVar18 + 1;
        uVar15 = uVar15 + 0xc + (int)lVar19 * 0x10;
      } while (uVar18 < param_1);
    }
    piVar10 = operator_new(uVar15);
    if (piVar10 != (int *)0x0) {
      iVar14 = 0;
      local_1c = 0;
      iVar17 = 4;
      iVar16 = iVar17;
      if (param_1 != 0) {
        do {
          local_8 = 0.0;
          local_10 = 0;
          fVar6 = (float)(int)local_1c;
          iVar17 = iVar16 + 4;
          if ((int)local_1c < 0) {
            fVar6 = fVar6 + _DAT_004cf558;
          }
          do {
            fVar7 = (float)(int)local_10;
            if ((int)local_10 < 0) {
              fVar7 = fVar7 + _DAT_004cf558;
            }
            fVar7 = (fVar7 + fVar6) - _DAT_004cf3f0;
            local_24 = fVar7 * fVar4;
            local_c = fVar4 + local_24;
            if (param_3 == 0) {
              if (local_24 < _DAT_004cf368) {
                local_24 = 0.0;
              }
              if (fVar2 < local_c) {
                local_c = fVar2;
              }
            }
            FPU_RoundDouble((double)local_24,(short)unaff_EDI);
            lVar19 = __ftol();
            iVar13 = (int)lVar19;
            fVar8 = (float)iVar13;
            if (fVar8 < local_c) {
              iVar11 = iVar13 - param_2;
              do {
                local_18 = _DAT_004cf310 + fVar8;
                if (iVar13 < 0) {
                  iVar12 = iVar13 + param_2;
                }
                else {
                  iVar12 = iVar11;
                  if (iVar13 < param_2) {
                    iVar12 = iVar13;
                  }
                }
                if (iVar12 != iVar14) {
                  if (_DAT_004dc040 < local_8) {
                    piVar1 = (int *)(iVar17 + (int)piVar10);
                    *piVar1 = iVar14;
                    iVar17 = iVar17 + 8;
                    piVar1[1] = (int)local_8;
                  }
                  local_8 = 0.0;
                  iVar14 = iVar12;
                }
                if (fVar8 < local_24) {
                  fVar8 = local_24;
                }
                if (local_c < local_18) {
                  local_18 = local_c;
                }
                if (param_3 == 0) {
                  if (_DAT_004cf368 <= fVar7) {
                    if (fVar7 + _DAT_004cf310 < fVar3) goto LAB_0048ed47;
                    fVar9 = 0.0;
                  }
                  else {
                    fVar9 = 1.0;
                  }
                }
                else {
LAB_0048ed47:
                  fVar9 = (local_18 + fVar8) * fVar5 - fVar7;
                }
                if (local_10 != 0) {
                  fVar9 = 1.0 - fVar9;
                }
                iVar13 = iVar13 + 1;
                iVar11 = iVar11 + 1;
                local_8 = (local_18 - fVar8) * fVar9 + local_8;
                fVar8 = (float)iVar13;
              } while (fVar8 < local_c);
            }
            local_10 = local_10 + 1;
          } while (local_10 < 2);
          if (_DAT_004dc040 < local_8) {
            piVar1 = (int *)(iVar17 + (int)piVar10);
            iVar17 = iVar17 + 8;
            *piVar1 = iVar14;
            piVar1[1] = (int)local_8;
          }
          local_1c = local_1c + 1;
          *(int *)(iVar16 + (int)piVar10) = iVar17 - iVar16;
          iVar16 = iVar17;
        } while (local_1c < param_1);
      }
      *piVar10 = iVar17;
      return piVar10;
    }
  }
  return (int *)0x0;
}

