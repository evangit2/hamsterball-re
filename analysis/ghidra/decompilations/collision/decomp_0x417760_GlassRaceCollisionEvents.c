// GlassRaceCollisionEvents @ 0x417760
// Verified: vtable[0x1D] handler, created+decompiled June 2026


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall GlassRaceCollisionEvents(void *param_1,int *param_2,int *param_3)

{
  float fVar1;
  float fVar2;
  int iVar3;
  void *pvVar4;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_004c9c96;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  iVar3 = __strnicmp(*(char **)(param_3[1] + 0x864),"N:GLASS",7);
  if (iVar3 == 0) {
    param_2[0x317] = 0xf;
  }
  iVar3 = __strnicmp(*(char **)(param_3[1] + 0x864),"N:TENBONUS1",0xb);
  if (iVar3 == 0) {
    iVar3 = param_2[0x69];
    fVar1 = *(float *)(iVar3 + 0xcac) * *(float *)(iVar3 + 0xcac) +
            *(float *)(iVar3 + 0xca8) * *(float *)(iVar3 + 0xca8) +
            *(float *)(iVar3 + 0xca4) * *(float *)(iVar3 + 0xca4);
    fVar2 = _DAT_004cf368;
    if (fVar1 < _DAT_004cf368 == (fVar1 == _DAT_004cf368)) {
      fVar2 = SQRT(fVar1);
    }
    if ((fVar2 < (float)_DAT_004d0178) || (*(char *)((int)param_1 + 0x438c) != '\0')) {
      fVar1 = *(float *)(iVar3 + 0xcac) * *(float *)(iVar3 + 0xcac) +
              *(float *)(iVar3 + 0xca8) * *(float *)(iVar3 + 0xca8) +
              *(float *)(iVar3 + 0xca4) * *(float *)(iVar3 + 0xca4);
      fVar2 = _DAT_004cf368;
      if (fVar1 < _DAT_004cf368 == (fVar1 == _DAT_004cf368)) {
        fVar2 = SQRT(fVar1);
      }
      if (((float)_DAT_004cf4f8 <= fVar2) && (*(char *)((int)param_1 + 0x438c) == '\0')) {
        Sound_Play3D(*(void **)(*(int *)((int)param_1 + 0x878) + 0x4d0),
                     *(float *)((int)param_1 + 0x436c),*(float *)((int)param_1 + 0x4370),
                     *(float *)((int)param_1 + 0x4374));
      }
    }
    else {
      *(undefined1 *)((int)param_1 + 0x438c) = 1;
      Sound_Play3D(*(void **)(*(int *)((int)param_1 + 0x878) + 0x52c),
                   *(float *)((int)param_1 + 0x436c),*(float *)((int)param_1 + 0x4370),
                   *(float *)((int)param_1 + 0x4374));
      iVar3 = *(int *)(*(int *)((int)param_1 + 0x878) + 0x220);
      if ((*(char *)(iVar3 + 0x10) == '\0') && (*(char *)(iVar3 + 0x11) == '\0')) {
        *(undefined4 *)(param_2[6] * 0xa0 + 0x5ec + *(int *)((int)param_1 + 0x878)) = 1000;
        pvVar4 = operator_new(0x30);
        iVar3 = 0;
        uStack_4 = 0;
        if (pvVar4 != (void *)0x0) {
          iVar3 = ScoreObject_ctor(pvVar4,(int)param_1,
                                   param_2[6] * 0xa0 + 0x5cc + *(int *)((int)param_1 + 0x878),
                                   "EXTRA TIME:");
        }
        uStack_4 = 0xffffffff;
        Timer_Decrement(iVar3);
        AthenaList_Append((void *)((int)param_1 + 0x8b8),iVar3);
      }
    }
  }
  iVar3 = __strnicmp(*(char **)(param_3[1] + 0x864),"N:TENBONUS2",0xb);
  if (iVar3 == 0) {
    iVar3 = param_2[0x69];
    fVar1 = *(float *)(iVar3 + 0xcac) * *(float *)(iVar3 + 0xcac) +
            *(float *)(iVar3 + 0xca8) * *(float *)(iVar3 + 0xca8) +
            *(float *)(iVar3 + 0xca4) * *(float *)(iVar3 + 0xca4);
    fVar2 = _DAT_004cf368;
    if (fVar1 < _DAT_004cf368 == (fVar1 == _DAT_004cf368)) {
      fVar2 = SQRT(fVar1);
    }
    if ((fVar2 < (float)_DAT_004d0178) || (*(char *)((int)param_1 + 0x438d) != '\0')) {
      fVar1 = *(float *)(iVar3 + 0xcac) * *(float *)(iVar3 + 0xcac) +
              *(float *)(iVar3 + 0xca8) * *(float *)(iVar3 + 0xca8) +
              *(float *)(iVar3 + 0xca4) * *(float *)(iVar3 + 0xca4);
      fVar2 = _DAT_004cf368;
      if (fVar1 < _DAT_004cf368 == (fVar1 == _DAT_004cf368)) {
        fVar2 = SQRT(fVar1);
      }
      if (((float)_DAT_004cf4f8 <= fVar2) && (*(char *)((int)param_1 + 0x438d) == '\0')) {
        Sound_Play3D(*(void **)(*(int *)((int)param_1 + 0x878) + 0x4d0),
                     *(float *)((int)param_1 + 0x4378),*(float *)((int)param_1 + 0x437c),
                     *(float *)((int)param_1 + 0x4380));
      }
    }
    else {
      *(undefined1 *)((int)param_1 + 0x438d) = 1;
      Sound_Play3D(*(void **)(*(int *)((int)param_1 + 0x878) + 0x52c),
                   *(float *)((int)param_1 + 0x4378),*(float *)((int)param_1 + 0x437c),
                   *(float *)((int)param_1 + 0x4380));
      iVar3 = *(int *)(*(int *)((int)param_1 + 0x878) + 0x220);
      if ((*(char *)(iVar3 + 0x10) == '\0') && (*(char *)(iVar3 + 0x11) == '\0')) {
        *(undefined4 *)(param_2[6] * 0xa0 + 0x5ec + *(int *)((int)param_1 + 0x878)) = 1000;
        pvVar4 = operator_new(0x30);
        uStack_4 = 1;
        if (pvVar4 == (void *)0x0) {
          iVar3 = 0;
        }
        else {
          iVar3 = ScoreObject_ctor(pvVar4,(int)param_1,
                                   param_2[6] * 0xa0 + 0x5cc + *(int *)((int)param_1 + 0x878),
                                   "EXTRA TIME:");
        }
        uStack_4 = 0xffffffff;
        Timer_Decrement(iVar3);
        AthenaList_Append((void *)((int)param_1 + 0x8b8),iVar3);
      }
    }
  }
  CreateNoDizzy(param_1,param_2,param_3);
  ExceptionList = pvStack_c;
  return;
}

