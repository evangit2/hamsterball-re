/*
 * Function: Gear_AdvanceAlongPath
 * Address: 0x00418930
 * Signature: Gear_AdvanceAlongPath(...)
 *
 * Patterns: collision. Calls: Gear_AdvanceAlongPath, Path_GetPosition, Collision_GradientEval_Stub, SQRT, Window_Notify. Offsets: 1, Lines: 300
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* Gear_AdvanceAlongPath: Spline path follower using 8-sample gradient descent. Samples path at +/-
   step/medium/far offsets, picks lowest collision cost, moves path_param. First frame: 100K iter at
   0.01 step, then 1 iter at 0.001. */

void __thiscall Gear_AdvanceAlongPath(void *this,float param_1,float param_2,float param_3)

{
  void *pvVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  int iVar6;
  float *pfVar7;
  undefined4 *puVar8;
  bool bVar9;
  float10 fVar10;
  float local_140;
  int local_138;
  float local_128;
  float local_124;
  float local_120;
  float local_11c;
  int local_118;
  float local_108 [9];
  float local_e4;
  float local_e0;
  float local_dc;
  undefined4 local_d8;
  undefined4 local_d4;
  undefined4 local_d0;
  undefined4 local_cc;
  undefined4 local_c8;
  undefined4 local_c4;
  undefined4 local_c0;
  undefined4 local_bc;
  undefined4 local_b8;
  undefined4 local_b4;
  undefined4 local_b0;
  undefined4 local_ac;
  undefined4 local_a8;
  undefined4 local_a4;
  undefined4 local_a0;
  undefined4 local_9c;
  undefined4 local_98;
  undefined4 local_94;
  undefined4 local_90;
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  undefined4 local_80;
  undefined4 local_7c;
  float local_70;
  float local_6c [3];
  float local_60 [3];
  float local_54 [3];
  float local_48 [3];
  float local_3c [3];
  float local_30 [3];
  float local_24 [3];
  float local_18 [3];
  float local_c [3];
  
  bVar9 = *(char *)((int)this + 8) != '\0';
  iVar6 = 1;
  local_138 = 1;
  local_140 = 0.001;
  if (bVar9) {
    iVar6 = 100000;
    local_138 = 100000;
    local_140 = 0.01;
    *(undefined1 *)((int)this + 8) = 0;
    *(undefined4 *)((int)this + 0xc) = 10;
  }
  local_118 = 0;
  if (iVar6 != 0) {
    fVar3 = local_140 * _DAT_004cf9f8;
    fVar4 = local_140 * _DAT_004cf3ec;
    fVar5 = local_140 * _DAT_004cf454;
    do {
      pfVar7 = (float *)Path_GetPosition(*(void **)this,local_48,*(float *)((int)this + 4));
      if (&local_e4 != pfVar7) {
        local_e4 = *pfVar7;
        local_e0 = pfVar7[1];
        local_dc = pfVar7[2];
      }
      local_11c = *(float *)((int)this + 4) - local_140;
      if (local_11c <= _DAT_004cf368) {
        local_11c = 0.0;
      }
      puVar8 = (undefined4 *)Path_GetPosition(*(void **)this,local_24,local_11c);
      if (&local_d8 != puVar8) {
        local_d8 = *puVar8;
        local_d4 = puVar8[1];
        local_d0 = puVar8[2];
      }
      pvVar1 = *(void **)this;
      if (*(int *)((int)pvVar1 + 4) == 0) {
        iVar6 = 0;
      }
      else {
        iVar6 = *(int *)((int)pvVar1 + 8) - *(int *)((int)pvVar1 + 4) >> 2;
      }
      fVar2 = local_140 + *(float *)((int)this + 4);
      if ((float)(iVar6 + -1) <= fVar2) {
        if (*(int *)((int)pvVar1 + 4) == 0) {
          iVar6 = 0;
        }
        else {
          iVar6 = *(int *)((int)pvVar1 + 8) - *(int *)((int)pvVar1 + 4) >> 2;
        }
        fVar2 = (float)(iVar6 + -1);
      }
      puVar8 = (undefined4 *)Path_GetPosition(pvVar1,local_c,fVar2);
      if (&local_cc != puVar8) {
        local_cc = *puVar8;
        local_c8 = puVar8[1];
        local_c4 = puVar8[2];
      }
      local_128 = *(float *)((int)this + 4) - fVar3;
      if (local_128 <= _DAT_004cf368) {
        local_128 = 0.0;
      }
      puVar8 = (undefined4 *)Path_GetPosition(*(void **)this,local_30,local_128);
      if (&local_c0 != puVar8) {
        local_c0 = *puVar8;
        local_bc = puVar8[1];
        local_b8 = puVar8[2];
      }
      pvVar1 = *(void **)this;
      if (*(int *)((int)pvVar1 + 4) == 0) {
        iVar6 = 0;
      }
      else {
        iVar6 = *(int *)((int)pvVar1 + 8) - *(int *)((int)pvVar1 + 4) >> 2;
      }
      fVar2 = fVar3 + *(float *)((int)this + 4);
      if ((float)(iVar6 + -1) <= fVar2) {
        if (*(int *)((int)pvVar1 + 4) == 0) {
          iVar6 = 0;
        }
        else {
          iVar6 = *(int *)((int)pvVar1 + 8) - *(int *)((int)pvVar1 + 4) >> 2;
        }
        fVar2 = (float)(iVar6 + -1);
      }
      puVar8 = (undefined4 *)Path_GetPosition(pvVar1,local_6c,fVar2);
      if (&local_b4 != puVar8) {
        local_b4 = *puVar8;
        local_b0 = puVar8[1];
        local_ac = puVar8[2];
      }
      local_124 = *(float *)((int)this + 4) - fVar4;
      if (local_124 <= _DAT_004cf368) {
        local_124 = 0.0;
      }
      puVar8 = (undefined4 *)Path_GetPosition(*(void **)this,local_18,local_124);
      if (&local_a8 != puVar8) {
        local_a8 = *puVar8;
        local_a4 = puVar8[1];
        local_a0 = puVar8[2];
      }
      pvVar1 = *(void **)this;
      if (*(int *)((int)pvVar1 + 4) == 0) {
        iVar6 = 0;
      }
      else {
        iVar6 = *(int *)((int)pvVar1 + 8) - *(int *)((int)pvVar1 + 4) >> 2;
      }
      fVar2 = fVar4 + *(float *)((int)this + 4);
      if ((float)(iVar6 + -1) <= fVar2) {
        if (*(int *)((int)pvVar1 + 4) == 0) {
          iVar6 = 0;
        }
        else {
          iVar6 = *(int *)((int)pvVar1 + 8) - *(int *)((int)pvVar1 + 4) >> 2;
        }
        fVar2 = (float)(iVar6 + -1);
      }
      puVar8 = (undefined4 *)Path_GetPosition(pvVar1,local_60,fVar2);
      if (&local_9c != puVar8) {
        local_9c = *puVar8;
        local_98 = puVar8[1];
        local_94 = puVar8[2];
      }
      local_120 = *(float *)((int)this + 4) - fVar5;
      if (local_120 <= _DAT_004cf368) {
        local_120 = 0.0;
      }
      puVar8 = (undefined4 *)Path_GetPosition(*(void **)this,local_54,local_120);
      if (&local_90 != puVar8) {
        local_90 = *puVar8;
        local_8c = puVar8[1];
        local_88 = puVar8[2];
      }
      pvVar1 = *(void **)this;
      if (*(int *)((int)pvVar1 + 4) == 0) {
        iVar6 = 0;
      }
      else {
        iVar6 = *(int *)((int)pvVar1 + 8) - *(int *)((int)pvVar1 + 4) >> 2;
      }
      fVar2 = fVar5 + *(float *)((int)this + 4);
      if ((float)(iVar6 + -1) <= fVar2) {
        if (*(int *)((int)pvVar1 + 4) == 0) {
          iVar6 = 0;
        }
        else {
          iVar6 = *(int *)((int)pvVar1 + 8) - *(int *)((int)pvVar1 + 4) >> 2;
        }
        fVar2 = (float)(iVar6 + -1);
      }
      puVar8 = (undefined4 *)Path_GetPosition(pvVar1,local_3c,fVar2);
      if (&local_84 != puVar8) {
        local_84 = *puVar8;
        local_80 = puVar8[1];
        local_7c = puVar8[2];
      }
      iVar6 = 0;
      do {
        fVar10 = (float10)Collision_GradientEval_Stub();
        local_108[iVar6] = (float)fVar10;
        iVar6 = iVar6 + 1;
      } while (iVar6 < 8);
      iVar6 = 0;
      if ((local_108[1] < local_108[0]) || (fVar2 = local_108[0], local_108[0] == _DAT_004d0250)) {
        iVar6 = 1;
        fVar2 = local_108[1];
      }
      if ((local_108[2] < fVar2) || (fVar2 == _DAT_004d0250)) {
        iVar6 = 2;
        fVar2 = local_108[2];
      }
      if ((local_108[3] < fVar2) || (fVar2 == _DAT_004d0250)) {
        iVar6 = 3;
        fVar2 = local_108[3];
      }
      if ((local_108[4] < fVar2) || (fVar2 == _DAT_004d0250)) {
        iVar6 = 4;
        fVar2 = local_108[4];
      }
      if ((local_108[5] < fVar2) || (fVar2 == _DAT_004d0250)) {
        iVar6 = 5;
        fVar2 = local_108[5];
      }
      if ((local_108[6] < fVar2) || (fVar2 == _DAT_004d0250)) {
        iVar6 = 6;
        fVar2 = local_108[6];
      }
      if ((local_108[7] < fVar2) || (fVar2 == _DAT_004d0250)) {
        iVar6 = 7;
      }
      if (iVar6 == 1) {
        fVar2 = -local_140;
      }
      else {
        fVar2 = local_140;
        if (iVar6 != 2) {
          if (iVar6 == 3) {
            fVar2 = local_140 * _DAT_004d024c;
          }
          else {
            fVar2 = fVar3;
            if (iVar6 != 4) {
              if (iVar6 == 5) {
                fVar2 = local_140 * _DAT_004cf4f0;
              }
              else {
                fVar2 = fVar4;
                if (iVar6 != 6) {
                  if (iVar6 == 7) {
                    fVar2 = local_140 * _DAT_004d0248;
                  }
                  else {
                    fVar2 = _DAT_004cf368;
                    if (iVar6 == 8) {
                      fVar2 = fVar5;
                    }
                  }
                }
              }
            }
          }
        }
      }
      *(float *)((int)this + 4) = fVar2 + *(float *)((int)this + 4);
    } while ((((iVar6 != 1) && (iVar6 != 2)) ||
             ((local_138 != 1 &&
              (local_70 = local_dc,
              _DAT_004cf55c <=
              SQRT((param_1 - local_e4) * (param_1 - local_e4) +
                   (param_2 - local_e0) * (param_2 - local_e0) +
                   (param_3 - local_dc) * (param_3 - local_dc)))))) &&
            (local_118 = local_118 + 1, local_118 < local_138));
  }
  if (bVar9) {
    Window_Notify(0x5341d0,(byte *)0x4d0238);
  }
  return;
}
