/*
 * Function: Ball_Update
 * Address: 0x00405e00
 * Signature: Ball_Update(...)
 *
 * Patterns: frees memory, allocates, vtable dispatch, SEH frame, audio, collision, rendering, camera, ball, scene, board. Calls: collisions, friction, Ball_Update, Sound_Play3DAtPosition, operator_new, RumbleScore_ctor, RNG_Rand, AthenaList_Append. Offsets: 65, Lines: 1078
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* Ball_Update: 23-phase physics tick. Decay timers, ambient sound, trail particles, save prev_pos,
   build collision tree+node, gravity reflect, iterate collisions (type1=ball-ball,2=wall,5=floor),
   apply external vel, facing atan2, spin friction (3 iter), roll physics, lerp display_pos,
   teleport override. See decompilations/ball/decomp_ball_physics_update.c */

void __fastcall Ball_Update(int *param_1)

{
  char cVar1;
  int *piVar2;
  int iVar3;
  bool bVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  void *pvVar8;
  int iVar9;
  int iVar10;
  uint *puVar11;
  float *pfVar12;
  char *pcVar13;
  char *pcVar14;
  int iVar15;
  float unaff_EBX;
  undefined4 *unaff_EBP;
  int *piVar16;
  float unaff_ESI;
  undefined1 *unaff_EDI;
  float10 fVar17;
  undefined1 auVar18 [10];
  ulonglong uVar19;
  ulonglong uVar20;
  void *pvStack_978;
  float fStack_974;
  float fStack_970;
  float fVar21;
  float fVar22;
  undefined4 *puVar23;
  void *local_938;
  float local_934;
  void *local_92c;
  float local_928;
  float local_924;
  float local_920;
  float local_91c;
  float local_918;
  undefined4 uStack_914;
  float fStack_910;
  float fStack_90c;
  float fStack_908;
  float fStack_904;
  float fStack_900;
  float fStack_8fc;
  float fStack_8f8;
  float fStack_8f4;
  float fStack_8f0;
  int iStack_8ec;
  float fStack_8e8;
  float fStack_8e4;
  float local_8e0 [3];
  float fStack_8d4;
  float fStack_8d0;
  float fStack_8cc;
  float fStack_8c8;
  void *pvStack_8c4;
  float fStack_8c0;
  float fStack_8bc;
  void *pvStack_8b8;
  float fStack_8b4;
  float fStack_8b0;
  float fStack_8a8;
  float fStack_8a4;
  float fStack_8a0;
  undefined4 auStack_88c [2];
  float fStack_884;
  undefined4 local_880;
  undefined1 local_87c [8];
  float fStack_874;
  float fStack_864;
  float fStack_854;
  undefined1 auStack_848 [12];
  undefined4 local_83c;
  int iStack_838;
  int aiStack_834 [257];
  undefined4 *puStack_430;
  undefined4 auStack_424 [261];
  undefined1 uStack_10;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c9419;
  local_c = ExceptionList;
  fVar21 = (float)param_1[0x314] * _DAT_004cf540;
  ExceptionList = &local_c;
  param_1[0x314] = (int)fVar21;
  fVar22 = (float)_DAT_004cf538;
  param_1[0x317] = param_1[0x317] - 1U & ((int)(param_1[0x317] - 1U) < 1) - 1;
  param_1[200] = param_1[200] - 1U & ((int)(param_1[200] - 1U) < 1) - 1;
  param_1[0xbc] = param_1[0xbc] - 1U & ((int)(param_1[0xbc] - 1U) < 1) - 1;
  param_1[0xbd] = param_1[0xbd] - 1U & ((int)(param_1[0xbd] - 1U) < 1) - 1;
  if (fVar21 < fVar22) {
    param_1[0x314] = 0;
  }
  if (param_1[0x315] != 0) {
    Sound_Play3DAtPosition(param_1[0x315]);
  }
  if (param_1[200] != 0) {
    pvVar8 = operator_new(0x28);
    local_4 = 0;
    if (pvVar8 == (void *)0x0) {
      pvVar8 = (void *)0x0;
    }
    else {
      pvVar8 = RumbleScore_ctor(pvVar8,*(undefined4 *)(param_1[5] + 0x878));
    }
    local_4 = 0xffffffff;
    iVar9 = RNG_Rand(&PTR_OBJ_VTABLE,0x19,'\x01');
    local_91c = (float)iVar9 + (float)param_1[0x5b];
    iVar9 = RNG_Rand(&PTR_OBJ_VTABLE,0x19,'\x01');
    fVar21 = (float)param_1[0x5a];
    iVar10 = RNG_Rand(&PTR_OBJ_VTABLE,0x19,'\x01');
    if ((float *)((int)pvVar8 + 8) != local_8e0) {
      *(float *)((int)pvVar8 + 8) = (float)iVar10 + (float)param_1[0x59];
      *(float *)((int)pvVar8 + 0xc) = (float)iVar9 + fVar21;
      *(float *)((int)pvVar8 + 0x10) = local_91c;
    }
    if ((float *)((int)pvVar8 + 0x14) != local_8e0) {
      *(float *)((int)pvVar8 + 0x14) = 0.0;
      *(undefined4 *)((int)pvVar8 + 0x18) = 0;
      *(undefined4 *)((int)pvVar8 + 0x1c) = 0;
    }
    AthenaList_Append((void *)(param_1[5] + 0x3b00),(int)pvVar8);
  }
  if (param_1[0x1d2] == 1) {
    fVar21 = (float)param_1[0x1d3] - _DAT_004cf310;
    if (fVar21 < _DAT_004cf530) {
      fVar21 = _DAT_004cf530;
    }
  }
  else {
    fVar21 = (float)param_1[0x1d3] + _DAT_004cf310;
    if (_DAT_004cf368 < fVar21) {
      fVar21 = _DAT_004cf368;
    }
  }
  param_1[0x1d3] = (int)fVar21;
  if (param_1[0x1d2] == 2) {
    fVar21 = (float)param_1[0x1d4] - _DAT_004cf310;
    if (fVar21 < _DAT_004cf530) {
      fVar21 = _DAT_004cf530;
    }
  }
  else {
    fVar21 = (float)param_1[0x1d4] + _DAT_004cf310;
    if (_DAT_004cf368 < fVar21) {
      fVar21 = _DAT_004cf368;
    }
  }
  param_1[0x1d4] = (int)fVar21;
  param_1[0x1d1] = param_1[0x1d1] - 1U & ((int)(param_1[0x1d1] - 1U) < 0) - 1;
  piVar16 = param_1 + 0xc0;
  *piVar16 = *piVar16 + -1;
  if (*piVar16 < 0) {
    param_1[0xc0] = 0;
  }
  iVar9 = param_1[0xa8];
  param_1[0xa8] = iVar9 + -1;
  if (iVar9 + -1 < 1) {
    param_1[0xa7] = 0x3f800000;
  }
  piVar16 = param_1 + 3;
  *piVar16 = *piVar16 + -1;
  if ((*piVar16 == 0) && ((void *)param_1[0x30a] != (void *)0x0)) {
    _free((void *)param_1[0x30a]);
    param_1[0x30a] = 0;
  }
  if ((char)param_1[0x53] == '\0') {
    if ((param_1[6] != -1) && ((char)param_1[0x1da] != '\0')) {
      if ((char)param_1[0xc4] != '\0') {
        if ((float *)(param_1 + 0xc1) != local_8e0) {
          param_1[0xc1] = param_1[0x59];
          param_1[0xc2] = param_1[0x5a];
          param_1[0xc3] = param_1[0x5b];
        }
        *(undefined1 *)(param_1 + 0xc4) = 0;
      }
      local_918 = (float)param_1[0x59];
      fVar21 = ((float)param_1[0xc1] - local_918) * ((float)param_1[0xc1] - local_918) +
               ((float)param_1[0xc3] - (float)param_1[0x5b]) *
               ((float)param_1[0xc3] - (float)param_1[0x5b]) +
               ((float)param_1[0xc2] - (float)param_1[0x5a]) *
               ((float)param_1[0xc2] - (float)param_1[0x5a]);
      if (((fVar21 < _DAT_004cf368 == (fVar21 == _DAT_004cf368)) && (_DAT_004cf3ec < SQRT(fVar21)))
         && (pvVar8 = (void *)param_1[4], *(char *)(param_1[6] * 0xa0 + 0x5d6 + (int)pvVar8) == '\0'
            )) {
        pfVar12 = (float *)(param_1[6] * 0xa0 + 0x5e4 + (int)pvVar8);
        fVar17 = Difficulty_GetTimeModifier(pvVar8,1.0);
        *pfVar12 = (float)(fVar17 + (float10)*pfVar12);
        *(undefined1 *)(param_1 + 0xc4) = 1;
      }
    }
    param_1[0x1de] = 0;
    param_1[0x1df] = 0;
    param_1[0x1e0] = 0;
    param_1[0x1e1] = 0;
    iVar9 = 0x10;
    puVar11 = (uint *)(param_1 + 0x1f2);
    do {
      iVar9 = iVar9 + -1;
      *puVar11 = *puVar11 - 1 & ((int)(*puVar11 - 1) < 0) - 1;
      puVar11 = puVar11 + 1;
    } while (iVar9 != 0);
    param_1[0x202] = param_1[0x202] - 1U & ((int)(param_1[0x202] - 1U) < 0) - 1;
    param_1[0x203] = param_1[0x203] - 1U & ((int)(param_1[0x203] - 1U) < 0) - 1;
    if (((char)param_1[0xc9] != '\0') &&
       (fVar21 = (float)param_1[0xbf] - _DAT_004cf430, param_1[0xbf] = (int)fVar21,
       fVar21 < _DAT_004cf368)) {
      param_1[0xbf] = 0;
      *(undefined1 *)(param_1 + 0xba) = 1;
    }
    iVar9 = param_1[0xb6];
    param_1[0xb6] = iVar9 + -1;
    if (iVar9 + -1 < 1) {
      *(undefined1 *)((int)param_1 + 0x2d5) = 0;
      param_1[0xb6] = 0;
    }
    piVar16 = param_1 + 0x204;
    iVar9 = AthenaList_GetSize((int)piVar16);
    if (iVar9 != 0) {
      AthenaList_Init(&local_83c,0);
      local_4 = 1;
      local_92c = (void *)0x0;
      iVar9 = AthenaList_NextIndex((int)piVar16);
      pfVar12 = (float *)0x0;
      param_1[iVar9 + 0x206] = 0;
      if (0 < param_1[0x205]) {
        pfVar12 = *(float **)param_1[0x307];
        param_1[iVar9 + 0x206] = 1;
      }
      while (pfVar12 != (float *)0x0) {
        Timer_Init(&local_880);
        local_4 = CONCAT31(local_4._1_3_,2);
        Gfx_RotateY(*pfVar12,pfVar12[1],pfVar12[2]);
        unaff_EDI = local_87c;
        Graphics_SetRenderState();
        if ((float)param_1[0x5a] - (float)param_1[0xa1] * _DAT_004cf3f0 <=
            fStack_864 * -(float)param_1[0xa1] + (fStack_874 + fStack_884) * _DAT_004cf368 +
            fStack_854) {
          *(undefined1 *)(pfVar12 + 4) = 0;
        }
        else {
          if ((*(char *)(pfVar12 + 4) == '\0') &&
             (fVar21 = pfVar12[3], pfVar12[3] = (float)((int)fVar21 + -1), (int)fVar21 + -1 < 1)) {
            thunk_Gfx_SetRenderState(piVar16,(int)pfVar12);
            AthenaList_Append(auStack_848,(int)pfVar12);
          }
          *(undefined1 *)(pfVar12 + 4) = 1;
        }
        uStack_10 = 1;
        Timer_Cleanup(auStack_88c);
        iVar10 = param_1[iVar9 + 0x206];
        if (param_1[0x205] <= iVar10) break;
        pfVar12 = *(float **)(param_1[0x307] + iVar10 * 4);
        param_1[iVar9 + 0x206] = iVar10 + 1;
      }
      iVar9 = param_1[0x69];
      if (&local_928 != (float *)(iVar9 + 0xca4)) {
        local_928 = *(float *)(iVar9 + 0xca4);
        local_924 = *(float *)(iVar9 + 0xca8);
        local_920 = *(float *)(iVar9 + 0xcac);
      }
      pvVar8 = local_92c;
      if (0 < (int)local_92c) {
        do {
          pvVar8 = (void *)((int)pvVar8 + -1);
          local_928 = local_928 * _DAT_004cf52c;
          local_920 = local_920 * _DAT_004cf52c;
        } while (pvVar8 != (void *)0x0);
      }
      if ((float *)(iVar9 + 0xca4) != &local_928) {
        *(float *)(iVar9 + 0xca4) = local_928;
        *(float *)(iVar9 + 0xca8) = local_924;
        *(float *)(iVar9 + 0xcac) = local_920;
      }
      if (0 < (int)local_92c) {
        iVar9 = 0x40646b;
        iVar10 = RNG_Rand(&PTR_OBJ_VTABLE,5,'\0');
        if (iVar10 == 0) {
          fVar21 = local_928 * local_928 + local_920 * local_920 + local_924 * local_924;
          fVar22 = _DAT_004cf368;
          if (fVar21 < _DAT_004cf368 == (fVar21 == _DAT_004cf368)) {
            fVar22 = SQRT(fVar21);
          }
          if ((float)_DAT_004cf440 < fVar22) {
            if ((float *)&stack0xfffff6a8 != local_8e0) {
              iVar9 = -0x40800000;
            }
            Mesh_FindClosestCollision(*(void **)(param_1[5] + 0x8b0),&iStack_8ec);
            if (ABS(fStack_8e8 - (float)param_1[0x5a]) <= (float)param_1[0xa1] + _DAT_004cf48c) {
              fStack_8e4 = (float)param_1[0x5b];
              iStack_8ec = param_1[0x59];
              fVar22 = 4.48416e-44;
              fStack_8e8 = (float)param_1[0x5a] - ((float)param_1[0xa1] + _DAT_004cf528);
              fVar21 = 5.913875e-39;
              pvVar8 = operator_new(0x20);
              local_4._0_1_ = 3;
              if (pvVar8 == (void *)0x0) {
                pvVar8 = (void *)0x0;
              }
              else {
                if ((int *)&stack0xfffff6ac != &iStack_8ec) {
                  iVar9 = iStack_8ec;
                  fVar21 = fStack_8e8;
                  fVar22 = fStack_8e4;
                }
                pvVar8 = RegisterDialog_Tick(pvVar8,*(undefined4 *)(param_1[5] + 0x878),iVar9,fVar21
                                             ,fVar22);
              }
              local_4 = CONCAT31(local_4._1_3_,1);
              AthenaList_Append((void *)(param_1[5] + 0x3b00),(int)pvVar8);
            }
          }
        }
      }
      iVar9 = AthenaList_NextIndex((int)&local_83c);
      piVar16 = aiStack_834 + iVar9;
      *piVar16 = 0;
      if (iStack_838 < 1) {
        pvVar8 = (void *)0x0;
      }
      else {
        pvVar8 = (void *)*puStack_430;
        *piVar16 = 1;
      }
      while (pvVar8 != (void *)0x0) {
        _free(pvVar8);
        iVar9 = *piVar16;
        if (iStack_838 <= iVar9) break;
        pvVar8 = (void *)puStack_430[iVar9];
        *piVar16 = iVar9 + 1;
      }
      local_4 = 0xffffffff;
      Vec3List_Free(&local_83c);
    }
    if ((*(char *)((int)param_1 + 0x2f9) != '\0') &&
       (fVar21 = (float)param_1[0xbf] + _DAT_004cf524, param_1[0xbf] = (int)fVar21,
       _DAT_004cf310 < fVar21)) {
      param_1[0xbf] = 0x3f800000;
      *(undefined1 *)((int)param_1 + 0x2f9) = 0;
    }
    param_1[0x58] = param_1[0x5b];
    fStack_8b0 = (float)param_1[0x5c];
    param_1[0x56] = param_1[0x59];
    fStack_8f0 = (float)param_1[0x5e];
    param_1[0x57] = param_1[0x5a];
    local_92c = (void *)param_1[0x5d];
    param_1[0x5e] = 0;
    param_1[0x5d] = 0;
    param_1[0x5c] = 0;
    AthenaList_Init(auStack_424,0);
    local_4 = 4;
    AthenaList_Append(auStack_424,*(int *)(param_1[5] + 0x8b0));
    *(int *)(param_1[0x69] + 0xc70) = param_1[0x62];
    piVar16 = (int *)param_1[0x69];
    if (&iStack_8ec != piVar16 + 0x329) {
      iStack_8ec = piVar16[0x329];
      fStack_8e8 = (float)piVar16[0x32a];
      fStack_8e4 = (float)piVar16[0x32b];
    }
    pvStack_8c4 = local_92c;
    fStack_8c8 = fStack_8b0;
    fStack_8c0 = fStack_8f0;
    fVar21 = -(fStack_8b0 * (float)param_1[0x6a] +
              fStack_8f0 * (float)param_1[0x6c] + (float)local_92c * (float)param_1[0x6b]);
    local_918 = fVar21 * (float)param_1[0x6a];
    fVar22 = fVar21 * (float)param_1[0x6c] + fStack_8f0;
    pvVar8 = (void *)(fVar21 * (float)param_1[0x6b] + (float)local_92c);
    fVar7 = fStack_8b0 + local_918;
    fVar21 = fVar22 * fVar22 + (float)pvVar8 * (float)pvVar8 + fVar7 * fVar7;
    fVar5 = _DAT_004cf368;
    if (fVar21 < _DAT_004cf368 == (fVar21 == _DAT_004cf368)) {
      fVar5 = SQRT(fVar21);
    }
    fVar6 = fStack_8f0 * fStack_8f0 + (float)local_92c * (float)local_92c + fStack_8b0 * fStack_8b0;
    fVar21 = _DAT_004cf368;
    if (fVar6 < _DAT_004cf368 == (fVar6 == _DAT_004cf368)) {
      fVar21 = SQRT(fVar6);
    }
    if (fVar5 == fVar21) {
      fStack_8c8 = fVar7;
      pvStack_8c4 = pvVar8;
      fStack_8c0 = fVar22;
    }
    fStack_8bc = fStack_8c8;
    pvStack_8b8 = pvStack_8c4;
    fStack_8b4 = fStack_8c0;
    (**(code **)(*piVar16 + 0x18))();
    pvVar8 = operator_new(0x20);
    local_4._0_1_ = 5;
    if (pvVar8 == (void *)0x0) {
      local_92c = (void *)0x0;
    }
    else {
      local_92c = SpatialTree_ctor(pvVar8,auStack_424);
    }
    *(undefined4 *)((int)local_92c + 0x10) = 10;
    *(int *)((int)local_92c + 0xc) = param_1[0x9e];
    local_4._0_1_ = 4;
    if (param_1[0x203] < 1) {
      (**(code **)(*(int *)param_1[0x69] + 0x14))();
    }
    local_938 = operator_new(0x14);
    local_4._0_1_ = 6;
    if (local_938 == (void *)0x0) {
      pvVar8 = (void *)0x0;
    }
    else {
      pvVar8 = CollisionNode_ctor(local_938,param_1[5] + 0x29d4);
    }
    local_4 = CONCAT31(local_4._1_3_,4);
    if ((char)param_1[0xc9] == '\0') {
      *(int *)((int)pvVar8 + 0xc) = param_1[0x9f];
      (**(code **)(*(int *)param_1[0x69] + 0x14))();
    }
    *(int *)(param_1[0x69] + 0xc78) = param_1[0xa1];
    local_91c = (float)param_1[0x5b];
    fStack_910 = (float)param_1[0xa1];
    local_934 = (float)param_1[0x5a];
    uStack_914 = (float)param_1[0xa1];
    local_928 = (float)param_1[0x59];
    local_918 = (float)param_1[0xa1];
    fStack_8cc = (float)param_1[0x5b];
    fStack_8d0 = (float)param_1[0x5a];
    fStack_8d4 = local_928;
    pfVar12 = (float *)(**(code **)(*(int *)param_1[0x69] + 4))();
    fStack_908 = pfVar12[2];
    fStack_90c = pfVar12[1];
    fStack_910 = *pfVar12;
    if ((((char)param_1[0xb3] != '\0') || (*(char *)((int)param_1 + 0x2f9) != '\0')) ||
       ((char)param_1[0xbe] != '\0')) {
      fStack_908 = (float)param_1[0x5b];
      fStack_910 = (float)param_1[0x59];
      fStack_90c = (float)param_1[0x5a];
    }
    fVar21 = ((float)unaff_EDI - fStack_90c) * ((float)unaff_EDI - fStack_90c) +
             ((float)pvVar8 - fStack_908) * ((float)pvVar8 - fStack_908) +
             (unaff_EBX - fStack_910) * (unaff_EBX - fStack_910);
    fVar22 = _DAT_004cf368;
    if (fVar21 < _DAT_004cf368 == (fVar21 == _DAT_004cf368)) {
      fVar22 = SQRT(fVar21);
    }
    fVar21 = fVar22 * _DAT_004cf374;
    if (fVar22 * _DAT_004cf374 < (float)_DAT_004cf428) {
      fVar21 = _DAT_004cf368;
    }
    if ((float)_DAT_004cf420 < fVar21) {
      fVar21 = _DAT_004cf374;
    }
    fVar21 = fVar21 + (float)param_1[0x54];
    bVar4 = _DAT_004cf41c <= fVar21;
    param_1[0x54] = (int)fVar21;
    if (bVar4) {
      param_1[0x54] = (int)(fVar21 - _DAT_004cf418);
    }
    local_920 = fStack_910;
    local_91c = fStack_90c;
    local_918 = fStack_908;
    (**(code **)(*(int *)param_1[0x69] + 0x18))();
    uStack_914 = (float)((uint)uStack_914 & 0xffffff);
    param_1[0x97] = (int)((float)param_1[0x97] * (float)_DAT_004cf410);
    *(undefined1 *)((int)param_1 + 0x14d) = 0;
    if ((char)param_1[0xc9] == '\0') {
      iVar10 = AthenaList_NextIndex(param_1[0x69] + 0x18);
      iVar9 = param_1[0x69];
      *(undefined4 *)(iVar9 + 0x20 + iVar10 * 4) = 0;
      if (*(int *)(iVar9 + 0x1c) < 1) {
        piVar16 = (int *)0x0;
      }
      else {
        piVar16 = (int *)**(undefined4 **)(iVar9 + 0x424);
        *(undefined4 *)(iVar9 + 0x20 + iVar10 * 4) = 1;
      }
      while (piVar16 != (int *)0x0) {
        if (((*piVar16 == 2) && ((undefined4 *)piVar16[0x19] == unaff_EBP)) &&
           (((char)param_1[0x1da] != '\0' && ((char)param_1[0xc9] == '\0')))) {
          if ((float *)(param_1 + 0xb7) != &fStack_8f4) {
            param_1[0xb7] = param_1[0x59];
            param_1[0xb8] = param_1[0x5a];
            param_1[0xb9] = param_1[0x5b];
          }
        }
        if (*piVar16 == 1) {
          if ((undefined4 *)piVar16[0x19] == unaff_EBP) {
            if ((1 < param_1[0xbb]) && (*(char *)((int)param_1 + 0x2e9) == '\0')) {
              Ball_ApplyTrajectory((int)param_1);
            }
            uStack_914 = (float)CONCAT13(1,(undefined3)uStack_914);
            if ((*(char *)((int)param_1 + 0x2e9) == '\x01') &&
               ((((iVar9 = param_1[0x69], _DAT_004cf368 <= *(float *)(iVar9 + 0xca8) &&
                  (param_1[0x1d2] == 0)) ||
                 ((*(float *)(iVar9 + 0xca4) < _DAT_004cf368 !=
                   (*(float *)(iVar9 + 0xca4) == _DAT_004cf368) && (param_1[0x1d2] == 1)))) ||
                ((_DAT_004cf368 <= *(float *)(iVar9 + 0xcac) && (param_1[0x1d2] == 2)))))) {
              (**(code **)(*param_1 + 0x20))();
            }
          }
          if ((*piVar16 == 1) && ((float)piVar16[0x19] == unaff_ESI)) {
            piVar2 = (int *)piVar16[3];
            local_938 = (void *)((float)param_1[0x5a] - (float)piVar2[0x5a]);
            unaff_EBX = ((float)param_1[0x59] - (float)piVar2[0x59]) * _DAT_004cf520;
            local_934 = ((float)param_1[0x5b] - (float)piVar2[0x5b]) * _DAT_004cf520;
            fVar21 = local_934 * local_934 +
                     (float)local_938 * _DAT_004cf520 * (float)local_938 * _DAT_004cf520 +
                     unaff_EBX * unaff_EBX;
            if ((fVar21 < _DAT_004cf368 != (fVar21 == _DAT_004cf368)) ||
               (SQRT(fVar21) < _DAT_004cf418)) {
              fVar22 = _DAT_004cf368;
              if ((fVar21 < _DAT_004cf368 == (fVar21 == _DAT_004cf368)) &&
                 (fVar22 = SQRT(fVar21), _DAT_004cf368 < fVar22)) {
                fVar22 = _DAT_004cf418 / fVar22;
              }
              unaff_EBX = unaff_EBX * fVar22;
              local_934 = local_934 * fVar22;
            }
            fStack_970 = 5.91681e-39;
            pvVar8 = (void *)piVar2[0x5b];
            (**(code **)(*piVar2 + 0x18))();
            pvStack_978 = pvVar8;
            fStack_970 = 1.0;
            fStack_974 = unaff_ESI;
            (**(code **)(*param_1 + 0x18))(pvStack_978);
            iVar9 = param_1[0x69];
            if (&local_920 != (float *)(iVar9 + 0xc98)) {
              local_91c = *(float *)(iVar9 + 0xc9c);
              local_920 = *(float *)(iVar9 + 0xc98);
              local_918 = *(float *)(iVar9 + 0xca0);
            }
            fVar22 = local_91c * local_91c + local_918 * local_918 + local_920 * local_920;
            fVar21 = _DAT_004cf368;
            if ((fVar22 < _DAT_004cf368 == (fVar22 == _DAT_004cf368)) &&
               (fVar21 = SQRT(fVar22), _DAT_004cf368 < fVar21)) {
              fVar21 = _DAT_004cf310 / fVar21;
            }
            local_920 = fVar21 * local_920;
            local_91c = local_91c * fVar21;
            local_918 = local_918 * fVar21;
            fVar21 = local_920 * (float)piVar16[8] +
                     local_91c * (float)piVar16[9] + local_918 * (float)piVar16[10];
            if ((fVar21 <= (float)_DAT_004cf518) || ((float)_DAT_004cf3e0 <= fVar21)) {
              fVar21 = (float)piVar16[0xe] * (float)piVar16[0xe] +
                       (float)piVar16[0xd] * (float)piVar16[0xd] +
                       (float)piVar16[0xc] * (float)piVar16[0xc];
              if ((fVar21 < _DAT_004cf368 == (fVar21 == _DAT_004cf368)) &&
                 (_DAT_004cf48c < SQRT(fVar21))) {
                local_92c = (void *)param_1[0x59];
                local_924 = (float)param_1[0x5b];
                local_928 = (float)param_1[0x5a];
                fStack_970 = 5.917578e-39;
                Sound_Play3D(*(void **)(param_1[4] + 0x43c),(float)local_92c,local_928,local_924);
              }
            }
            else {
              local_8e0[0] = (float)param_1[0x5b];
              fStack_8e8 = (float)param_1[0x59];
              fStack_8e4 = (float)param_1[0x5a];
              fStack_970 = 5.917374e-39;
              Sound_Play3D(*(void **)(param_1[4] + 0x43c),fStack_8e8,fStack_8e4,local_8e0[0]);
            }
            if (((float)piVar2[0xa1] < (float)param_1[0xa1] * (float)_DAT_004cf508) &&
               ((**(code **)(*piVar2 + 0x20))(), param_1[6] != -1)) {
              Difficulty_GetTimeModifier((void *)param_1[4],500.0);
              __ftol2();
              pcVar13 = (char *)AthenaString_Format(0x4f7448,&DAT_004cf500);
              if ((void *)param_1[0x30a] != (void *)0x0) {
                _free((void *)param_1[0x30a]);
              }
              pcVar14 = pcVar13;
              do {
                cVar1 = *pcVar14;
                pcVar14 = pcVar14 + 1;
              } while (cVar1 != '\0');
              pvVar8 = operator_new((uint)(pcVar14 + (1 - (int)(pcVar13 + 1))));
              param_1[0x30a] = (int)pvVar8;
              iVar9 = (int)pvVar8 - (int)pcVar13;
              do {
                cVar1 = *pcVar13;
                pcVar13[iVar9] = cVar1;
                pcVar13 = pcVar13 + 1;
              } while (cVar1 != '\0');
              param_1[3] = 200;
              pfVar12 = (float *)(param_1[6] * 0xa0 + 0x5e4 + param_1[4]);
              fVar17 = Difficulty_GetTimeModifier((void *)param_1[4],500.0);
              *pfVar12 = (float)(fVar17 + (float10)*pfVar12);
            }
            if (((float)param_1[0xa1] < (float)piVar2[0xa1] * (float)_DAT_004cf508) &&
               ((**(code **)(*param_1 + 0x20))(), piVar2[6] != -1)) {
              Difficulty_GetTimeModifier((void *)param_1[4],500.0);
              __ftol2();
              pcVar13 = (char *)AthenaString_Format(0x4f7448,&DAT_004cf500);
              if ((void *)piVar2[0x30a] != (void *)0x0) {
                _free((void *)piVar2[0x30a]);
              }
              pcVar14 = pcVar13;
              do {
                cVar1 = *pcVar14;
                pcVar14 = pcVar14 + 1;
              } while (cVar1 != '\0');
              pvVar8 = operator_new((uint)(pcVar14 + (1 - (int)(pcVar13 + 1))));
              piVar2[0x30a] = (int)pvVar8;
              iVar9 = (int)pvVar8 - (int)pcVar13;
              do {
                cVar1 = *pcVar13;
                pcVar13[iVar9] = cVar1;
                pcVar13 = pcVar13 + 1;
              } while (cVar1 != '\0');
              piVar2[3] = 200;
              pfVar12 = (float *)(piVar2[6] * 0xa0 + 0x5e4 + param_1[4]);
              fVar17 = Difficulty_GetTimeModifier((void *)param_1[4],500.0);
              *pfVar12 = (float)(fVar17 + (float10)*pfVar12);
            }
          }
        }
        iVar9 = param_1[0x69];
        iVar15 = *(int *)(iVar9 + 0x20 + iVar10 * 4);
        if (*(int *)(iVar9 + 0x1c) <= iVar15) break;
        piVar16 = *(int **)(*(int *)(iVar9 + 0x424) + iVar15 * 4);
        *(int *)(iVar9 + 0x20 + iVar10 * 4) = iVar15 + 1;
      }
    }
    if (*(char *)((int)param_1 + 0x14d) != '\0') {
      iVar9 = param_1[0xbd];
      param_1[0xbb] = 0;
      if (iVar9 < 0x4c) {
        iVar9 = 0x4b;
      }
      param_1[0xbd] = iVar9;
    }
    if (((*(char *)((int)param_1 + 0x2e9) != '\0') && ((char)param_1[0xc9] == '\0')) &&
       ((((iVar9 = param_1[0x1d2], iVar9 == 0 &&
          (ABS((float)param_1[0x5a] - fStack_90c) < (float)_DAT_004cf4f8)) ||
         ((iVar9 == 1 && (ABS((float)param_1[0x59] - fStack_910) < (float)_DAT_004cf4f8)))) ||
        ((iVar9 == 2 && (ABS((float)param_1[0x5b] - fStack_908) < (float)_DAT_004cf4f8)))))) {
      (**(code **)(*param_1 + 0x20))();
    }
    if ((char)param_1[0xc9] == '\0') {
      iVar10 = AthenaList_NextIndex(param_1[0x69] + 0x848);
      iVar9 = param_1[0x69];
      *(undefined4 *)(iVar9 + 0x850 + iVar10 * 4) = 0;
      if (*(int *)(iVar9 + 0x84c) < 1) {
        iVar15 = 0;
      }
      else {
        iVar15 = **(int **)(iVar9 + 0xc54);
        *(undefined4 *)(iVar9 + 0x850 + iVar10 * 4) = 1;
      }
      while (iVar15 != 0) {
        if ((char)param_1[0x1da] != '\0') {
          (**(code **)(*(int *)param_1[5] + 0x74))();
        }
        iVar9 = param_1[0x69];
        iVar3 = *(int *)(iVar9 + 0x850 + iVar10 * 4);
        if (*(int *)(iVar9 + 0x84c) <= iVar3) break;
        iVar15 = *(int *)(*(int *)(iVar9 + 0xc54) + iVar3 * 4);
        *(int *)(iVar9 + 0x850 + iVar10 * 4) = iVar3 + 1;
      }
      iVar10 = AthenaList_NextIndex(param_1[0x69] + 0x18);
      iVar9 = param_1[0x69];
      piVar16 = (int *)0x0;
      *(undefined4 *)(iVar9 + 0x20 + iVar10 * 4) = 0;
      if (0 < *(int *)(iVar9 + 0x1c)) {
        piVar16 = (int *)**(undefined4 **)(iVar9 + 0x424);
        *(undefined4 *)(iVar9 + 0x20 + iVar10 * 4) = 1;
      }
      while (piVar16 != (int *)0x0) {
        iVar9 = *piVar16;
        if (((iVar9 == 2) || (iVar9 == 1)) && ((undefined4 *)piVar16[0x19] == unaff_EBP)) {
          pfVar12 = (float *)(piVar16 + 8);
          fVar21 = -(fStack_8d0 * *pfVar12 +
                    fStack_8cc * (float)piVar16[9] + fStack_8c8 * (float)piVar16[10]);
          fStack_8f4 = fVar21 * *pfVar12;
          fVar22 = fStack_8c8 + fVar21 * (float)piVar16[10];
          fVar21 = fStack_8cc + fVar21 * (float)piVar16[9];
          fVar5 = fStack_8f4 + fStack_8d0;
          fVar7 = fVar22 * fVar22 + fVar21 * fVar21 + fVar5 * fVar5;
          fVar6 = _DAT_004cf368;
          if ((fVar7 < _DAT_004cf368 == (fVar7 == _DAT_004cf368)) &&
             (fVar6 = SQRT(fVar7), _DAT_004cf368 < fVar6)) {
            fVar6 = _DAT_004cf310 / fVar6;
          }
          iVar9 = param_1[0x69];
          bVar4 = _DAT_004cf4e0 <=
                  fVar5 * fVar6 * *(float *)(iVar9 + 0xc8c) +
                  fVar21 * fVar6 * *(float *)(iVar9 + 0xc90) +
                  fVar22 * fVar6 * *(float *)(iVar9 + 0xc94);
          local_92c = (void *)((float)param_1[0x59] - fStack_910);
          fVar21 = (float)local_92c * (float)local_92c +
                   ((float)param_1[0x5a] - fStack_90c) * ((float)param_1[0x5a] - fStack_90c) +
                   ((float)param_1[0x5b] - fStack_908) * ((float)param_1[0x5b] - fStack_908);
          fVar22 = _DAT_004cf368;
          if ((fVar21 < _DAT_004cf368 == (fVar21 == _DAT_004cf368)) &&
             (fVar22 = SQRT(fVar21), _DAT_004cf4dc < SQRT(fVar21))) {
            fVar22 = _DAT_004cf4dc;
          }
          param_1[0x97] = (int)(fVar22 * _DAT_004cf4d8);
          if ((*piVar16 == 2) &&
             (uStack_914._3_1_ = (char)((uint)uStack_914 >> 0x18), uStack_914._3_1_ == '\0')) {
            if (bVar4) {
              if ((float *)&stack0xfffff6c4 != pfVar12) {
                unaff_EBX = *pfVar12;
                local_934 = (float)piVar16[10];
                local_938 = (void *)piVar16[9];
              }
              iVar10 = *(int *)(piVar16[4] + 0x434);
              local_920 = unaff_EBX * *(float *)(iVar10 + 4) +
                          (float)local_938 * *(float *)(iVar10 + 0x14) +
                          local_934 * *(float *)(iVar10 + 0x24);
              local_91c = unaff_EBX * *(float *)(iVar10 + 8) +
                          (float)local_938 * *(float *)(iVar10 + 0x18) +
                          local_934 * *(float *)(iVar10 + 0x28);
              local_918 = unaff_EBX * *(float *)(iVar10 + 0xc) +
                          (float)local_938 * *(float *)(iVar10 + 0x1c) +
                          local_934 * *(float *)(iVar10 + 0x2c);
              fVar21 = -(fStack_8fc * local_91c + fStack_8f8 * local_918 + local_920 * fStack_900);
              fStack_8f4 = local_920 * fVar21;
              fStack_8f8 = fStack_8f8 + local_918 * fVar21;
              fStack_8fc = fStack_8fc + local_91c * fVar21;
              local_92c = (void *)(fStack_8f4 + fStack_900);
              fVar21 = *(float *)(iVar9 + 0xcac) * *(float *)(iVar9 + 0xcac) +
                       *(float *)(iVar9 + 0xca8) * *(float *)(iVar9 + 0xca8) +
                       *(float *)(iVar9 + 0xca4) * *(float *)(iVar9 + 0xca4);
              fVar22 = _DAT_004cf368;
              if (fVar21 < _DAT_004cf368 == (fVar21 == _DAT_004cf368)) {
                fVar22 = SQRT(fVar21);
              }
              fVar5 = fStack_8f8 * fStack_8f8 +
                      fStack_8fc * fStack_8fc + (float)local_92c * (float)local_92c;
              fVar21 = _DAT_004cf368;
              if (fVar5 < _DAT_004cf368 == (fVar5 == _DAT_004cf368)) {
                fVar21 = SQRT(fVar5);
              }
              if (((fVar21 <= fVar22) && (fVar5 < _DAT_004cf368 == (fVar5 == _DAT_004cf368))) &&
                 (_DAT_004cf418 < SQRT(fVar5))) {
                if (param_1 + 0x6a != (int *)&stack0xfffff6c4) {
                  param_1[0x6a] = (int)local_920;
                  param_1[0x6b] = (int)local_91c;
                  param_1[0x6c] = (int)local_918;
                }
                if ((float)pvStack_8c4 + fStack_904 == _DAT_004cf368) {
                  iVar9 = param_1[0x69];
                  if ((void **)(iVar9 + 0xca4) != &local_92c) {
                    *(void **)(iVar9 + 0xca4) = local_92c;
                    *(float *)(iVar9 + 0xca8) = fStack_8fc;
                    *(float *)(iVar9 + 0xcac) = fStack_8f8;
                  }
                }
              }
              break;
            }
          }
          else if (bVar4) break;
          fVar21 = (((_DAT_004cf310 / _DAT_004d8e00) /
                    (_DAT_004cf310 / (*(float *)(iVar9 + 0xc78) / _DAT_004d8dfc))) / _DAT_004d8e04)
                   * *(float *)(iVar9 + 0xc64) * (float)_DAT_004cf3e0 * (float)param_1[0xa7];
          if ((float)pvStack_8c4 + fStack_904 == _DAT_004cf368) {
            if (fVar21 < (float)_DAT_004cf4d0) {
              fVar21 = _DAT_004cf434;
            }
          }
          else if (fVar21 < (float)_DAT_004cf440) {
            *(undefined1 *)(param_1 + 0x98) = 1;
            fVar21 = _DAT_004cf380;
          }
          fVar21 = (fVar21 * _DAT_004d8e00 * _DAT_004d8e04) /
                   (*(float *)(iVar9 + 0xc78) / _DAT_004d8dfc);
          *(float *)(iVar9 + 0xc64) = fVar21;
          puVar23 = (undefined4 *)(fVar21 * *(float *)(iVar9 + 0xc94));
          if ((void **)(iVar9 + 0xc98) != &pvStack_8b8) {
            *(void **)(iVar9 + 0xc98) = (void *)(fVar21 * *(float *)(iVar9 + 0xc8c));
            *(float *)(iVar9 + 0xc9c) = fVar21 * *(float *)(iVar9 + 0xc90);
            *(undefined4 **)(iVar9 + 0xca0) = puVar23;
          }
          goto LAB_00407bb4;
        }
        if (iVar9 == 5) {
          if ((float)_DAT_004cf420 < (float)piVar16[0x15]) {
            if ((void **)(param_1 + 0x6a) != &pvStack_8b8) {
              param_1[0x6a] = 0;
              param_1[0x6b] = 0x3f800000;
              param_1[0x6c] = 0;
            }
            param_1[0x97] = (int)((float)param_1[0x97] * (float)_DAT_004cf438);
          }
          if ((_DAT_004cf310 < (float)piVar16[0x15]) && ((char)param_1[0x313] == '\0')) {
            *(undefined1 *)((int)param_1 + 0x2e9) = 1;
            Scene_SetCamera((void *)param_1[5],param_1,'\x01');
            fStack_970 = 5.91903e-39;
            Graphics_SetViewport
                      (*(void **)(*(int *)(param_1[5] + 0x878) + 0x174),&local_92c,param_1[0x59],
                       param_1[0x5a]);
            if ((((float)local_92c < _DAT_004cf4f0) ||
                ((local_928 < _DAT_004cf4f0 ||
                 ((float)(*(int *)(param_1[4] + 0x15c) + 0x32) < (float)local_92c)))) ||
               ((float)(*(int *)(param_1[4] + 0x160) + 0x32) < local_928)) {
              *(undefined1 *)(param_1 + 0xba) = 1;
            }
          }
          if (param_1[6] != -1) {
            if ((((float)_DAT_004cf4e8 <= (float)piVar16[0x15]) && (param_1[0xbb] == 0)) &&
               (param_1[0xbd] == 0)) {
              iVar9 = param_1[0x69];
              pfVar12 = (float *)(iVar9 + 0xc8c);
              if (&local_920 != pfVar12) {
                local_920 = *pfVar12;
                local_91c = *(float *)(iVar9 + 0xc90);
                local_918 = *(float *)(iVar9 + 0xc94);
              }
              unaff_EBX = (float)param_1[0x59];
              fVar21 = (float)param_1[0xa1] * _DAT_004cf3f0;
              fStack_8a0 = fVar21 * *pfVar12;
              if (&pvStack_978 != &pvStack_8b8) {
                pvStack_978 = (void *)(unaff_EBX - fStack_8a0);
                fStack_974 = (float)param_1[0x5a] - fVar21 * *(float *)(iVar9 + 0xc90);
                fStack_970 = (float)param_1[0x5b] - fVar21 * *(float *)(iVar9 + 0xc94);
              }
              Mesh_FindClosestCollision(*(void **)(param_1[5] + 0x8b0),&fStack_8e8);
              fStack_8f4 = (float)param_1[0x59];
              fStack_8a8 = fStack_8e4;
              if ((float)param_1[0xa1] * _DAT_004cf418 <
                  SQRT(((float)param_1[0x5b] - local_8e0[0]) * ((float)param_1[0x5b] - local_8e0[0])
                       + ((float)param_1[0x5a] - fStack_8e4) * ((float)param_1[0x5a] - fStack_8e4) +
                         (fStack_8f4 - fStack_8e8) * (fStack_8f4 - fStack_8e8))) {
                param_1[0xbb] = param_1[0xbb] + 1;
              }
            }
            if (((param_1[0xbb] != 0) && ((char)param_1[0x313] == '\0')) &&
               ((float)_DAT_004cf308 <= (float)piVar16[0x15])) {
              param_1[0xbb] = param_1[0xbb] + 1;
            }
          }
        }
        iVar9 = param_1[0x69];
        iVar15 = *(int *)(iVar9 + 0x20 + iVar10 * 4);
        if (*(int *)(iVar9 + 0x1c) <= iVar15) break;
        piVar16 = *(int **)(*(int *)(iVar9 + 0x424) + iVar15 * 4);
        *(int *)(iVar9 + 0x20 + iVar10 * 4) = iVar15 + 1;
      }
      iVar9 = param_1[0x69];
      fVar21 = (((_DAT_004cf310 / _DAT_004d8e00) /
                (_DAT_004cf310 / (*(float *)(iVar9 + 0xc78) / _DAT_004d8dfc))) / _DAT_004d8e04) *
               *(float *)(iVar9 + 0xc64) * (float)_DAT_004cf458 * (float)param_1[0xa7];
      if ((float)param_1[0xa9] < fVar21) {
        fVar21 = (float)param_1[0xa9];
        *(undefined1 *)(param_1 + 0x98) = 0;
        if ((char)param_1[0x313] != '\0') {
          fVar21 = fVar21 * _DAT_004cf380;
        }
      }
      fVar21 = (fVar21 * _DAT_004d8e00 * _DAT_004d8e04) /
               (*(float *)(iVar9 + 0xc78) / _DAT_004d8dfc);
      *(float *)(iVar9 + 0xc64) = fVar21;
      puVar23 = (undefined4 *)(fVar21 * *(float *)(iVar9 + 0xc94));
      if ((void **)(iVar9 + 0xc98) != &pvStack_8b8) {
        *(void **)(iVar9 + 0xc98) = (void *)(fVar21 * *(float *)(iVar9 + 0xc8c));
        *(float *)(iVar9 + 0xc9c) = fVar21 * *(float *)(iVar9 + 0xc90);
        *(undefined4 **)(iVar9 + 0xca0) = puVar23;
      }
    }
    else {
      iVar9 = param_1[0x69];
      fVar21 = ((float)param_1[0xa9] * _DAT_004d8e00 * _DAT_004d8e04) /
               (*(float *)(iVar9 + 0xc78) / _DAT_004d8dfc);
      *(float *)(iVar9 + 0xc64) = fVar21;
      puVar23 = (undefined4 *)(fVar21 * *(float *)(iVar9 + 0xc94));
      if ((void **)(iVar9 + 0xc98) != &pvStack_8b8) {
        *(void **)(iVar9 + 0xc98) = (void *)(fVar21 * *(float *)(iVar9 + 0xc8c));
        *(float *)(iVar9 + 0xc9c) = fVar21 * *(float *)(iVar9 + 0xc90);
        *(undefined4 **)(iVar9 + 0xca0) = puVar23;
      }
    }
LAB_00407bb4:
    (**(code **)*unaff_EBP)();
    if (puVar23 != (undefined4 *)0x0) {
      (**(code **)*puVar23)();
    }
    fStack_970 = 5.92195e-39;
    Sound_CalculateDistanceAttenuation
              (*(void **)(param_1[4] + 0x178),(float)param_1[0x59],(float)param_1[0x5a],
               (float)param_1[0x5b]);
    if (param_1[0x96] != 0) {
      Sound_Play3DAtPosition(param_1[0x96]);
    }
    RumbleBoard_TickTimer((int)(param_1 + 0x99));
    param_1[0x5f] = (int)(uStack_914 - (float)param_1[0x59]);
    pfVar12 = (float *)(param_1 + 0xb0);
    param_1[0x59] = (int)uStack_914;
    param_1[0x60] = (int)(fStack_910 - (float)param_1[0x5a]);
    param_1[0x5a] = (int)fStack_910;
    fVar21 = (float)param_1[0x5b];
    param_1[0x5b] = (int)fStack_90c;
    param_1[0x61] = (int)(fStack_90c - fVar21);
    fStack_970 = *pfVar12 * _DAT_004cf418 + uStack_914;
    fStack_974 = 5.922224e-39;
    (**(code **)(param_1[7] + 0x34))();
    pvStack_978 = (void *)param_1[0x5b];
    fStack_974 = 0.0;
    (**(code **)(param_1[7] + 0x34))(param_1[0x59],param_1[0x5a]);
    param_1[0x59] = (int)(*pfVar12 + (float)param_1[0x59]);
    param_1[0x5a] = (int)((float)param_1[0xb1] + (float)param_1[0x5a]);
    param_1[0x5b] = (int)((float)param_1[0xb2] + (float)param_1[0x5b]);
    if (pfVar12 != &fStack_8a4) {
      *pfVar12 = 0.0;
      param_1[0xb1] = 0;
      param_1[0xb2] = 0;
    }
    if (-1 < param_1[6]) {
      if (&local_918 != (float *)(param_1 + 0x1d6)) {
        local_918 = (float)param_1[0x1d6];
        uStack_914 = (float)param_1[0x1d7];
        fStack_910 = (float)param_1[0x1d8];
      }
      pfVar12 = (float *)(*(int *)(param_1[4] + 0x178) + 0x854 + param_1[6] * 0xc);
      *pfVar12 = local_918;
      pfVar12[1] = uStack_914;
      pfVar12[2] = fStack_910;
    }
    fVar17 = Math_Atan2Angle(&PTR_PTR_004f7188,(float)param_1[100],(float)param_1[0x65],0.0,0.0);
    pvVar8 = (void *)(float)fVar17;
    iVar9 = 3;
    if ((char)param_1[0x67] != '\0') {
      iVar9 = 0xf;
    }
    for (; local_92c = pvVar8, iVar9 != 0; iVar9 = iVar9 + -1) {
      uVar19 = __ftol2();
      uVar20 = __ftol2();
      if ((int)uVar19 != (int)uVar20) {
        iVar10 = Scene_CheckPath((int)uVar20,(int)uVar19);
        local_92c = (void *)((float)iVar10 + (float)local_92c);
      }
      pvVar8 = local_92c;
    }
    auVar18 = Wave_Sin(&PTR_PTR_004f7188,(float)pvVar8);
    param_1[100] = (int)(float)-(float10)auVar18;
    auVar18 = Wave_Cos(&PTR_PTR_004f7188,(float)pvVar8);
    param_1[0x65] = (int)(float)(float10)auVar18;
    if (param_1[0x1d2] == 0) {
      fVar17 = Math_Atan2Angle(&PTR_PTR_004f7188,(float)param_1[0x5f],(float)param_1[0x61],0.0,0.0);
      param_1[0x66] = (int)(float)fVar17;
    }
    if (param_1[0x1d2] == 1) {
      fVar17 = Math_Atan2Angle(&PTR_PTR_004f7188,-(float)param_1[0x60],(float)param_1[0x61],0.0,0.0)
      ;
      param_1[0x66] = (int)(float)fVar17;
    }
    if (param_1[0x1d2] == 2) {
      fVar17 = Math_Atan2Angle(&PTR_PTR_004f7188,(float)param_1[0x5f],(float)param_1[0x60],0.0,0.0);
      param_1[0x66] = (int)(float)fVar17;
    }
    *(undefined1 *)(param_1 + 0x67) = 0;
    Collision_GradientEval_Stub();
    uVar19 = __ftol2();
    param_1[2] = (int)uVar19;
    if ((char)param_1[0xc9] == '\0') {
      iVar9 = param_1[0x69];
      fVar21 = (float)param_1[0x62] * _DAT_004cf468;
      iVar10 = 3;
      do {
        fVar5 = *(float *)(iVar9 + 0xca4) * *(float *)(iVar9 + 0xca4) +
                *(float *)(iVar9 + 0xcac) * *(float *)(iVar9 + 0xcac) +
                *(float *)(iVar9 + 0xca8) * *(float *)(iVar9 + 0xca8);
        fVar22 = _DAT_004cf368;
        if (fVar5 < _DAT_004cf368 == (fVar5 == _DAT_004cf368)) {
          fVar22 = SQRT(fVar5);
        }
        if (fVar22 <= fVar21) {
          if (*(char *)((int)param_1 + 0x31e) == '\0') {
            fVar5 = (float)param_1[0xc6] - _DAT_004cf3d0;
            param_1[0xc6] = (int)fVar5;
            if (fVar5 < _DAT_004cf368) {
              param_1[0xc6] = 0;
            }
            fVar22 = fVar22 * _DAT_004cf374;
            *(undefined1 *)((int)param_1 + 0x31d) = 0;
            fVar7 = _DAT_004cf454;
            fVar5 = fVar22 + (float)param_1[0xc5];
            param_1[0xc5] = (int)fVar5;
            if (fVar7 < fVar5) {
              fVar5 = fVar5 - _DAT_004cf310;
              *(undefined1 *)(param_1 + 199) = 1;
              param_1[0xc5] = (int)fVar5;
            }
            fVar5 = ((float)param_1[0xc5] - (float)param_1[0xc6]) - _DAT_004cf3f0;
            param_1[0xc5] = (int)fVar5;
            if (((char)param_1[199] != '\0') && ((float)param_1[0xc6] == _DAT_004cf368)) {
              param_1[0xc5] = (int)(fVar5 - _DAT_004cf310);
            }
            if (((float)param_1[0xc5] < _DAT_004cf4c4) && ((float)param_1[0xc6] == _DAT_004cf368)) {
              *(undefined1 *)(param_1 + 199) = 0;
              param_1[0xc5] = (int)((float)param_1[0xc5] + _DAT_004cf48c);
            }
            if ((((float)param_1[0xc5] < _DAT_004cf368) && ((char)param_1[199] == '\0')) &&
               ((float)param_1[0xc6] == _DAT_004cf368)) {
              fVar5 = (float)param_1[0xc5] + _DAT_004cf380;
              param_1[0xc5] = (int)fVar5;
              if (fVar22 < _DAT_004cf41c) {
                param_1[0xc5] = (int)(fVar5 + _DAT_004cf380);
              }
              if (fVar22 < _DAT_004cf48c) {
                param_1[0xc5] = (int)((float)param_1[0xc5] + _DAT_004cf380);
              }
            }
          }
        }
        else {
          fVar22 = fVar22 * _DAT_004cf3f0;
          *(undefined1 *)((int)param_1 + 0x31d) = 1;
          param_1[0xc6] = (int)fVar22;
          fVar22 = (float)param_1[0xc5] - fVar22;
          bVar4 = _DAT_004cf4c8 <= fVar22;
          param_1[0xc5] = (int)fVar22;
          if (bVar4) {
            *(undefined1 *)(param_1 + 199) = 1;
          }
          else {
            fVar22 = fVar22 + _DAT_004cf44c;
            *(undefined1 *)(param_1 + 199) = 1;
            param_1[0xc5] = (int)fVar22;
          }
        }
        iVar10 = iVar10 + -1;
      } while (iVar10 != 0);
    }
    if (param_1[6] != -1) {
      local_8e0[0] = (float)param_1[0x59];
      fVar21 = (float)param_1[0x1d9];
      uStack_914 = ((float)param_1[0x5a] - (float)param_1[0x1d7]) * fVar21;
      fStack_910 = ((float)param_1[0x5b] - (float)param_1[0x1d8]) * fVar21;
      param_1[0x1d6] =
           (int)((local_8e0[0] - (float)param_1[0x1d6]) * fVar21 + (float)param_1[0x1d6]);
      param_1[0x1d7] = (int)(uStack_914 + (float)param_1[0x1d7]);
      param_1[0x1d8] = (int)(fStack_910 + (float)param_1[0x1d8]);
      if ((char)param_1[0x1da] == '\0') {
        param_1[0x1d9] = (int)((float)param_1[0x1d9] * _DAT_004cf4c0);
      }
      else {
        if ((float)param_1[0x1d9] < (float)_DAT_004cf538 !=
            ((float)param_1[0x1d9] == (float)_DAT_004cf538)) {
          param_1[0x1d9] = 0x3c23d70a;
        }
        fVar21 = (float)param_1[0x1d9];
        fVar22 = (float)_DAT_004cf4b8;
        param_1[0x1d9] = (int)(fVar21 * fVar22);
        if (_DAT_004cf310 < fVar21 * fVar22) {
          param_1[0x1d9] = 0x3f800000;
        }
      }
    }
    if (*(char *)((int)param_1 + 0x769) != '\0') {
      if ((float *)(param_1 + 0x1d6) != &fStack_8a4) {
        param_1[0x1d6] = param_1[0x59];
        param_1[0x1d7] = param_1[0x5a];
        param_1[0x1d8] = param_1[0x5b];
      }
      *(undefined1 *)((int)param_1 + 0x769) = 0;
    }
    if ((char)param_1[0x30f] != '\0') {
      param_1[0x59] = param_1[0x310];
      param_1[0x5a] = param_1[0x311];
      param_1[0x5b] = param_1[0x312];
      *(undefined1 *)(param_1 + 0x30f) = 0;
    }
    local_4 = 0xffffffff;
    Vec3List_Free(auStack_424);
  }
  ExceptionList = local_c;
  return;
}
