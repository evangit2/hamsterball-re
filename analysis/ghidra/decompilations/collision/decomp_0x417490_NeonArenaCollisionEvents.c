// NeonArenaCollisionEvents @ 0x417490
// Verified: vtable[0x1D] handler, created+decompiled June 2026


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall NeonArenaCollisionEvents(void *param_1,int *param_2,int *param_3)

{
  float *pfVar1;
  float fVar2;
  int iVar3;
  char *pcVar4;
  char *pcVar5;
  bool bVar6;
  float fStack_c;
  float fStack_8;
  float fStack_4;
  
  iVar3 = __strnicmp(*(char **)(param_3[1] + 0x864),"N:BUMP",8);
  if (iVar3 == 0) {
    fStack_c = (float)param_2[0x59];
    fStack_8 = (float)param_2[0x5a];
    fStack_4 = (float)param_2[0x5b];
    Sound_Play3D(*(void **)(*(int *)((int)param_1 + 0x878) + 0x524),fStack_c,fStack_8,fStack_4);
    iVar3 = param_2[0x69];
    pfVar1 = (float *)(iVar3 + 0xca4);
    if (&fStack_c != pfVar1) {
      fStack_c = *pfVar1;
      fStack_4 = *(float *)(iVar3 + 0xcac);
    }
    fStack_8 = 0.0;
    fStack_c = fStack_c * _DAT_004cf55c;
    fStack_4 = fStack_4 * _DAT_004cf55c;
    fVar2 = fStack_c * fStack_c + fStack_4 * fStack_4;
    if ((fVar2 < _DAT_004cf368 != (fVar2 == _DAT_004cf368)) || (SQRT(fVar2) < _DAT_004cf55c)) {
      Vec3_NormalizeAndScale(&fStack_c,5.0);
    }
    fVar2 = fStack_c * fStack_c + fStack_4 * fStack_4 + fStack_8 * fStack_8;
    if ((fVar2 < _DAT_004cf368 == (fVar2 == _DAT_004cf368)) && (_DAT_004cf4dc < SQRT(fVar2))) {
      Vec3_NormalizeAndScale(&fStack_c,8.0);
    }
    if (pfVar1 != &fStack_c) {
      *pfVar1 = fStack_c;
      *(float *)(iVar3 + 0xca8) = fStack_8;
      *(float *)(iVar3 + 0xcac) = fStack_4;
    }
  }
  pcVar5 = "DN:SINKPLATFORM";
  iVar3 = 0xf;
  bVar6 = true;
  pcVar4 = *(char **)(param_3[1] + 0x864);
  do {
    pcVar5 = pcVar5 + 1;
    if (iVar3 == 0) break;
    iVar3 = iVar3 + -1;
    bVar6 = *pcVar4 == *pcVar5;
    pcVar4 = pcVar4 + 1;
  } while (bVar6);
  if (bVar6) {
    Scene_StartCountdown(*(void **)(*param_3 + 0x47c),(int)param_2);
  }
  CreateNoDizzy(param_1,param_2,param_3);
  return;
}

