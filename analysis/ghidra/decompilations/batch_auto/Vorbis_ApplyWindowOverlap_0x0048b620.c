
undefined4 __cdecl Vorbis_ApplyWindowOverlap(int param_1,int *param_2)

{
  int iVar1;
  int *piVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  undefined4 *puVar7;
  int *piVar8;
  int iVar9;
  uint uVar10;
  int *piVar11;
  int iVar12;
  int iVar13;
  int iVar14;
  uint local_28;
  uint local_20;
  int local_18;
  
  iVar9 = *(int *)(param_1 + 4);
  iVar1 = *(int *)(param_1 + 0x48);
  piVar2 = *(int **)(iVar9 + 0x1c);
  if ((*(int *)(param_1 + 0x18) < *(int *)(param_1 + 0x14)) && (*(int *)(param_1 + 0x18) != -1)) {
    return 0xffffff7d;
  }
  *(undefined4 *)(param_1 + 0x24) = *(undefined4 *)(param_1 + 0x28);
  uVar5 = *(uint *)(param_1 + 0x40);
  *(int *)(param_1 + 0x28) = param_2[8];
  *(undefined4 *)(param_1 + 0x2c) = 0xffffffff;
  if ((((uVar5 & *(uint *)(param_1 + 0x44)) == 0xffffffff) || (uVar5 + 1 != param_2[0x10])) ||
     (*(uint *)(param_1 + 0x44) + (uint)(0xfffffffe < uVar5) != param_2[0x11])) {
    *(undefined4 *)(param_1 + 0x38) = 0xffffffff;
    *(undefined4 *)(param_1 + 0x3c) = 0xffffffff;
    *(undefined4 *)(iVar1 + 0x10) = 0xffffffff;
    *(undefined4 *)(iVar1 + 0x14) = 0xffffffff;
  }
  *(int *)(param_1 + 0x40) = param_2[0x10];
  *(int *)(param_1 + 0x44) = param_2[0x11];
  if (*param_2 != 0) {
    iVar4 = piVar2[*(int *)(param_1 + 0x28)] / 2;
    uVar5 = piVar2[1] / 2;
    iVar13 = *piVar2 / 2;
    if (*(int *)(param_1 + 0x30) == 0) {
      local_20 = 0;
      uVar10 = uVar5;
    }
    else {
      uVar10 = 0;
      local_20 = uVar5;
    }
    local_28 = 0;
    if (0 < *(int *)(iVar9 + 4)) {
      do {
        uVar3 = local_28;
        if (*(int *)(param_1 + 0x24) == 0) {
          local_18 = iVar13;
          if (*(int *)(param_1 + 0x28) == 0) {
            piVar8 = (int *)(*(int *)(*(int *)(param_1 + 8) + local_28 * 4) + uVar10 * 4);
            if (0 < iVar13) {
              iVar14 = *(int *)(*param_2 + local_28 * 4) - (int)piVar8;
              do {
                *piVar8 = *piVar8 + *(int *)(iVar14 + (int)piVar8);
                piVar8 = piVar8 + 1;
                local_18 = local_18 + -1;
              } while (local_18 != 0);
            }
          }
          else {
            piVar8 = (int *)(*(int *)(*(int *)(param_1 + 8) + local_28 * 4) + uVar10 * 4);
            iVar14 = *(int *)(*param_2 + local_28 * 4) + ((int)uVar5 / 2 - iVar13 / 2) * 4;
            iVar12 = 0;
            if (0 < iVar13) {
              piVar11 = piVar8;
              do {
                *piVar11 = *piVar11 + *(int *)((int)piVar11 + (iVar14 - (int)piVar8));
                piVar11 = piVar11 + 1;
                local_18 = local_18 + -1;
                iVar12 = iVar13;
              } while (local_18 != 0);
            }
            iVar6 = iVar13 / 2 + (int)uVar5 / 2;
            if (iVar12 < iVar6) {
              piVar11 = piVar8 + iVar12;
              iVar6 = iVar6 - iVar12;
              do {
                *piVar11 = *(int *)((int)piVar11 + (iVar14 - (int)piVar8));
                piVar11 = piVar11 + 1;
                iVar6 = iVar6 + -1;
              } while (iVar6 != 0);
            }
          }
        }
        else if (*(int *)(param_1 + 0x28) == 0) {
          piVar8 = (int *)(*(int *)(*(int *)(param_1 + 8) + local_28 * 4) +
                          (((int)uVar5 / 2 - iVar13 / 2) + uVar10) * 4);
          if (0 < iVar13) {
            iVar14 = *(int *)(*param_2 + local_28 * 4) - (int)piVar8;
            local_28 = iVar13;
            do {
              *piVar8 = *piVar8 + *(int *)(iVar14 + (int)piVar8);
              piVar8 = piVar8 + 1;
              local_28 = local_28 + -1;
            } while (local_28 != 0);
          }
        }
        else {
          piVar8 = (int *)(*(int *)(*(int *)(param_1 + 8) + local_28 * 4) + uVar10 * 4);
          if (0 < (int)uVar5) {
            iVar14 = *(int *)(*param_2 + local_28 * 4) - (int)piVar8;
            local_28 = uVar5;
            do {
              *piVar8 = *piVar8 + *(int *)(iVar14 + (int)piVar8);
              piVar8 = piVar8 + 1;
              local_28 = local_28 - 1;
            } while (local_28 != 0);
          }
        }
        puVar7 = (undefined4 *)(*(int *)(*(int *)(param_1 + 8) + uVar3 * 4) + local_20 * 4);
        if (0 < iVar4) {
          iVar14 = (*(int *)(*param_2 + uVar3 * 4) + iVar4 * 4) - (int)puVar7;
          local_18 = iVar4;
          do {
            *puVar7 = *(undefined4 *)(iVar14 + (int)puVar7);
            puVar7 = puVar7 + 1;
            local_18 = local_18 + -1;
          } while (local_18 != 0);
        }
        local_28 = uVar3 + 1;
      } while ((int)local_28 < *(int *)(iVar9 + 4));
    }
    *(uint *)(param_1 + 0x30) = (*(int *)(param_1 + 0x30) != 0) - 1 & uVar5;
    if (*(int *)(param_1 + 0x18) == -1) {
      *(uint *)(param_1 + 0x18) = local_20;
    }
    else {
      *(uint *)(param_1 + 0x18) = uVar10;
      local_20 = ((int)(piVar2[*(int *)(param_1 + 0x24)] +
                       (piVar2[*(int *)(param_1 + 0x24)] >> 0x1f & 3U)) >> 2) +
                 uVar10 + ((int)(piVar2[*(int *)(param_1 + 0x28)] +
                                (piVar2[*(int *)(param_1 + 0x28)] >> 0x1f & 3U)) >> 2);
    }
    *(uint *)(param_1 + 0x14) = local_20;
  }
  if ((*(uint *)(iVar1 + 0x10) & *(uint *)(iVar1 + 0x14)) == 0xffffffff) {
    iVar9 = 0;
    *(undefined4 *)(iVar1 + 0x14) = 0;
  }
  else {
    uVar5 = ((int)(piVar2[*(int *)(param_1 + 0x24)] +
                  (piVar2[*(int *)(param_1 + 0x24)] >> 0x1f & 3U)) >> 2) +
            ((int)(piVar2[*(int *)(param_1 + 0x28)] +
                  (piVar2[*(int *)(param_1 + 0x28)] >> 0x1f & 3U)) >> 2);
    iVar9 = uVar5 + *(uint *)(iVar1 + 0x10);
    *(uint *)(iVar1 + 0x14) =
         ((int)uVar5 >> 0x1f) + *(int *)(iVar1 + 0x14) + (uint)CARRY4(uVar5,*(uint *)(iVar1 + 0x10))
    ;
  }
  *(int *)(iVar1 + 0x10) = iVar9;
  uVar5 = *(uint *)(param_1 + 0x38);
  if ((uVar5 & *(uint *)(param_1 + 0x3c)) == 0xffffffff) {
    uVar5 = param_2[0xf];
    if ((param_2[0xe] & uVar5) != 0xffffffff) {
      *(int *)(param_1 + 0x38) = param_2[0xe];
      *(uint *)(param_1 + 0x3c) = uVar5;
      if (((int)uVar5 <= *(int *)(iVar1 + 0x14)) &&
         (((int)uVar5 < *(int *)(iVar1 + 0x14) ||
          (*(uint *)(param_1 + 0x38) < *(uint *)(iVar1 + 0x10))))) {
        if (param_2[0xc] == 0) {
          iVar9 = *(int *)(param_1 + 0x18) + (*(int *)(iVar1 + 0x10) - *(int *)(param_1 + 0x38));
          *(int *)(param_1 + 0x18) = iVar9;
          if (*(int *)(param_1 + 0x14) < iVar9) {
            *(int *)(param_1 + 0x18) = *(int *)(param_1 + 0x14);
          }
        }
        else {
          *(int *)(param_1 + 0x14) =
               *(int *)(param_1 + 0x14) + (*(int *)(param_1 + 0x38) - *(int *)(iVar1 + 0x10));
        }
      }
    }
  }
  else {
    uVar10 = ((int)(piVar2[*(int *)(param_1 + 0x24)] +
                   (piVar2[*(int *)(param_1 + 0x24)] >> 0x1f & 3U)) >> 2) +
             ((int)(piVar2[*(int *)(param_1 + 0x28)] +
                   (piVar2[*(int *)(param_1 + 0x28)] >> 0x1f & 3U)) >> 2);
    *(uint *)(param_1 + 0x38) = uVar10 + uVar5;
    *(uint *)(param_1 + 0x3c) =
         ((int)uVar10 >> 0x1f) + *(int *)(param_1 + 0x3c) + (uint)CARRY4(uVar10,uVar5);
    uVar5 = param_2[0xe];
    uVar10 = param_2[0xf];
    if ((uVar5 & uVar10) != 0xffffffff) {
      uVar3 = *(uint *)(param_1 + 0x3c);
      if ((*(uint *)(param_1 + 0x38) != uVar5) || (uVar3 != uVar10)) {
        if (((((int)uVar10 <= (int)uVar3) &&
             (((int)uVar10 < (int)uVar3 || (uVar5 < *(uint *)(param_1 + 0x38))))) &&
            (iVar9 = *(int *)(param_1 + 0x38) - param_2[0xe], iVar9 != 0)) && (param_2[0xc] != 0)) {
          *(int *)(param_1 + 0x14) = *(int *)(param_1 + 0x14) - iVar9;
        }
        *(int *)(param_1 + 0x38) = param_2[0xe];
        *(int *)(param_1 + 0x3c) = param_2[0xf];
      }
    }
  }
  if (param_2[0xc] != 0) {
    *(undefined4 *)(param_1 + 0x20) = 1;
  }
  return 0;
}

