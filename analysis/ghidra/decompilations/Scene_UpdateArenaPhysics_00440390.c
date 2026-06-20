// Scene_UpdateArenaPhysics @ 0x00440390
// Decompiled via GhidraMCP v5.12.0-headless
// Session 2928 - Batch decompilation

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall Scene_UpdateArenaPhysics(int *param_1)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  int *piVar5;
  undefined4 *puVar6;
  undefined4 *puVar7;
  undefined1 auVar8 [10];
  ulonglong uVar9;
  undefined4 uStack_bc;
  float fStack_b8;
  undefined4 uStack_b4;
  float local_90;
  float local_8c;
  float local_88;
  float fStack_84;
  float fStack_80;
  int iStack_7c;
  undefined4 *puStack_78;
  int *local_74;
  float fStack_70;
  undefined4 uStack_68;
  float fStack_64;
  int local_5c [17];
  undefined1 auStack_18 [12];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cbd58;
  local_c = ExceptionList;
  iVar4 = param_1[0x43e] + 1;
  ExceptionList = &local_c;
  param_1[0x43e] = iVar4;
  if (2 < iVar4) {
    local_74 = param_1;
    Timer_Init(local_5c);
    local_4 = 0;
    if (&local_90 != (float *)(param_1 + 0x438)) {
      local_90 = (float)param_1[0x438];
      local_8c = (float)param_1[0x439];
      local_88 = (float)param_1[0x43a];
    }
    (**(code **)(local_5c[0] + 8))();
    uStack_b4 = 0x440437;
    Graphics_BeginFrame(*(void **)(*(int *)(param_1[0x434] + 0x878) + 0x174),(int)&uStack_68);
    iVar4 = (**(code **)(*param_1 + 0x24))();
    if (iVar4 != 0) {
      param_1[0x43e] = 0;
      iStack_7c = 0;
      if (0 < param_1[0x43b]) {
        piVar5 = param_1 + 0x442;
        do {
          fStack_84 = (float)(param_1[0x43c] * 0x168);
          puStack_78 = (undefined4 *)
                       ((float)(int)fStack_84 * ((float)iStack_7c / (float)param_1[0x43b]) +
                       (float)param_1[0x436]);
          fStack_70 = ((float)iStack_7c / (float)param_1[0x43b]) * _DAT_004d0418;
          auVar8 = Wave_Sin(&PTR_PTR_004f7188,fStack_70);
          fStack_84 = (float)((float10)auVar8 * (float10)(float)param_1[0x43d]);
          iVar2 = AthenaList_NextIndex((int)(piVar5 + -1));
          iVar4 = 0;
          piVar5[iVar2 + 1] = 0;
          if (0 < *piVar5) {
            iVar4 = *(int *)piVar5[0x102];
            piVar5[iVar2 + 1] = 1;
          }
          while (iVar4 != 0) {
            auVar8 = Wave_Sin(&PTR_PTR_004f7188,(float)puStack_78);
            *(float *)(iVar4 + 4) = (float)((float10)auVar8 * (float10)fStack_84);
            if (_DAT_004cf368 < *(float *)(iVar4 + 0x10)) {
              auVar8 = Wave_Sin(&PTR_PTR_004f7188,(float)puStack_78 + _DAT_004d03a0);
              fStack_80 = (float)(float10)auVar8;
              auVar8 = Wave_Sin(&PTR_PTR_004f7188,fStack_70);
              *(float *)(iVar4 + 0x14) = (float)((float10)auVar8 * (float10)fStack_80);
            }
            iVar1 = piVar5[iVar2 + 1];
            if (*piVar5 <= iVar1) break;
            iVar4 = *(int *)(piVar5[0x102] + iVar1 * 4);
            piVar5[iVar2 + 1] = iVar1 + 1;
          }
          iVar4 = AthenaList_NextIndex((int)(piVar5 + 0x332b));
          piVar5[iVar4 + 0x332d] = 0;
          if (piVar5[0x332c] < 1) {
            iVar2 = 0;
          }
          else {
            iVar2 = *(int *)piVar5[0x342e];
            piVar5[iVar4 + 0x332d] = 1;
          }
          while (iVar2 != 0) {
            auVar8 = Wave_Sin(&PTR_PTR_004f7188,(float)puStack_78);
            *(float *)(iVar2 + 4) =
                 (float)((float10)auVar8 * (float10)fStack_84 - (float10)_DAT_004cfecc);
            iVar1 = piVar5[iVar4 + 0x332d];
            if (piVar5[0x332c] <= iVar1) break;
            iVar2 = *(int *)(piVar5[0x342e] + iVar1 * 4);
            piVar5[iVar4 + 0x332d] = iVar1 + 1;
          }
          iStack_7c = iStack_7c + 1;
          piVar5 = piVar5 + 0x106;
          param_1 = local_74;
        } while (iStack_7c < local_74[0x43b]);
      }
      piVar5 = (int *)0x0;
      iVar4 = AthenaList_NextIndex((int)(param_1 + 0x6a99));
      param_1[iVar4 + 0x6a9b] = 0;
      if (0 < param_1[0x6a9a]) {
        piVar5 = *(int **)param_1[0x6b9c];
        param_1[iVar4 + 0x6a9b] = 1;
      }
      while (piVar5 != (int *)0x0) {
        iVar2 = piVar5[1];
        piVar5[1] = iVar2 + -1;
        if (iVar2 + -1 < 1) {
          thunk_Gfx_SetRenderState(param_1 + 0x6a99,(int)piVar5);
          _free(piVar5);
        }
        else {
          iVar2 = *piVar5;
          iVar1 = *(int *)(iVar2 + 0x1a4);
          if (&local_90 != (float *)(iVar1 + 0xc98)) {
            local_90 = *(float *)(iVar1 + 0xc98);
            local_8c = *(float *)(iVar1 + 0xc9c);
            local_88 = *(float *)(iVar1 + 0xca0);
          }
          fStack_80 = *(float *)(iVar2 + 0x284);
          if (&uStack_bc != (undefined4 *)auStack_18) {
            uStack_bc = *(undefined4 *)(iVar2 + 0x164);
            fStack_b8 = *(float *)(iVar2 + 0x168) + _DAT_004cf454;
            uStack_b4 = *(undefined4 *)(iVar2 + 0x16c);
          }
          Mesh_FindClosestCollision((void *)param_1[0x435],&uStack_68);
          iVar2 = *piVar5;
          if (ABS(fStack_64 - *(float *)(iVar2 + 0x168)) < (float)param_1[0x43d]) {
            *(float *)(iVar2 + 0x168) = fStack_64 + *(float *)(iVar2 + 0x284);
          }
        }
        iVar2 = param_1[iVar4 + 0x6a9b];
        if (param_1[0x6a9a] <= iVar2) break;
        piVar5 = *(int **)(param_1[0x6b9c] + iVar2 * 4);
        param_1[iVar4 + 0x6a9b] = iVar2 + 1;
      }
      (**(code **)(**(int **)(param_1[0x120] + 0x44c) + 0x38))();
      puVar6 = (undefined4 *)param_1[0x43f];
      puVar7 = puStack_78;
      for (uVar3 = (uint)(param_1[0x440] << 5) >> 2; uVar3 != 0; uVar3 = uVar3 - 1) {
        *puVar7 = *puVar6;
        puVar6 = puVar6 + 1;
        puVar7 = puVar7 + 1;
      }
      for (iVar4 = 0; iVar4 != 0; iVar4 = iVar4 + -1) {
        *(undefined1 *)puVar7 = *(undefined1 *)puVar6;
        puVar6 = (undefined4 *)((int)puVar6 + 1);
        puVar7 = (undefined4 *)((int)puVar7 + 1);
      }
      uStack_b4 = 0x44078e;
      (**(code **)(**(int **)(param_1[0x120] + 0x44c) + 0x3c))();
      param_1[0x436] = (int)((float)param_1[0x437] + (float)param_1[0x436]);
      uVar9 = __ftol2();
      if ((int)((longlong)((ulonglong)(uint)((int)uVar9 >> 0x1f) << 0x20 | uVar9 & 0xffffffff) %
               0x168) == 0) {
        local_88 = (float)param_1[0x43a];
        local_90 = (float)param_1[0x438];
        local_8c = (float)param_1[0x439];
        uStack_b4 = 0x440802;
        Sound_Play3D(*(void **)(*(int *)(param_1[0x434] + 0x878) + 0x4e4),local_90,local_8c,local_88
                    );
      }
    }
    local_4 = 0xffffffff;
    iVar4 = Timer_Cleanup(local_5c);
  }
  ExceptionList = local_c;
  return CONCAT31((int3)((uint)iVar4 >> 8),1);
}