
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall
Scene_Render3DObjects
          (void *this,int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
          undefined4 param_5,undefined4 param_6,char param_7)

{
  char cVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  float *pfVar10;
  void *this_00;
  int iVar11;
  int iVar12;
  float *pfVar13;
  float unaff_EBX;
  float *pfVar14;
  float *pfVar15;
  int iVar16;
  float *pfVar17;
  int *piVar18;
  float *pfVar19;
  bool bVar20;
  float10 extraout_ST0;
  float10 extraout_ST0_00;
  float10 extraout_ST0_01;
  float10 extraout_ST0_02;
  float10 extraout_ST0_03;
  float10 extraout_ST0_04;
  ulonglong uVar21;
  float unaff_retaddr;
  void *pvStack_f8;
  float fStack_f0;
  void *local_ec;
  int iStack_e4;
  float fStack_dc;
  float fStack_d4;
  float fStack_c0;
  float fStack_bc;
  float fStack_b8;
  float fStack_b0;
  float fStack_ac;
  float fStack_a8;
  float fStack_a0;
  float fStack_9c;
  float fStack_98;
  float fStack_90;
  float fStack_8c;
  float fStack_88;
  float fStack_80;
  float fStack_74;
  undefined4 uStack_68;
  undefined4 auStack_5c [18];
  undefined4 uStack_14;
  void *local_c;
  undefined1 *puStack_8;
  float fStack_4;
  
  fStack_4 = -NAN;
  puStack_8 = &LAB_004ccec8;
  local_c = ExceptionList;
  if (*(char *)(*(int *)((int)this + 0x480) + 0x10d0) == '\0') {
    ExceptionList = &local_c;
    iVar7 = AthenaList_NextIndex(*(int *)((int)this + 0x480) + 0x1c);
    iVar8 = *(int *)((int)this + 0x480);
    *(undefined4 *)(iVar8 + 0x24 + iVar7 * 4) = 0;
    if (*(int *)(iVar8 + 0x20) < 1) {
      this_00 = (void *)0x0;
    }
    else {
      this_00 = (void *)**(undefined4 **)(iVar8 + 0x428);
      *(undefined4 *)(iVar8 + 0x24 + iVar7 * 4) = 1;
    }
    while (this_00 != (void *)0x0) {
      Scene_Render3DObjects(this_00,param_1,param_2,param_3,param_4,param_5,param_6,'\x01');
      iVar8 = *(int *)((int)this + 0x480);
      iVar12 = *(int *)(iVar8 + 0x24 + iVar7 * 4);
      if (*(int *)(iVar8 + 0x20) <= iVar12) break;
      this_00 = *(void **)(*(int *)(iVar8 + 0x428) + iVar12 * 4);
      *(int *)(iVar8 + 0x24 + iVar7 * 4) = iVar12 + 1;
    }
    if (param_7 == '\x01') {
      DAT_005341c8 = 0;
    }
    Graphics_BeginFrame(*(void **)((int)this + 4),*(int *)((int)this + 0x434));
    iVar8 = (**(code **)(*(int *)this + 0x24))();
    if (iVar8 != 0) {
      piVar18 = (int *)0x0;
      if (*(char *)((int)this + 0x430) == '\x01') {
        *(undefined4 *)((int)this + 0x20) = 0;
        if (0 < *(int *)((int)this + 0x1c)) {
          piVar18 = (int *)**(undefined4 **)((int)this + 0x424);
          *(undefined4 *)((int)this + 0x20) = 1;
        }
        while (local_ec = this, piVar18 != (int *)0x0) {
          iVar8 = piVar18[2];
          fVar2 = *(float *)(iVar8 + 0x464);
          uStack_68 = *(undefined4 *)(iVar8 + 0x45c);
          fStack_74 = *(float *)(iVar8 + 0x468);
          D3D_Thunk_5();
          fVar4 = fStack_c0 * (float)puStack_8 + fStack_b0 * fStack_4 + unaff_retaddr * fStack_a0 +
                  fStack_90;
          fVar5 = (float)puStack_8 * fStack_bc + unaff_retaddr * fStack_9c + fStack_4 * fStack_ac +
                  fStack_8c;
          fVar3 = (float)puStack_8 * fStack_b8 + unaff_retaddr * fStack_98 + fStack_4 * fStack_a8 +
                  fStack_88;
          if (((((fStack_74 - unaff_EBX < fVar4) && (fVar4 < unaff_EBX + fStack_80)) &&
               (fStack_f0 - unaff_EBX < fVar3)) &&
              ((fVar3 < fStack_dc + unaff_EBX && (fStack_d4 < fVar5)))) &&
             ((fVar5 < fVar2 + _DAT_004d8fac &&
              (iVar8 = (**(code **)(*piVar18 + 0x24))(), iVar8 != 0)))) {
            Scene_Render3DObjects(piVar18,param_1,param_2,param_3,param_4,param_5,param_6,'\0');
          }
          iVar8 = *(int *)((int)this + 0x20);
          if (*(int *)((int)this + 0x1c) <= iVar8) break;
          piVar18 = *(int **)(*(int *)((int)this + 0x424) + iVar8 * 4);
          *(int *)((int)this + 0x20) = iVar8 + 1;
        }
      }
      else {
        D3D_Thunk_5();
        Matrix_TransformVec3(&fStack_c0,(float *)&puStack_8);
        fVar5 = (float)param_1;
        fVar2 = (float)puStack_8 - fVar5;
        iVar8 = *(int *)((int)this + 8) + 0x2c;
        local_ec = (void *)((float)puStack_8 + fVar5);
        fVar4 = unaff_retaddr - fVar5;
        fVar5 = fVar5 + unaff_retaddr;
        if (*(char *)(*(int *)((int)this + 8) + 0x459) == '\x01') {
          iVar7 = AthenaList_NextIndex(iVar8);
          iVar8 = *(int *)((int)this + 8);
          *(undefined4 *)(iVar8 + 0x34 + iVar7 * 4) = 0;
          if (*(int *)(iVar8 + 0x30) < 1) {
            iVar12 = 0;
          }
          else {
            iVar12 = **(int **)(iVar8 + 0x438);
            *(undefined4 *)(iVar8 + 0x34 + iVar7 * 4) = 1;
          }
          while (iVar12 != 0) {
            if ((*(char *)(iVar12 + 0x863) == '\0') && (*(char *)(iVar12 + 0x85e) == '\0')) {
              iVar8 = AthenaList_NextIndex(iVar12 + 0x424);
              *(undefined4 *)(iVar12 + 0x42c + iVar8 * 4) = 0;
              if (*(int *)(iVar12 + 0x428) < 1) {
                iVar11 = 0;
              }
              else {
                iVar11 = **(int **)(iVar12 + 0x830);
                *(undefined4 *)(iVar12 + 0x42c + iVar8 * 4) = 1;
              }
              while (iVar11 != 0) {
                if (*(char *)(*(int *)((int)this + 0x480) + 0x434) == '\0') {
                  iVar9 = *(int *)(iVar11 + 8);
                  iVar16 = *(int *)(*(int *)((int)this + 8) + 0x448);
                }
                else {
                  iVar9 = *(int *)(iVar11 + 0xc);
                  iVar16 = *(int *)(*(int *)(*(int *)((int)this + 0x47c) + 0x480) + 0x440);
                }
                pfVar10 = (float *)(iVar9 * 0x20 + iVar16);
                bVar20 = false;
                iStack_e4 = 0;
                if (0 < *(int *)(iVar11 + 4)) {
                  pfVar14 = pfVar10 + 0x10;
                  pfVar13 = (float *)(&DAT_004fe1cc + DAT_005341c8 * 9);
                  do {
                    if (bVar20) {
                      pfVar19 = pfVar14 + -8;
                      pfVar15 = pfVar14;
                    }
                    else {
                      pfVar15 = pfVar14 + -8;
                      pfVar19 = pfVar14;
                    }
                    bVar20 = bVar20 == false;
                    pfVar13[-1] = *pfVar10;
                    *pfVar13 = pfVar14[-0xf];
                    pfVar13[1] = pfVar14[-0xe];
                    pfVar13[8] = *pfVar15;
                    pfVar13[9] = pfVar15[1];
                    pfVar13[10] = pfVar15[2];
                    pfVar13[0x11] = *pfVar19;
                    pfVar13[0x12] = pfVar19[1];
                    fVar3 = pfVar19[2];
                    pfVar13[0x13] = fVar3;
                    pfVar14 = pfVar14 + 8;
                    pfVar17 = pfVar13;
                    if (((((((pfVar10[4] < _DAT_004cf368 == (pfVar10[4] == _DAT_004cf368)) ||
                            (pfVar15[4] < _DAT_004cf368 == (pfVar15[4] == _DAT_004cf368))) ||
                           (pfVar19[4] < _DAT_004cf368 == (pfVar19[4] == _DAT_004cf368))) &&
                          ((((*pfVar13 <= fStack_4 || (pfVar13[9] <= fStack_4)) ||
                            (pfVar13[0x12] <= fStack_4)) &&
                           (((fVar6 = fStack_4 - _DAT_004d8fac, fVar6 <= *pfVar13 ||
                             (fVar6 <= pfVar13[9])) || (fVar6 <= pfVar13[0x12])))))) &&
                         (((fVar2 <= pfVar13[-1] || (fVar2 <= pfVar13[8])) ||
                          (fVar2 <= pfVar13[0x11])))) &&
                        ((((pfVar13[-1] <= (float)local_ec || (pfVar13[8] <= (float)local_ec)) ||
                          (pfVar13[0x11] <= (float)local_ec)) &&
                         (((fVar4 <= pfVar13[1] || (fVar4 <= pfVar13[10])) || (fVar4 <= fVar3))))))
                       && (((pfVar13[1] <= fVar5 || (pfVar13[10] <= fVar5)) || (fVar3 <= fVar5)))) {
                      pfVar13[2] = pfVar10[3];
                      pfVar13[3] = pfVar10[4];
                      pfVar13[4] = pfVar10[5];
                      uVar21 = __ftol2();
                      pfVar13[5] = (float)((int)uVar21 << 0x18 | 0xffffff);
                      pfVar13[6] = (pfVar13[-1] - fVar2) / (float)(param_1 * 2);
                      pfVar13[7] = (pfVar13[1] - fVar4) / (float)(param_1 * 2);
                      pfVar13[0xb] = pfVar15[3];
                      pfVar13[0xc] = pfVar15[4];
                      pfVar13[0xd] = pfVar15[5];
                      uVar21 = __ftol2();
                      pfVar13[0xe] = (float)((int)uVar21 << 0x18 | 0xffffff);
                      pfVar13[0xf] = (float)(((float10)pfVar13[8] - (float10)fVar2) / extraout_ST0);
                      pfVar13[0x10] =
                           (float)(((float10)pfVar13[10] - (float10)fVar4) / extraout_ST0);
                      pfVar13[0x14] = pfVar19[3];
                      pfVar13[0x15] = pfVar19[4];
                      pfVar13[0x16] = pfVar19[5];
                      uVar21 = __ftol2();
                      pfVar13[0x17] = (float)((int)uVar21 << 0x18 | 0xffffff);
                      DAT_005341c8 = DAT_005341c8 + 3;
                      pfVar17 = pfVar13 + 0x1b;
                      pfVar13[0x18] =
                           (float)(((float10)pfVar13[0x11] - (float10)fVar2) / extraout_ST0_00);
                      pfVar13[0x19] =
                           (float)(((float10)pfVar13[0x13] - (float10)fVar4) / extraout_ST0_00);
                    }
                    iStack_e4 = iStack_e4 + 1;
                    pfVar13 = pfVar17;
                    pfVar10 = pfVar10 + 8;
                  } while (iStack_e4 < *(int *)(iVar11 + 4));
                }
                iVar9 = *(int *)(iVar12 + 0x42c + iVar8 * 4);
                if (*(int *)(iVar12 + 0x428) <= iVar9) break;
                iVar11 = *(int *)(*(int *)(iVar12 + 0x830) + iVar9 * 4);
                *(int *)(iVar12 + 0x42c + iVar8 * 4) = iVar9 + 1;
                this = pvStack_f8;
              }
            }
            iVar8 = *(int *)((int)pvStack_f8 + 8);
            iVar11 = *(int *)(iVar8 + 0x34 + iVar7 * 4);
            if (*(int *)(iVar8 + 0x30) <= iVar11) break;
            iVar12 = *(int *)(*(int *)(iVar8 + 0x438) + iVar11 * 4);
            *(int *)(iVar8 + 0x34 + iVar7 * 4) = iVar11 + 1;
            this = pvStack_f8;
          }
        }
        else {
          iVar7 = AthenaList_NextIndex(iVar8);
          iVar8 = *(int *)((int)this + 8);
          *(undefined4 *)(iVar8 + 0x34 + iVar7 * 4) = 0;
          if (*(int *)(iVar8 + 0x30) < 1) {
            iVar12 = 0;
          }
          else {
            iVar12 = **(int **)(iVar8 + 0x438);
            *(undefined4 *)(iVar8 + 0x34 + iVar7 * 4) = 1;
          }
          while (iVar12 != 0) {
            if (*(char *)(iVar12 + 0x863) == '\0') {
              if (*(char *)(iVar12 + 0x85c) == '\0') {
                iVar8 = AthenaList_NextIndex(iVar12 + 0xc);
                *(undefined4 *)(iVar12 + 0x14 + iVar8 * 4) = 0;
                if (*(int *)(iVar12 + 0x10) < 1) {
                  pfVar10 = (float *)0x0;
                }
                else {
                  pfVar10 = (float *)**(undefined4 **)(iVar12 + 0x418);
                  *(undefined4 *)(iVar12 + 0x14 + iVar8 * 4) = 1;
                }
                if (pfVar10 != (float *)0x0) {
                  pfVar13 = (float *)(&DAT_004fe1cc + DAT_005341c8 * 9);
                  do {
                    pfVar13[-1] = *pfVar10;
                    *pfVar13 = pfVar10[1];
                    pfVar13[1] = pfVar10[2];
                    pfVar13[8] = pfVar10[8];
                    pfVar13[9] = pfVar10[9];
                    pfVar13[10] = pfVar10[10];
                    pfVar13[0x11] = pfVar10[0x10];
                    pfVar13[0x12] = pfVar10[0x11];
                    fVar3 = pfVar10[0x12];
                    pfVar13[0x13] = fVar3;
                    pfVar14 = pfVar13;
                    if (((((pfVar10[4] < _DAT_004cf368 == (pfVar10[4] == _DAT_004cf368)) ||
                          (pfVar10[0xc] < _DAT_004cf368 == (pfVar10[0xc] == _DAT_004cf368))) ||
                         (pfVar10[0x14] < _DAT_004cf368 == (pfVar10[0x14] == _DAT_004cf368))) &&
                        ((((*pfVar13 <= fStack_4 || (pfVar13[9] <= fStack_4)) ||
                          (pfVar13[0x12] <= fStack_4)) &&
                         ((((fVar2 <= pfVar13[-1] || (fVar2 <= pfVar13[8])) ||
                           (fVar2 <= pfVar13[0x11])) &&
                          (((pfVar13[-1] <= (float)local_ec || (pfVar13[8] <= (float)local_ec)) ||
                           (pfVar13[0x11] <= (float)local_ec)))))))) &&
                       ((((fVar4 <= pfVar13[1] || (fVar4 <= pfVar13[10])) || (fVar4 <= fVar3)) &&
                        (((pfVar13[1] <= fVar5 || (pfVar13[10] <= fVar5)) || (fVar3 <= fVar5)))))) {
                      pfVar13[2] = pfVar10[3];
                      pfVar13[3] = pfVar10[4];
                      pfVar13[4] = pfVar10[5];
                      uVar21 = __ftol2();
                      pfVar13[5] = (float)((int)uVar21 << 0x18 | 0xffffff);
                      pfVar13[6] = (pfVar13[-1] - fVar2) / (float)(param_1 * 2);
                      pfVar13[7] = (pfVar13[1] - fVar4) / (float)(param_1 * 2);
                      pfVar13[0xb] = pfVar10[0xb];
                      pfVar13[0xc] = pfVar10[0xc];
                      pfVar13[0xd] = pfVar10[0xd];
                      uVar21 = __ftol2();
                      pfVar13[0xe] = (float)((int)uVar21 << 0x18 | 0xffffff);
                      pfVar13[0xf] = (float)(((float10)pfVar13[8] - (float10)fVar2) /
                                            extraout_ST0_01);
                      pfVar13[0x10] =
                           (float)(((float10)pfVar13[10] - (float10)fVar4) / extraout_ST0_01);
                      pfVar13[0x14] = pfVar10[0x13];
                      pfVar13[0x15] = pfVar10[0x14];
                      pfVar13[0x16] = pfVar10[0x15];
                      uVar21 = __ftol2();
                      pfVar13[0x17] = (float)((int)uVar21 << 0x18 | 0xffffff);
                      DAT_005341c8 = DAT_005341c8 + 3;
                      pfVar14 = pfVar13 + 0x1b;
                      pfVar13[0x18] =
                           (float)(((float10)pfVar13[0x11] - (float10)fVar2) / extraout_ST0_02);
                      pfVar13[0x19] =
                           (float)(((float10)pfVar13[0x13] - (float10)fVar4) / extraout_ST0_02);
                    }
                    iVar11 = *(int *)(iVar12 + 0x14 + iVar8 * 4);
                    if (*(int *)(iVar12 + 0x10) <= iVar11) break;
                    pfVar10 = *(float **)(*(int *)(iVar12 + 0x418) + iVar11 * 4);
                    *(int *)(iVar12 + 0x14 + iVar8 * 4) = iVar11 + 1;
                    pfVar13 = pfVar14;
                  } while (pfVar10 != (float *)0x0);
                }
              }
              else {
                iVar8 = AthenaList_NextIndex(iVar12 + 0x424);
                *(undefined4 *)(iVar12 + 0x42c + iVar8 * 4) = 0;
                if (*(int *)(iVar12 + 0x428) < 1) {
                  iVar11 = 0;
                }
                else {
                  iVar11 = **(int **)(iVar12 + 0x830);
                  *(undefined4 *)(iVar12 + 0x42c + iVar8 * 4) = 1;
                }
                while (iVar11 != 0) {
                  bVar20 = false;
                  fStack_f0 = 0.0;
                  if (0 < *(int *)(iVar11 + 4)) {
                    pfVar10 = *(float **)(iVar11 + 0x10) + 0x10;
                    pfVar13 = *(float **)(iVar11 + 0x10);
                    pfVar14 = (float *)(&DAT_004fe1cc + DAT_005341c8 * 9);
                    do {
                      if (bVar20) {
                        pfVar19 = pfVar10 + -8;
                        pfVar15 = pfVar10;
                      }
                      else {
                        pfVar15 = pfVar10 + -8;
                        pfVar19 = pfVar10;
                      }
                      bVar20 = bVar20 == false;
                      pfVar14[-1] = *pfVar13;
                      *pfVar14 = pfVar10[-0xf];
                      pfVar14[1] = pfVar10[-0xe];
                      pfVar14[8] = *pfVar15;
                      pfVar14[9] = pfVar15[1];
                      pfVar14[10] = pfVar15[2];
                      pfVar14[0x11] = *pfVar19;
                      pfVar14[0x12] = pfVar19[1];
                      fVar3 = pfVar19[2];
                      pfVar14[0x13] = fVar3;
                      pfVar10 = pfVar10 + 8;
                      pfVar17 = pfVar14;
                      if ((((((pfVar13[4] < _DAT_004cf368 == (pfVar13[4] == _DAT_004cf368)) ||
                             (pfVar15[4] < _DAT_004cf368 == (pfVar15[4] == _DAT_004cf368))) ||
                            (pfVar19[4] < _DAT_004cf368 == (pfVar19[4] == _DAT_004cf368))) &&
                           (((*pfVar14 <= fStack_4 || (pfVar14[9] <= fStack_4)) ||
                            (pfVar14[0x12] <= fStack_4)))) &&
                          ((((fVar6 = fStack_4 - _DAT_004d8fac, fVar6 <= *pfVar14 ||
                             (fVar6 <= pfVar14[9])) || (fVar6 <= pfVar14[0x12])) &&
                           (((fVar2 <= pfVar14[-1] || (fVar2 <= pfVar14[8])) ||
                            (fVar2 <= pfVar14[0x11])))))) &&
                         ((((pfVar14[-1] <= (float)local_ec || (pfVar14[8] <= (float)local_ec)) ||
                           (pfVar14[0x11] <= (float)local_ec)) &&
                          ((((fVar4 <= pfVar14[1] || (fVar4 <= pfVar14[10])) || (fVar4 <= fVar3)) &&
                           (((pfVar14[1] <= fVar5 || (pfVar14[10] <= fVar5)) || (fVar3 <= fVar5)))))
                          ))) {
                        pfVar14[2] = pfVar13[3];
                        pfVar14[3] = pfVar13[4];
                        pfVar14[4] = pfVar13[5];
                        uVar21 = __ftol2();
                        pfVar14[5] = (float)((int)uVar21 << 0x18 | 0xffffff);
                        pfVar14[6] = (pfVar14[-1] - fVar2) / (float)(param_1 * 2);
                        pfVar14[7] = (pfVar14[1] - fVar4) / (float)(param_1 * 2);
                        pfVar14[0xb] = pfVar15[3];
                        pfVar14[0xc] = pfVar15[4];
                        pfVar14[0xd] = pfVar15[5];
                        uVar21 = __ftol2();
                        pfVar14[0xe] = (float)((int)uVar21 << 0x18 | 0xffffff);
                        pfVar14[0xf] = (float)(((float10)pfVar14[8] - (float10)fVar2) /
                                              extraout_ST0_03);
                        pfVar14[0x10] =
                             (float)(((float10)pfVar14[10] - (float10)fVar4) / extraout_ST0_03);
                        pfVar14[0x14] = pfVar19[3];
                        pfVar14[0x15] = pfVar19[4];
                        pfVar14[0x16] = pfVar19[5];
                        uVar21 = __ftol2();
                        pfVar14[0x17] = (float)((int)uVar21 << 0x18 | 0xffffff);
                        DAT_005341c8 = DAT_005341c8 + 3;
                        pfVar17 = pfVar14 + 0x1b;
                        pfVar14[0x18] =
                             (float)(((float10)pfVar14[0x11] - (float10)fVar2) / extraout_ST0_04);
                        pfVar14[0x19] =
                             (float)(((float10)pfVar14[0x13] - (float10)fVar4) / extraout_ST0_04);
                      }
                      fStack_f0 = (float)((int)fStack_f0 + 1);
                      pfVar13 = pfVar13 + 8;
                      pfVar14 = pfVar17;
                    } while ((int)fStack_f0 < *(int *)(iVar11 + 4));
                  }
                  iVar9 = *(int *)(iVar12 + 0x42c + iVar8 * 4);
                  if (*(int *)(iVar12 + 0x428) <= iVar9) break;
                  iVar11 = *(int *)(*(int *)(iVar12 + 0x830) + iVar9 * 4);
                  *(int *)(iVar12 + 0x42c + iVar8 * 4) = iVar9 + 1;
                }
              }
            }
            iVar8 = *(int *)((int)pvStack_f8 + 8);
            iVar11 = *(int *)(iVar8 + 0x34 + iVar7 * 4);
            if (*(int *)(iVar8 + 0x30) <= iVar11) break;
            iVar12 = *(int *)(*(int *)(iVar8 + 0x438) + iVar11 * 4);
            *(int *)(iVar8 + 0x34 + iVar7 * 4) = iVar11 + 1;
          }
        }
      }
      if ((param_7 == '\x01') && (0 < DAT_005341c8)) {
        iVar8 = *(int *)((int)local_ec + 4);
        iVar7 = *(int *)(iVar8 + 0x704);
        cVar1 = *(char *)(iVar8 + 0x700);
        if (iVar7 != 2) {
          *(int *)(iVar8 + 0x7c8) = *(int *)(iVar8 + 0x7c8) + 1;
          (**(code **)(**(int **)(iVar8 + 0x154) + 0xfc))(*(int **)(iVar8 + 0x154),0,0xd,3);
          (**(code **)(**(int **)(iVar8 + 0x154) + 0xfc))(*(int **)(iVar8 + 0x154),0,0xe,3);
          *(undefined4 *)(iVar8 + 0x704) = 2;
        }
        Gfx_SetupAlphaRenderState(*(void **)((int)local_ec + 4));
        RenderContext_Init(auStack_5c);
        fStack_4 = 0.0;
        uStack_14 = param_5;
        Graphics_ApplyMaterialAndDraw(*(void **)((int)local_ec + 4),auStack_5c);
        piVar18 = *(int **)(*(int *)((int)local_ec + 4) + 0x154);
        (**(code **)(*piVar18 + 200))(piVar18,0x8d,1);
        piVar18 = *(int **)(*(int *)((int)local_ec + 4) + 0x154);
        (**(code **)(*piVar18 + 200))(piVar18,0x91,1);
        piVar18 = *(int **)(*(int *)((int)local_ec + 4) + 0x154);
        (**(code **)(*piVar18 + 200))(piVar18,0x93,1);
        piVar18 = *(int **)(*(int *)((int)local_ec + 4) + 0x154);
        (**(code **)(*piVar18 + 0xfc))(piVar18,0,4,4);
        piVar18 = *(int **)(*(int *)((int)local_ec + 4) + 0x154);
        (**(code **)(*piVar18 + 0xfc))(piVar18,0,5,0x22);
        piVar18 = *(int **)(*(int *)((int)local_ec + 4) + 0x154);
        (**(code **)(*piVar18 + 0xfc))(piVar18,0,6,0);
        iVar8 = *(int *)((int)local_ec + 4);
        if (*(char *)(iVar8 + 0x70c) != '\0') {
          (**(code **)(**(int **)(iVar8 + 0x154) + 200))(*(int **)(iVar8 + 0x154),0xe,0);
          *(undefined1 *)(iVar8 + 0x70c) = 0;
          *(int *)(iVar8 + 0x7c8) = *(int *)(iVar8 + 0x7c8) + 1;
        }
        Graphics_BeginFrame(*(void **)((int)local_ec + 4),*(int *)((int)local_ec + 0x434));
        piVar18 = *(int **)(*(int *)((int)local_ec + 4) + 0x154);
        (**(code **)(*piVar18 + 0x120))(piVar18,4,DAT_005341c8 / 3,&DAT_004fe1c8,0x24);
        iVar8 = *(int *)((int)local_ec + 4);
        if (*(char *)(iVar8 + 0x70c) != '\x01') {
          (**(code **)(**(int **)(iVar8 + 0x154) + 200))(*(int **)(iVar8 + 0x154),0xe,1);
          *(undefined1 *)(iVar8 + 0x70c) = 1;
          *(int *)(iVar8 + 0x7c8) = *(int *)(iVar8 + 0x7c8) + 1;
        }
        piVar18 = *(int **)(*(int *)((int)local_ec + 4) + 0x154);
        (**(code **)(*piVar18 + 200))(piVar18,0x8d,0);
        piVar18 = *(int **)(*(int *)((int)local_ec + 4) + 0x154);
        (**(code **)(*piVar18 + 200))(piVar18,0x91,0);
        piVar18 = *(int **)(*(int *)((int)local_ec + 4) + 0x154);
        (**(code **)(*piVar18 + 200))(piVar18,0x93,0);
        piVar18 = *(int **)(*(int *)((int)local_ec + 4) + 0x154);
        (**(code **)(*piVar18 + 0xfc))(piVar18,0,4,2);
        iVar8 = *(int *)((int)local_ec + 4);
        if (*(char *)(iVar8 + 0x700) != cVar1) {
          piVar18 = *(int **)(iVar8 + 0x154);
          *(char *)(iVar8 + 0x700) = cVar1;
          *(int *)(iVar8 + 0x7c8) = *(int *)(iVar8 + 0x7c8) + 1;
          if (cVar1 == '\x01') {
            (**(code **)(*piVar18 + 200))(piVar18,0x1b,1);
            (**(code **)(**(int **)(iVar8 + 0x154) + 0xfc))(*(int **)(iVar8 + 0x154),0,4,4);
          }
          else {
            (**(code **)(*piVar18 + 200))(piVar18,0x1b,0);
            (**(code **)(**(int **)(iVar8 + 0x154) + 0xfc))(*(int **)(iVar8 + 0x154),0,4,2);
          }
        }
        iVar8 = *(int *)((int)local_ec + 4);
        if (*(int *)(iVar8 + 0x704) != iVar7) {
          if (iVar7 == 0) {
            App_CompleteRace(iVar8);
          }
          else if (iVar7 == 1) {
            Gfx_SetBlendState(iVar8);
          }
          else if (iVar7 == 2) {
            Gfx_SetAlphaBlendState(iVar8);
          }
        }
        fStack_4 = -NAN;
        Matrix4_Identity(auStack_5c);
      }
    }
  }
  ExceptionList = local_c;
  return;
}

