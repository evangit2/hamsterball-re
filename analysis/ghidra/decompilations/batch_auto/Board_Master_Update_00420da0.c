/*
 * Function: Board_Master_Update
 * Address: 0x00420da0
 * Signature: Board_Master_Update(...)
 *
 * Patterns: allocates, vtable dispatch, SEH frame, audio, rendering, input, ball, scene, board. Calls: Board_Master_Update, Scene_Update, RNG_Rand, operator_new, RegisterDialog_Render, AthenaList_Append, AthenaList_NextIndex, SQRT. Offsets: 26, Lines: 224
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall Board_Master_Update(int *param_1)

{
  float fVar1;
  float fVar2;
  float fVar3;
  int iVar4;
  void *pvVar5;
  int iVar6;
  float *pfVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  undefined1 uVar12;
  int iVar13;
  int iVar14;
  float fStack_60;
  float fStack_5c;
  float fStack_58;
  float fStack_54;
  float fStack_4c;
  float fStack_48;
  float fStack_44;
  float fStack_40;
  float fStack_38;
  float fStack_34;
  float fStack_30;
  float fStack_1c;
  float afStack_18 [3];
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_004ca836;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  Scene_Update(param_1);
  (**(code **)(*param_1 + 0x90))();
  (**(code **)(*param_1 + 0x94))();
  iVar14 = 0;
  iVar4 = RNG_Rand(&PTR_OBJ_VTABLE,0xb,'\0');
  if (iVar4 == 10) {
    pvVar5 = operator_new(0x1c);
    uStack_4 = 0;
    if (pvVar5 == (void *)0x0) {
      pvVar5 = (void *)0x0;
    }
    else {
      pvVar5 = RegisterDialog_Render(pvVar5,param_1[0x21e],(int)(param_1 + 0x1719));
    }
    uStack_4 = 0xffffffff;
    AthenaList_Append(param_1 + 0xec0,(int)pvVar5);
  }
  iVar4 = AthenaList_NextIndex((int)(param_1 + 0x1820));
  param_1[iVar4 + 0x1822] = 0;
  if (param_1[0x1821] < 1) {
    iVar11 = 0;
  }
  else {
    iVar11 = *(int *)param_1[0x1923];
    param_1[iVar4 + 0x1822] = 1;
  }
  while (iVar11 != 0) {
    *(undefined1 *)(iVar11 + 0x1104) = 0;
    iVar6 = param_1[iVar4 + 0x1822];
    if (param_1[0x1821] <= iVar6) break;
    iVar11 = *(int *)(param_1[0x1923] + iVar6 * 4);
    param_1[iVar4 + 0x1822] = iVar6 + 1;
  }
  iVar4 = AthenaList_NextIndex((int)(param_1 + 0xa75));
  param_1[iVar4 + 0xa77] = 0;
  if (0 < param_1[0xa76]) {
    iVar14 = *(int *)param_1[0xb78];
    param_1[iVar4 + 0xa77] = 1;
  }
  do {
    if (iVar14 == 0) {
      ExceptionList = pvStack_c;
      return;
    }
    if (*(char *)(iVar14 + 0x2cc) == '\0') {
      uVar12 = 0;
      iVar11 = AthenaList_NextIndex((int)(param_1 + 0x1820));
      param_1[iVar11 + 0x1822] = 0;
      if (param_1[0x1821] < 1) {
        iVar6 = 0;
      }
      else {
        iVar6 = *(int *)param_1[0x1923];
        param_1[iVar11 + 0x1822] = 1;
      }
      while (iVar6 != 0) {
        if (-1 < *(int *)(iVar14 + 0x18)) {
          fStack_38 = *(float *)(iVar14 + 0x168);
          fStack_34 = *(float *)(iVar14 + 0x16c);
          fStack_1c = *(float *)(iVar6 + 0x10e8);
          fStack_30 = *(float *)(iVar6 + 0x10e0) - *(float *)(iVar14 + 0x164);
          fVar1 = *(float *)(iVar6 + 0x10e4) - fStack_38;
          if (SQRT(fStack_30 * fStack_30 +
                   fVar1 * fVar1 + (fStack_1c - fStack_34) * (fStack_1c - fStack_34)) <
              *(float *)(iVar6 + 0x1100) * _DAT_004d0930) {
            iVar13 = *(int *)(iVar14 + 0x1a4);
            pfVar7 = (float *)(iVar13 + 0xca4);
            if (&fStack_60 != pfVar7) {
              fStack_60 = *pfVar7;
              fStack_5c = *(float *)(iVar13 + 0xca8);
              fStack_58 = *(float *)(iVar13 + 0xcac);
            }
            fVar1 = fStack_58 * fStack_58 + fStack_60 * fStack_60 + fStack_5c * fStack_5c;
            fVar2 = _DAT_004cf368;
            if (fVar1 < _DAT_004cf368 == (fVar1 == _DAT_004cf368)) {
              fVar2 = SQRT(fVar1);
            }
            fVar3 = _DAT_004cf368;
            if ((fVar1 < _DAT_004cf368 == (fVar1 == _DAT_004cf368)) &&
               (fVar3 = SQRT(fVar1), _DAT_004cf368 < fVar3)) {
              fVar3 = (fVar2 * _DAT_004d092c) / fVar3;
            }
            fStack_60 = fStack_60 * fVar3;
            fStack_5c = fStack_5c * fVar3;
            fStack_58 = fStack_58 * fVar3;
            if (pfVar7 != &fStack_60) {
              *pfVar7 = fStack_60;
              *(float *)(iVar13 + 0xca8) = fStack_5c;
              *(float *)(iVar13 + 0xcac) = fStack_58;
            }
            if (*(char *)(iVar14 + 700) == '\0') {
              fStack_44 = *(float *)(iVar14 + 0x168);
              fStack_40 = *(float *)(iVar14 + 0x16c);
              fStack_48 = *(float *)(iVar14 + 0x164);
              Sound_Play3D(*(void **)(param_1[0x21e] + 0x484),fStack_48,fStack_44,fStack_40);
              iVar13 = 3;
              do {
                pfVar7 = operator_new(0x14);
                if (pfVar7 == (float *)0x0) {
                  pfVar7 = (float *)0x0;
                }
                else {
                  pfVar7[3] = 8.40779e-45;
                  *(undefined1 *)(pfVar7 + 4) = 0;
                }
                iVar8 = RNG_Rand(&PTR_OBJ_VTABLE,100,'\x01');
                iVar9 = RNG_Rand(&PTR_OBJ_VTABLE,100,'\x01');
                iVar10 = RNG_Rand(&PTR_OBJ_VTABLE,100,'\x01');
                if (pfVar7 != afStack_18) {
                  *pfVar7 = (float)iVar10;
                  pfVar7[1] = (float)iVar9;
                  pfVar7[2] = (float)iVar8;
                }
                fVar1 = pfVar7[2] * pfVar7[2] + pfVar7[1] * pfVar7[1] + *pfVar7 * *pfVar7;
                fVar2 = _DAT_004cf368;
                if ((fVar1 < _DAT_004cf368 == (fVar1 == _DAT_004cf368)) &&
                   (fVar2 = SQRT(fVar1), _DAT_004cf368 < fVar2)) {
                  fVar2 = _DAT_004cf310 / fVar2;
                }
                *pfVar7 = fVar2 * *pfVar7;
                pfVar7[1] = fVar2 * pfVar7[1];
                pfVar7[2] = fVar2 * pfVar7[2];
                iVar8 = AthenaList_GetSize(iVar14 + 0x810);
                if (iVar8 < 0x1e) {
                  AthenaList_Append((void *)(iVar14 + 0x810),(int)pfVar7);
                }
                iVar13 = iVar13 + -1;
              } while (iVar13 != 0);
            }
            uVar12 = 1;
            *(undefined1 *)(iVar6 + 0x1104) = 1;
          }
        }
        iVar13 = param_1[iVar11 + 0x1822];
        if (param_1[0x1821] <= iVar13) break;
        iVar6 = *(int *)(param_1[0x1923] + iVar13 * 4);
        param_1[iVar11 + 0x1822] = iVar13 + 1;
      }
      *(undefined1 *)(iVar14 + 700) = uVar12;
    }
    else {
      *(float *)(iVar14 + 0x168) = *(float *)(iVar14 + 0x168) - _DAT_004cf380;
      iVar11 = RNG_Rand(&PTR_OBJ_VTABLE,0xf,'\0');
      if (iVar11 == 1) {
        iVar11 = RNG_Rand(&PTR_OBJ_VTABLE,100,'\x01');
        fVar1 = (float)iVar11;
        iVar11 = RNG_Rand(&PTR_OBJ_VTABLE,100,'\x01');
        fStack_54 = (float)iVar11;
        iVar11 = RNG_Rand(&PTR_OBJ_VTABLE,10,'\x01');
        fVar2 = fStack_54 * fStack_54 + fVar1 * fVar1;
        fStack_4c = _DAT_004cf368;
        if ((fVar2 < _DAT_004cf368 == (fVar2 == _DAT_004cf368)) &&
           (fStack_4c = SQRT(fVar2), _DAT_004cf368 < fStack_4c)) {
          fStack_4c = ((float)iVar11 + *(float *)(iVar14 + 0x284)) / fStack_4c;
        }
        fStack_54 = fStack_54 * fStack_4c;
        fStack_4c = fStack_4c * fVar1;
        pvVar5 = operator_new(0x1c);
        uStack_4 = 1;
        if (pvVar5 == (void *)0x0) {
          pvVar5 = (void *)0x0;
        }
        else {
          pvVar5 = RegisterDialog_OnKey
                             (pvVar5,param_1[0x21e],fStack_54 + *(float *)(iVar14 + 0x164),
                              *(undefined4 *)(iVar14 + 0x2d0),fStack_4c + *(float *)(iVar14 + 0x16c)
                             );
        }
        uStack_4 = 0xffffffff;
        AthenaList_Append(param_1 + 0xec0,(int)pvVar5);
      }
      if (*(float *)(iVar14 + 0x168) <
          *(float *)(iVar14 + 0x2d0) - *(float *)(iVar14 + 0x284) * (float)_DAT_004cf378) {
        Ball_FindClosestRespawnPoint(iVar14);
      }
    }
    iVar11 = param_1[iVar4 + 0xa77];
    if (param_1[0xa76] <= iVar11) {
      ExceptionList = pvStack_c;
      return;
    }
    iVar14 = *(int *)(param_1[0xb78] + iVar11 * 4);
    param_1[iVar4 + 0xa77] = iVar11 + 1;
  } while( true );
}
