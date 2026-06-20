
undefined4 D3DX_TransformTex_Bilinear(void)

{
  float *pfVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  uint uVar9;
  int iVar10;
  int iVar11;
  int *piVar12;
  undefined4 *puVar13;
  undefined4 uVar14;
  uint *puVar15;
  void *pvVar16;
  int iVar17;
  int *piVar18;
  int *extraout_ECX;
  float *pfVar19;
  int iVar20;
  int unaff_EBP;
  int *piVar21;
  
  __security_init_cookie();
  *(undefined4 *)(unaff_EBP + -0x14) = 0;
  *(undefined4 *)(unaff_EBP + -0x1c) = 0;
  uVar9 = extraout_ECX[2];
  *(int **)(unaff_EBP + -0x40) = extraout_ECX;
  if ((char)uVar9 != '\x03') {
    uVar14 = 0x80004005;
    goto LAB_004917bd;
  }
  *(uint *)(unaff_EBP + -0x10) = ~(uVar9 >> 0x11) & 1;
  puVar15 = ComputeScanlineZBuffer(~(uVar9 >> 0x10) & 1);
  *(uint **)(unaff_EBP + -0x30) = puVar15;
  puVar15 = ComputeScanlineZBuffer(*(int *)(unaff_EBP + -0x10));
  *(uint **)(unaff_EBP + -0x10) = puVar15;
  if ((*(int *)(unaff_EBP + -0x30) == 0) || (puVar15 == (uint *)0x0)) {
LAB_00491791:
    uVar14 = 0x8007000e;
  }
  else {
    iVar17 = *(int *)(extraout_ECX[1] + 0x1058);
    pvVar16 = operator_new(iVar17 << 4);
    *(void **)(unaff_EBP + -0x14) = pvVar16;
    *(undefined4 *)(unaff_EBP + -4) = 0;
    if (pvVar16 == (void *)0x0) {
      *(undefined4 *)(unaff_EBP + -0x20) = 0;
      iVar17 = *(int *)(unaff_EBP + -0x20);
    }
    else {
      RepeatCall(pvVar16,0x10,iVar17,&LAB_0047d949);
      iVar17 = *(int *)(unaff_EBP + -0x14);
      *(int *)(unaff_EBP + -0x20) = iVar17;
    }
    *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
    *(int *)(unaff_EBP + -0x14) = iVar17;
    if (iVar17 == 0) goto LAB_00491791;
    iVar17 = *(int *)(*extraout_ECX + 0x1058);
    pvVar16 = operator_new(iVar17 << 5);
    *(void **)(unaff_EBP + -0x1c) = pvVar16;
    *(undefined4 *)(unaff_EBP + -4) = 1;
    if (pvVar16 == (void *)0x0) {
      iVar17 = 0;
    }
    else {
      RepeatCall(pvVar16,0x10,iVar17 << 1,&LAB_0047d949);
      iVar17 = *(int *)(unaff_EBP + -0x1c);
    }
    *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
    *(int *)(unaff_EBP + -0x1c) = iVar17;
    if (iVar17 == 0) goto LAB_00491791;
    iVar10 = *(int *)(*extraout_ECX + 0x1058);
    *(undefined4 *)(unaff_EBP + -0x28) = 0;
    *(undefined4 *)(unaff_EBP + -0x24) = 0xffffffff;
    *(undefined4 *)(unaff_EBP + -0x18) = 0xffffffff;
    *(int *)(unaff_EBP + -0x2c) = iVar17;
    if (*(int *)(extraout_ECX[1] + 0x105c) != 0) {
      piVar21 = (int *)(*(int *)(unaff_EBP + -0x10) + 8);
      *(int **)(unaff_EBP + -0x38) = piVar21;
      iVar17 = iVar10 * 0x10 + iVar17;
      piVar18 = extraout_ECX;
      do {
        iVar10 = piVar21[-2];
        *(undefined4 *)(unaff_EBP + -0x34) = 0;
        iVar20 = iVar17;
        if (iVar10 != *(int *)(unaff_EBP + -0x24)) {
          iVar11 = *(int *)(unaff_EBP + -0x18);
          if (iVar10 == iVar11) {
            iVar20 = *(int *)(unaff_EBP + -0x2c);
            *(undefined4 *)(unaff_EBP + -0x18) = 0xffffffff;
            *(int *)(unaff_EBP + -0x2c) = iVar17;
            *(int *)(unaff_EBP + -0x24) = iVar11;
          }
          else {
            piVar12 = (int *)*piVar18;
            *(int *)(unaff_EBP + -0x24) = iVar10;
            (**(code **)(*piVar12 + 4))
                      (*(undefined4 *)(unaff_EBP + -0x24),0,*(undefined4 *)(unaff_EBP + -0x2c));
          }
        }
        iVar17 = *piVar21;
        if (iVar17 != *(int *)(unaff_EBP + -0x18)) {
          iVar10 = *(int *)*piVar18;
          *(int *)(unaff_EBP + -0x18) = iVar17;
          (**(code **)(iVar10 + 4))(iVar17,0,iVar20);
        }
        if (*(int *)(piVar18[1] + 0x1058) != 0) {
          piVar18 = (int *)(*(int *)(unaff_EBP + -0x30) + 8);
          *(undefined4 *)(unaff_EBP + -0x3c) = *(undefined4 *)(unaff_EBP + -0x20);
          do {
            iVar17 = *piVar18;
            fVar2 = (float)piVar18[1];
            pfVar1 = (float *)(iVar17 * 0x10 + iVar20);
            *(float *)(unaff_EBP + -0x50) = fVar2 * *pfVar1;
            *(float *)(unaff_EBP + -0x4c) = fVar2 * pfVar1[1];
            *(float *)(unaff_EBP + -0x48) = fVar2 * pfVar1[2];
            fVar3 = pfVar1[3];
            iVar10 = piVar18[-2];
            fVar4 = (float)piVar18[-1];
            pfVar1 = (float *)(iVar10 * 0x10 + iVar20);
            fVar5 = *pfVar1;
            *(float *)(unaff_EBP + -0x9c) = fVar4 * pfVar1[1];
            *(float *)(unaff_EBP + -0x98) = fVar4 * pfVar1[2];
            pfVar19 = (float *)(iVar17 * 0x10 + *(int *)(unaff_EBP + -0x2c));
            *(float *)(unaff_EBP + -0x94) = fVar4 * pfVar1[3];
            *(float *)(unaff_EBP + -0x80) = fVar4 * fVar5 + *(float *)(unaff_EBP + -0x50);
            *(float *)(unaff_EBP + -0x7c) =
                 *(float *)(unaff_EBP + -0x9c) + *(float *)(unaff_EBP + -0x4c);
            *(float *)(unaff_EBP + -0x78) =
                 *(float *)(unaff_EBP + -0x98) + *(float *)(unaff_EBP + -0x48);
            fVar4 = (float)piVar21[1];
            *(float *)(unaff_EBP + -0xb0) = *(float *)(unaff_EBP + -0x80) * fVar4;
            *(float *)(unaff_EBP + -0xac) = *(float *)(unaff_EBP + -0x7c) * fVar4;
            *(float *)(unaff_EBP + -0xa8) = *(float *)(unaff_EBP + -0x78) * fVar4;
            fVar5 = (float)piVar18[1];
            *(float *)(unaff_EBP + -0xc0) = fVar5 * *pfVar19;
            *(float *)(unaff_EBP + -0xbc) = fVar5 * pfVar19[1];
            *(float *)(unaff_EBP + -0xb8) = fVar5 * pfVar19[2];
            fVar6 = pfVar19[3];
            pfVar1 = (float *)(iVar10 * 0x10 + *(int *)(unaff_EBP + -0x2c));
            fVar7 = (float)piVar18[-1];
            piVar18 = piVar18 + 4;
            fVar8 = *pfVar1;
            *(int *)(unaff_EBP + -0x34) = *(int *)(unaff_EBP + -0x34) + 1;
            uVar9 = *(uint *)(unaff_EBP + -0x34);
            *(float *)(unaff_EBP + -0x5c) = fVar7 * pfVar1[1];
            *(float *)(unaff_EBP + -0x58) = fVar7 * pfVar1[2];
            iVar17 = *(int *)(unaff_EBP + -0x40);
            *(float *)(unaff_EBP + -0x54) = fVar7 * pfVar1[3];
            *(float *)(unaff_EBP + -0x70) = fVar7 * fVar8 + *(float *)(unaff_EBP + -0xc0);
            *(float *)(unaff_EBP + -0x6c) =
                 *(float *)(unaff_EBP + -0x5c) + *(float *)(unaff_EBP + -0xbc);
            *(float *)(unaff_EBP + -0x68) =
                 *(float *)(unaff_EBP + -0x58) + *(float *)(unaff_EBP + -0xb8);
            fVar7 = (float)piVar21[-1];
            puVar13 = *(undefined4 **)(unaff_EBP + -0x3c);
            *(int *)(unaff_EBP + -0x3c) = *(int *)(unaff_EBP + -0x3c) + 0x10;
            *(float *)(unaff_EBP + -0x90) = *(float *)(unaff_EBP + -0x70) * fVar7;
            *(float *)(unaff_EBP + -0x8c) = *(float *)(unaff_EBP + -0x6c) * fVar7;
            *(float *)(unaff_EBP + -0x88) = *(float *)(unaff_EBP + -0x68) * fVar7;
            *(float *)(unaff_EBP + -0xd0) =
                 *(float *)(unaff_EBP + -0x90) + *(float *)(unaff_EBP + -0xb0);
            *(float *)(unaff_EBP + -0xcc) =
                 *(float *)(unaff_EBP + -0x8c) + *(float *)(unaff_EBP + -0xac);
            *(float *)(unaff_EBP + -200) =
                 *(float *)(unaff_EBP + -0x88) + *(float *)(unaff_EBP + -0xa8);
            *(float *)(unaff_EBP + -0xc4) =
                 (*(float *)(unaff_EBP + -0x54) + fVar5 * fVar6) * fVar7 +
                 (*(float *)(unaff_EBP + -0x94) + fVar2 * fVar3) * fVar4;
            *puVar13 = *(undefined4 *)(unaff_EBP + -0xd0);
            puVar13[1] = *(undefined4 *)(unaff_EBP + -0xcc);
            puVar13[2] = *(undefined4 *)(unaff_EBP + -200);
            puVar13[3] = *(undefined4 *)(unaff_EBP + -0xc4);
            piVar21 = *(int **)(unaff_EBP + -0x38);
          } while (uVar9 < *(uint *)(*(int *)(iVar17 + 4) + 0x1058));
          piVar18 = *(int **)(unaff_EBP + -0x40);
        }
        (**(code **)(*(int *)piVar18[1] + 8))
                  (*(undefined4 *)(unaff_EBP + -0x28),0,*(undefined4 *)(unaff_EBP + -0x20));
        iVar17 = piVar18[1];
        piVar21 = piVar21 + 4;
        *(int *)(unaff_EBP + -0x28) = *(int *)(unaff_EBP + -0x28) + 1;
        uVar9 = *(uint *)(iVar17 + 0x105c);
        *(int **)(unaff_EBP + -0x38) = piVar21;
        iVar17 = iVar20;
      } while (*(uint *)(unaff_EBP + -0x28) < uVar9);
    }
    uVar14 = 0;
  }
  _free(*(void **)(unaff_EBP + -0x30));
  _free(*(void **)(unaff_EBP + -0x10));
  _free(*(void **)(unaff_EBP + -0x14));
  _free(*(void **)(unaff_EBP + -0x1c));
LAB_004917bd:
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return uVar14;
}

