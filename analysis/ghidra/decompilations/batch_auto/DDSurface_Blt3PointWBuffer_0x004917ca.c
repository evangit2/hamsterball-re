
undefined4 DDSurface_Blt3PointWBuffer(void)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  float fVar10;
  float fVar11;
  float fVar12;
  float fVar13;
  float fVar14;
  uint uVar15;
  int iVar16;
  int *piVar17;
  undefined4 *puVar18;
  undefined4 uVar19;
  uint *puVar20;
  void *pvVar21;
  int iVar22;
  int *piVar23;
  int *extraout_ECX;
  int iVar24;
  int iVar25;
  int *piVar26;
  int unaff_EBP;
  float *pfVar27;
  float *pfVar28;
  int iVar29;
  
  __security_init_cookie();
  *(undefined4 *)(unaff_EBP + -0x44) = 0;
  *(undefined4 *)(unaff_EBP + -0x30) = 0;
  uVar15 = extraout_ECX[2];
  *(int **)(unaff_EBP + -0x54) = extraout_ECX;
  if ((char)uVar15 != '\x03') {
    uVar19 = 0x80004005;
    goto LAB_00491eee;
  }
  *(uint *)(unaff_EBP + -0x20) = ~(uVar15 >> 0x12) & 1;
  *(uint *)(unaff_EBP + -0x3c) = ~(uVar15 >> 0x11) & 1;
  puVar20 = ComputeScanlineZBuffer(~(uVar15 >> 0x10) & 1);
  *(uint **)(unaff_EBP + -0x48) = puVar20;
  puVar20 = ComputeScanlineZBuffer(*(int *)(unaff_EBP + -0x3c));
  *(uint **)(unaff_EBP + -0x4c) = puVar20;
  puVar20 = ComputeScanlineZBuffer(*(int *)(unaff_EBP + -0x20));
  *(uint **)(unaff_EBP + -0x3c) = puVar20;
  if (((*(int *)(unaff_EBP + -0x48) == 0) || (*(int *)(unaff_EBP + -0x4c) == 0)) ||
     (puVar20 == (uint *)0x0)) {
LAB_00491eba:
    uVar19 = 0x8007000e;
  }
  else {
    iVar24 = *(int *)(extraout_ECX[1] + 0x1058);
    pvVar21 = operator_new(iVar24 << 4);
    *(void **)(unaff_EBP + -0x44) = pvVar21;
    *(undefined4 *)(unaff_EBP + -4) = 0;
    if (pvVar21 == (void *)0x0) {
      *(undefined4 *)(unaff_EBP + -0x34) = 0;
      iVar24 = *(int *)(unaff_EBP + -0x34);
    }
    else {
      RepeatCall(pvVar21,0x10,iVar24,&LAB_0047d949);
      iVar24 = *(int *)(unaff_EBP + -0x44);
      *(int *)(unaff_EBP + -0x34) = iVar24;
    }
    *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
    *(int *)(unaff_EBP + -0x44) = iVar24;
    if (iVar24 == 0) goto LAB_00491eba;
    iVar24 = *(int *)(*extraout_ECX + 0x1058);
    pvVar21 = operator_new(iVar24 << 6);
    *(void **)(unaff_EBP + -0x30) = pvVar21;
    *(undefined4 *)(unaff_EBP + -4) = 1;
    if (pvVar21 == (void *)0x0) {
      iVar24 = 0;
    }
    else {
      RepeatCall(pvVar21,0x10,iVar24 << 2,&LAB_0047d949);
      iVar24 = *(int *)(unaff_EBP + -0x30);
    }
    *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
    *(int *)(unaff_EBP + -0x30) = iVar24;
    if (iVar24 == 0) goto LAB_00491eba;
    iVar25 = *(int *)(*extraout_ECX + 0x1058);
    *(undefined4 *)(unaff_EBP + -0x38) = 0;
    *(int *)(unaff_EBP + -0x1c) = iVar25 * 0x10 + iVar24;
    iVar29 = iVar25 * 0x30 + iVar24;
    *(int *)(unaff_EBP + -0x28) = iVar24;
    iVar22 = extraout_ECX[1];
    iVar16 = *(int *)(iVar22 + 0x1060);
    *(int *)(unaff_EBP + -0x40) = iVar25 * 0x20 + iVar24;
    *(int *)(unaff_EBP + -0x2c) = iVar29;
    if (iVar16 != 0) {
      *(int *)(unaff_EBP + -0x10) = *(int *)(unaff_EBP + -0x3c) + 8;
      piVar23 = extraout_ECX;
      do {
        *(undefined4 *)(unaff_EBP + -0x24) = 0;
        *(undefined4 *)(unaff_EBP + -0x18) = 0xffffffff;
        *(undefined4 *)(unaff_EBP + -0x14) = 0xffffffff;
        if (*(int *)(iVar22 + 0x105c) != 0) {
          piVar26 = (int *)(*(int *)(unaff_EBP + -0x4c) + 8);
          do {
            iVar24 = piVar26[-2];
            *(undefined4 *)(unaff_EBP + -0x20) = 0;
            if (iVar24 != *(int *)(unaff_EBP + -0x18)) {
              iVar25 = *(int *)(unaff_EBP + -0x14);
              if (iVar24 == iVar25) {
                *(undefined4 *)(unaff_EBP + -0x14) = 0xffffffff;
                *(int *)(unaff_EBP + -0x18) = iVar25;
                uVar19 = *(undefined4 *)(unaff_EBP + -0x1c);
                *(undefined4 *)(unaff_EBP + -0x1c) = *(undefined4 *)(unaff_EBP + -0x28);
                iVar24 = *(int *)(unaff_EBP + -0x40);
                *(int *)(unaff_EBP + -0x40) = iVar29;
                *(undefined4 *)(unaff_EBP + -0x28) = uVar19;
                *(int *)(unaff_EBP + -0x2c) = iVar24;
                iVar29 = iVar24;
              }
              else {
                uVar19 = *(undefined4 *)(*(int *)(unaff_EBP + -0x10) + -8);
                piVar17 = (int *)*piVar23;
                *(int *)(unaff_EBP + -0x18) = iVar24;
                (**(code **)(*piVar17 + 4))
                          (*(undefined4 *)(unaff_EBP + -0x18),uVar19,
                           *(undefined4 *)(unaff_EBP + -0x28));
                (**(code **)(*(int *)*piVar23 + 4))
                          (*(undefined4 *)(unaff_EBP + -0x18),**(undefined4 **)(unaff_EBP + -0x10),
                           *(undefined4 *)(unaff_EBP + -0x40));
              }
            }
            if (*piVar26 != *(int *)(unaff_EBP + -0x14)) {
              uVar19 = *(undefined4 *)(*(int *)(unaff_EBP + -0x10) + -8);
              piVar17 = (int *)*piVar23;
              *(int *)(unaff_EBP + -0x14) = *piVar26;
              (**(code **)(*piVar17 + 4))
                        (*(undefined4 *)(unaff_EBP + -0x14),uVar19,
                         *(undefined4 *)(unaff_EBP + -0x1c));
              (**(code **)(*(int *)*piVar23 + 4))
                        (*(undefined4 *)(unaff_EBP + -0x14),**(undefined4 **)(unaff_EBP + -0x10),
                         iVar29);
            }
            if (*(int *)(piVar23[1] + 0x1058) != 0) {
              piVar23 = (int *)(*(int *)(unaff_EBP + -0x48) + 8);
              *(undefined4 *)(unaff_EBP + -0x50) = *(undefined4 *)(unaff_EBP + -0x34);
              do {
                fVar1 = (float)piVar23[1];
                iVar24 = *piVar23 * 0x10;
                pfVar27 = (float *)(iVar24 + iVar29);
                *(float *)(unaff_EBP + -0x114) = fVar1 * *pfVar27;
                *(float *)(unaff_EBP + -0x110) = fVar1 * pfVar27[1];
                *(float *)(unaff_EBP + -0x10c) = fVar1 * pfVar27[2];
                fVar2 = pfVar27[3];
                fVar3 = (float)piVar23[-1];
                iVar25 = piVar23[-2] * 0x10;
                pfVar27 = (float *)(iVar25 + iVar29);
                fVar4 = *pfVar27;
                *(float *)(unaff_EBP + -0x60) = fVar3 * pfVar27[1];
                *(float *)(unaff_EBP + -0x5c) = fVar3 * pfVar27[2];
                pfVar28 = (float *)(iVar24 + *(int *)(unaff_EBP + -0x40));
                *(float *)(unaff_EBP + -0x58) = fVar3 * pfVar27[3];
                *(float *)(unaff_EBP + -0x94) = fVar3 * fVar4 + *(float *)(unaff_EBP + -0x114);
                *(float *)(unaff_EBP + -0x90) =
                     *(float *)(unaff_EBP + -0x60) + *(float *)(unaff_EBP + -0x110);
                *(float *)(unaff_EBP + -0x8c) =
                     *(float *)(unaff_EBP + -0x5c) + *(float *)(unaff_EBP + -0x10c);
                fVar3 = (float)piVar26[1];
                *(float *)(unaff_EBP + -0x174) = *(float *)(unaff_EBP + -0x94) * fVar3;
                *(float *)(unaff_EBP + -0x170) = *(float *)(unaff_EBP + -0x90) * fVar3;
                *(float *)(unaff_EBP + -0x16c) = *(float *)(unaff_EBP + -0x8c) * fVar3;
                fVar4 = (float)piVar23[1];
                *(float *)(unaff_EBP + -0x194) = fVar4 * *pfVar28;
                *(float *)(unaff_EBP + -400) = fVar4 * pfVar28[1];
                *(float *)(unaff_EBP + -0x18c) = fVar4 * pfVar28[2];
                fVar5 = pfVar28[3];
                pfVar27 = (float *)(iVar25 + *(int *)(unaff_EBP + -0x40));
                fVar6 = (float)piVar23[-1];
                fVar7 = *pfVar27;
                *(float *)(unaff_EBP + -0xb0) = fVar6 * pfVar27[1];
                *(float *)(unaff_EBP + -0xac) = fVar6 * pfVar27[2];
                *(float *)(unaff_EBP + -0xa8) = fVar6 * pfVar27[3];
                *(float *)(unaff_EBP + -0x134) = fVar6 * fVar7 + *(float *)(unaff_EBP + -0x194);
                *(float *)(unaff_EBP + -0x130) =
                     *(float *)(unaff_EBP + -0xb0) + *(float *)(unaff_EBP + -400);
                *(float *)(unaff_EBP + -300) =
                     *(float *)(unaff_EBP + -0xac) + *(float *)(unaff_EBP + -0x18c);
                fVar6 = (float)piVar26[-1];
                *(float *)(unaff_EBP + -0xd4) = *(float *)(unaff_EBP + -0x134) * fVar6;
                *(float *)(unaff_EBP + -0xd0) = *(float *)(unaff_EBP + -0x130) * fVar6;
                *(float *)(unaff_EBP + -0xcc) = *(float *)(unaff_EBP + -300) * fVar6;
                *(float *)(unaff_EBP + -0xf4) =
                     *(float *)(unaff_EBP + -0xd4) + *(float *)(unaff_EBP + -0x174);
                *(float *)(unaff_EBP + -0xf0) =
                     *(float *)(unaff_EBP + -0xd0) + *(float *)(unaff_EBP + -0x170);
                *(float *)(unaff_EBP + -0xec) =
                     *(float *)(unaff_EBP + -0xcc) + *(float *)(unaff_EBP + -0x16c);
                fVar7 = *(float *)(*(int *)(unaff_EBP + -0x10) + 4);
                *(float *)(unaff_EBP + -0x184) = *(float *)(unaff_EBP + -0xf4) * fVar7;
                *(float *)(unaff_EBP + -0x180) = *(float *)(unaff_EBP + -0xf0) * fVar7;
                *(float *)(unaff_EBP + -0x17c) = *(float *)(unaff_EBP + -0xec) * fVar7;
                fVar8 = (float)piVar23[1];
                pfVar27 = (float *)(*(int *)(unaff_EBP + -0x1c) + iVar24);
                *(float *)(unaff_EBP + -0x154) = fVar8 * *pfVar27;
                *(float *)(unaff_EBP + -0x150) = fVar8 * pfVar27[1];
                *(float *)(unaff_EBP + -0x14c) = fVar8 * pfVar27[2];
                fVar9 = pfVar27[3];
                fVar10 = (float)piVar23[-1];
                pfVar28 = (float *)(*(int *)(unaff_EBP + -0x1c) + iVar25);
                fVar11 = *pfVar28;
                *(float *)(unaff_EBP + -0x70) = fVar10 * pfVar28[1];
                *(float *)(unaff_EBP + -0x6c) = fVar10 * pfVar28[2];
                pfVar27 = (float *)(iVar24 + *(int *)(unaff_EBP + -0x28));
                *(float *)(unaff_EBP + -0x68) = fVar10 * pfVar28[3];
                *(float *)(unaff_EBP + -0x84) = fVar10 * fVar11 + *(float *)(unaff_EBP + -0x154);
                *(float *)(unaff_EBP + -0x80) =
                     *(float *)(unaff_EBP + -0x70) + *(float *)(unaff_EBP + -0x150);
                *(float *)(unaff_EBP + -0x7c) =
                     *(float *)(unaff_EBP + -0x6c) + *(float *)(unaff_EBP + -0x14c);
                fVar10 = (float)piVar26[1];
                *(float *)(unaff_EBP + -0x124) = *(float *)(unaff_EBP + -0x84) * fVar10;
                *(float *)(unaff_EBP + -0x120) = *(float *)(unaff_EBP + -0x80) * fVar10;
                *(float *)(unaff_EBP + -0x11c) = *(float *)(unaff_EBP + -0x7c) * fVar10;
                fVar11 = (float)piVar23[1];
                *(float *)(unaff_EBP + -0xa4) = fVar11 * *pfVar27;
                *(float *)(unaff_EBP + -0xa0) = fVar11 * pfVar27[1];
                *(float *)(unaff_EBP + -0x9c) = fVar11 * pfVar27[2];
                fVar12 = pfVar27[3];
                fVar13 = (float)piVar23[-1];
                pfVar27 = (float *)(iVar25 + *(int *)(unaff_EBP + -0x28));
                fVar14 = *pfVar27;
                *(float *)(unaff_EBP + -0xc0) = fVar13 * pfVar27[1];
                *(float *)(unaff_EBP + -0xbc) = fVar13 * pfVar27[2];
                *(float *)(unaff_EBP + -0xb8) = fVar13 * pfVar27[3];
                *(float *)(unaff_EBP + -0xe4) = fVar13 * fVar14 + *(float *)(unaff_EBP + -0xa4);
                *(float *)(unaff_EBP + -0xe0) =
                     *(float *)(unaff_EBP + -0xc0) + *(float *)(unaff_EBP + -0xa0);
                *(float *)(unaff_EBP + -0xdc) =
                     *(float *)(unaff_EBP + -0xbc) + *(float *)(unaff_EBP + -0x9c);
                fVar13 = (float)piVar26[-1];
                *(float *)(unaff_EBP + -0x104) = *(float *)(unaff_EBP + -0xe4) * fVar13;
                *(float *)(unaff_EBP + -0x100) = *(float *)(unaff_EBP + -0xe0) * fVar13;
                *(float *)(unaff_EBP + -0xfc) = *(float *)(unaff_EBP + -0xdc) * fVar13;
                *(float *)(unaff_EBP + -0x144) =
                     *(float *)(unaff_EBP + -0x104) + *(float *)(unaff_EBP + -0x124);
                *(float *)(unaff_EBP + -0x140) =
                     *(float *)(unaff_EBP + -0x100) + *(float *)(unaff_EBP + -0x120);
                *(float *)(unaff_EBP + -0x13c) =
                     *(float *)(unaff_EBP + -0xfc) + *(float *)(unaff_EBP + -0x11c);
                fVar14 = *(float *)(*(int *)(unaff_EBP + -0x10) + -4);
                *(float *)(unaff_EBP + -0x164) = *(float *)(unaff_EBP + -0x144) * fVar14;
                *(float *)(unaff_EBP + -0x160) = *(float *)(unaff_EBP + -0x140) * fVar14;
                *(float *)(unaff_EBP + -0x15c) = *(float *)(unaff_EBP + -0x13c) * fVar14;
                puVar18 = *(undefined4 **)(unaff_EBP + -0x50);
                iVar24 = *(int *)(unaff_EBP + -0x54);
                *(int *)(unaff_EBP + -0x50) = *(int *)(unaff_EBP + -0x50) + 0x10;
                *(float *)(unaff_EBP + -0x1a4) =
                     *(float *)(unaff_EBP + -0x164) + *(float *)(unaff_EBP + -0x184);
                piVar23 = piVar23 + 4;
                *(int *)(unaff_EBP + -0x20) = *(int *)(unaff_EBP + -0x20) + 1;
                uVar15 = *(uint *)(unaff_EBP + -0x20);
                *(float *)(unaff_EBP + -0x1a0) =
                     *(float *)(unaff_EBP + -0x160) + *(float *)(unaff_EBP + -0x180);
                *(float *)(unaff_EBP + -0x19c) =
                     *(float *)(unaff_EBP + -0x15c) + *(float *)(unaff_EBP + -0x17c);
                *(float *)(unaff_EBP + -0x198) =
                     ((*(float *)(unaff_EBP + -0xb8) + fVar11 * fVar12) * fVar13 +
                     (*(float *)(unaff_EBP + -0x68) + fVar8 * fVar9) * fVar10) * fVar14 +
                     ((*(float *)(unaff_EBP + -0xa8) + fVar4 * fVar5) * fVar6 +
                     (*(float *)(unaff_EBP + -0x58) + fVar1 * fVar2) * fVar3) * fVar7;
                *puVar18 = *(undefined4 *)(unaff_EBP + -0x1a4);
                puVar18[1] = *(undefined4 *)(unaff_EBP + -0x1a0);
                puVar18[2] = *(undefined4 *)(unaff_EBP + -0x19c);
                puVar18[3] = *(undefined4 *)(unaff_EBP + -0x198);
                iVar29 = *(int *)(unaff_EBP + -0x2c);
              } while (uVar15 < *(uint *)(*(int *)(iVar24 + 4) + 0x1058));
              piVar23 = *(int **)(unaff_EBP + -0x54);
            }
            (**(code **)(*(int *)piVar23[1] + 8))
                      (*(undefined4 *)(unaff_EBP + -0x24),*(undefined4 *)(unaff_EBP + -0x38),
                       *(undefined4 *)(unaff_EBP + -0x34));
            iVar24 = piVar23[1];
            piVar26 = piVar26 + 4;
            *(int *)(unaff_EBP + -0x24) = *(int *)(unaff_EBP + -0x24) + 1;
          } while (*(uint *)(unaff_EBP + -0x24) < *(uint *)(iVar24 + 0x105c));
        }
        *(int *)(unaff_EBP + -0x10) = *(int *)(unaff_EBP + -0x10) + 0x10;
        *(int *)(unaff_EBP + -0x38) = *(int *)(unaff_EBP + -0x38) + 1;
        iVar22 = piVar23[1];
      } while (*(uint *)(unaff_EBP + -0x38) < *(uint *)(iVar22 + 0x1060));
    }
    uVar19 = 0;
  }
  _free(*(void **)(unaff_EBP + -0x48));
  _free(*(void **)(unaff_EBP + -0x4c));
  _free(*(void **)(unaff_EBP + -0x3c));
  _free(*(void **)(unaff_EBP + -0x44));
  _free(*(void **)(unaff_EBP + -0x30));
LAB_00491eee:
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return uVar19;
}

