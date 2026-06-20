
undefined4 SDFGrid_MultiplySparse(void)

{
  int iVar1;
  float fVar2;
  int *piVar3;
  int *piVar4;
  void *pvVar5;
  int *extraout_ECX;
  uint uVar6;
  int iVar7;
  int *piVar8;
  int *piVar9;
  float *pfVar10;
  int unaff_EBP;
  float *pfVar11;
  undefined4 uVar12;
  undefined4 *puVar13;
  
  __security_init_cookie();
  iVar7 = extraout_ECX[1];
  iVar1 = *(int *)(iVar7 + 0x1060);
  *(undefined4 *)(unaff_EBP + -0x20) = 0;
  *(undefined4 *)(unaff_EBP + -0x34) = 0;
  *(undefined4 *)(unaff_EBP + -0x18) = 0;
  *(undefined4 *)(unaff_EBP + -0x3c) = 0;
  if ((iVar1 != 1) || (*(int *)(*extraout_ECX + 0x1060) != 1)) {
    uVar12 = 0x80004005;
    goto LAB_0049228e;
  }
  uVar6 = extraout_ECX[2];
  piVar3 = BuildSDFGrid(*(uint *)(*extraout_ECX + 0x1058),*(int *)(iVar7 + 0x1058),
                        ~(uVar6 >> 0x10) & 1);
  *(int **)(unaff_EBP + -0x38) = piVar3;
  if (piVar3 == (int *)0x0) {
LAB_00492250:
    uVar12 = 0x80004005;
  }
  else {
    piVar4 = BuildSDFGrid(*(uint *)(*extraout_ECX + 0x105c),*(int *)(extraout_ECX[1] + 0x105c),
                          ~(uVar6 >> 0x11) & 1);
    *(int **)(unaff_EBP + -0x20) = piVar4;
    if (piVar4 == (int *)0x0) goto LAB_00492250;
    *(int *)(unaff_EBP + -0x2c) = *piVar3 + (int)piVar3;
    piVar4 = (int *)(**(int **)(unaff_EBP + -0x20) + (int)*(int **)(unaff_EBP + -0x20));
    iVar7 = *(int *)(extraout_ECX[1] + 0x105c);
    *(int **)(unaff_EBP + -0x44) = piVar4;
    piVar3 = operator_new(iVar7 * 0xc + 4);
    *(int **)(unaff_EBP + -0x40) = piVar3;
    *(undefined4 *)(unaff_EBP + -4) = 0;
    if (piVar3 == (int *)0x0) {
      *(undefined4 *)(unaff_EBP + -0x14) = 0;
      iVar7 = *(int *)(unaff_EBP + -0x14);
    }
    else {
      *piVar3 = iVar7;
      *(int **)(unaff_EBP + -0x34) = piVar3 + 1;
      _eh_vector_constructor_iterator_(piVar3 + 1,0xc,iVar7,FUN_0048eb0a,FUN_0048eb17);
      iVar7 = *(int *)(unaff_EBP + -0x34);
      *(int *)(unaff_EBP + -0x14) = iVar7;
    }
    *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
    *(int *)(unaff_EBP + -0x34) = iVar7;
    if (iVar7 == 0) {
LAB_00492249:
      uVar12 = 0x8007000e;
    }
    else {
      iVar7 = *(int *)(*extraout_ECX + 0x1058);
      pvVar5 = operator_new(iVar7 << 4);
      *(void **)(unaff_EBP + -0x3c) = pvVar5;
      *(undefined4 *)(unaff_EBP + -4) = 1;
      if (pvVar5 == (void *)0x0) {
        *(undefined4 *)(unaff_EBP + -0x24) = 0;
        iVar7 = *(int *)(unaff_EBP + -0x24);
      }
      else {
        RepeatCall(pvVar5,0x10,iVar7,&LAB_0047d949);
        iVar7 = *(int *)(unaff_EBP + -0x3c);
        *(int *)(unaff_EBP + -0x24) = iVar7;
      }
      *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
      *(int *)(unaff_EBP + -0x3c) = iVar7;
      if (iVar7 == 0) goto LAB_00492249;
      piVar3 = (int *)(*(int *)(unaff_EBP + -0x20) + 4);
      *(int **)(unaff_EBP + -0x30) = piVar3;
      while (piVar3 < piVar4) {
        piVar9 = (int *)(*piVar3 + (int)piVar3);
        for (piVar8 = piVar3 + 1; piVar3 = piVar9, piVar8 < piVar9; piVar8 = piVar8 + 2) {
          piVar3 = (int *)(*(int *)(unaff_EBP + -0x14) + 8 + *piVar8 * 0xc);
          *piVar3 = *piVar3 + 1;
        }
      }
      piVar3 = *(int **)(unaff_EBP + -0x30);
      *(undefined4 *)(unaff_EBP + -0x28) = 0;
      if (piVar3 < piVar4) {
        do {
          iVar7 = *piVar3;
          piVar4 = piVar3 + 1;
          *(int **)(unaff_EBP + -0x1c) = (int *)(iVar7 + (int)piVar3);
          *(int **)(unaff_EBP + -0x30) = piVar4;
          piVar3 = (int *)(iVar7 + (int)piVar3);
          while (*(int **)(unaff_EBP + -0x10) = piVar4, piVar4 < piVar3) {
            piVar4 = (int *)(*(int *)(unaff_EBP + -0x14) + *piVar4 * 0xc);
            if (*piVar4 == 0) {
              piVar3 = *(int **)(unaff_EBP + -0x18);
              if (piVar3 == (int *)0x0) {
                iVar7 = *(int *)(extraout_ECX[1] + 0x1058);
                pvVar5 = operator_new(iVar7 << 4);
                *(void **)(unaff_EBP + -0x40) = pvVar5;
                *(undefined4 *)(unaff_EBP + -4) = 2;
                if (pvVar5 == (void *)0x0) {
                  iVar7 = 0;
                }
                else {
                  RepeatCall(pvVar5,0x10,iVar7,&LAB_0047d949);
                  iVar7 = *(int *)(unaff_EBP + -0x40);
                }
                *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
                *piVar4 = iVar7;
                if (iVar7 == 0) goto LAB_00492249;
              }
              else {
                *piVar4 = *piVar3;
                *piVar3 = 0;
                *(int *)(unaff_EBP + -0x18) = piVar3[1];
              }
              piVar3 = *(int **)(unaff_EBP + -0x1c);
              puVar13 = (undefined4 *)*piVar4;
              for (uVar6 = (uint)(*(int *)(extraout_ECX[1] + 0x1058) << 4) >> 2; uVar6 != 0;
                  uVar6 = uVar6 - 1) {
                *puVar13 = 0;
                puVar13 = puVar13 + 1;
              }
              for (iVar7 = 0; iVar7 != 0; iVar7 = iVar7 + -1) {
                *(undefined1 *)puVar13 = 0;
                puVar13 = (undefined4 *)((int)puVar13 + 1);
              }
            }
            piVar4 = (int *)(*(int *)(unaff_EBP + -0x10) + 8);
          }
          iVar7 = *(int *)(unaff_EBP + -0x24);
          (**(code **)(*(int *)*extraout_ECX + 4))(*(undefined4 *)(unaff_EBP + -0x28),0,iVar7);
          piVar4 = (int *)(*(int *)(unaff_EBP + -0x38) + 4);
          if (piVar4 < *(int **)(unaff_EBP + -0x2c)) {
            pfVar10 = (float *)(iVar7 + 8);
            do {
              piVar8 = *(int **)(unaff_EBP + -0x30);
              piVar9 = (int *)(*piVar4 + (int)piVar4);
              *(int **)(unaff_EBP + -0x10) = piVar8;
              if (piVar8 < piVar3) {
                *(int **)(unaff_EBP + -0x40) = piVar4 + 1;
                while( true ) {
                  iVar7 = *(int *)(*(int *)(unaff_EBP + -0x14) + *piVar8 * 0xc);
                  piVar4 = *(int **)(unaff_EBP + -0x40);
                  if (piVar4 < piVar9) {
                    do {
                      fVar2 = (float)piVar4[1] * *(float *)(*(int *)(unaff_EBP + -0x10) + 4);
                      pfVar11 = (float *)(*piVar4 * 0x10 + iVar7);
                      *pfVar11 = fVar2 * pfVar10[-2] + *pfVar11;
                      pfVar11 = (float *)(*piVar4 * 0x10 + 4 + iVar7);
                      *pfVar11 = fVar2 * pfVar10[-1] + *pfVar11;
                      pfVar11 = (float *)(*piVar4 * 0x10 + 8 + iVar7);
                      *pfVar11 = fVar2 * *pfVar10 + *pfVar11;
                      pfVar11 = (float *)(*piVar4 * 0x10 + 0xc + iVar7);
                      piVar4 = piVar4 + 2;
                      *pfVar11 = fVar2 * pfVar10[1] + *pfVar11;
                    } while (piVar4 < piVar9);
                    piVar3 = *(int **)(unaff_EBP + -0x1c);
                  }
                  *(int *)(unaff_EBP + -0x10) = *(int *)(unaff_EBP + -0x10) + 8;
                  if (piVar3 <= *(int **)(unaff_EBP + -0x10)) break;
                  piVar8 = *(int **)(unaff_EBP + -0x10);
                }
              }
              pfVar10 = pfVar10 + 4;
              piVar4 = piVar9;
            } while (piVar9 < *(int **)(unaff_EBP + -0x2c));
          }
          piVar4 = *(int **)(unaff_EBP + -0x30);
          if (piVar4 < piVar3) {
            do {
              puVar13 = (undefined4 *)(*(int *)(unaff_EBP + -0x14) + *piVar4 * 0xc);
              piVar3 = puVar13 + 2;
              *piVar3 = *piVar3 + -1;
              if (*piVar3 == 0) {
                (**(code **)(*(int *)extraout_ECX[1] + 8))(*piVar4,0,*puVar13);
                puVar13[1] = *(undefined4 *)(unaff_EBP + -0x18);
                *(undefined4 **)(unaff_EBP + -0x18) = puVar13;
              }
              piVar4 = piVar4 + 2;
            } while (piVar4 < *(int **)(unaff_EBP + -0x1c));
            piVar3 = *(int **)(unaff_EBP + -0x1c);
          }
          *(int *)(unaff_EBP + -0x28) = *(int *)(unaff_EBP + -0x28) + 1;
        } while (piVar3 < *(int **)(unaff_EBP + -0x44));
      }
      uVar12 = 0;
    }
  }
  if (*(void **)(unaff_EBP + -0x34) != (void *)0x0) {
    D3DX_SkinWeights_VectorDtor(*(void **)(unaff_EBP + -0x34),3);
  }
  _free(*(void **)(unaff_EBP + -0x20));
  _free(*(void **)(unaff_EBP + -0x38));
  _free(*(void **)(unaff_EBP + -0x3c));
  _free((void *)0x0);
LAB_0049228e:
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return uVar12;
}

