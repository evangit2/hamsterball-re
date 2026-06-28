/*
 * Function: HandleArenaCollisionEvents
 * Address: 0x00412850
 * Signature: HandleArenaCollisionEvents(...)
 *
 * Patterns: allocates, SEH frame, audio, ball. Calls: HandleArenaCollisionEvents, __strnicmp, Rotator_AddObject, Sound_Play3D, SQRT, Vec3_NormalizeAndScale, _atol, AthenaHashTable_Lookup. Offsets: 25, Lines: 168
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall HandleArenaCollisionEvents(void *this,int *param_1,int *param_2)

{
  float *pfVar1;
  int iVar2;
  float10 fVar3;
  float10 fVar4;
  float fVar5;
  float fVar6;
  int *piVar7;
  int iVar8;
  long lVar9;
  void *this_00;
  int iVar10;
  void *pvVar11;
  undefined1 auVar12 [10];
  undefined1 auVar13 [10];
  float local_30;
  float local_2c;
  float local_28;
  float local_24;
  float local_20;
  float local_1c;
  float local_18;
  float local_14;
  float local_10;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  piVar7 = param_1;
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c9a7b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  iVar8 = __strnicmp(*(char **)(param_2[1] + 0x864),"N:SPINNER",9);
  if (iVar8 == 0) {
    Rotator_AddObject(*(void **)(*param_2 + 0x47c),(int)param_1);
  }
  iVar8 = __strnicmp(*(char **)(param_2[1] + 0x864),"N:BUMPER",8);
  if (iVar8 == 0) {
    local_1c = (float)param_1[0x5b];
    local_24 = (float)param_1[0x59];
    local_20 = (float)param_1[0x5a];
    Sound_Play3D(*(void **)(*(int *)((int)this + 0x878) + 0x448),local_24,local_20,local_1c);
    iVar8 = param_1[0x69];
    if (&local_30 != (float *)(iVar8 + 0xca4)) {
      local_30 = *(float *)(iVar8 + 0xca4);
      local_28 = *(float *)(iVar8 + 0xcac);
    }
    local_2c = 0.0;
    local_30 = local_30 * _DAT_004cf55c;
    local_28 = local_28 * _DAT_004cf55c;
    fVar5 = local_30 * local_30 + local_28 * local_28;
    if ((fVar5 < _DAT_004cf368 != (fVar5 == _DAT_004cf368)) || (SQRT(fVar5) < _DAT_004cf55c)) {
      Vec3_NormalizeAndScale(&local_30,5.0);
    }
    fVar5 = local_30 * local_30 + local_28 * local_28 + local_2c * local_2c;
    if ((fVar5 < _DAT_004cf368 == (fVar5 == _DAT_004cf368)) && (_DAT_004cf3dc < SQRT(fVar5))) {
      Vec3_NormalizeAndScale(&local_30,12.0);
    }
    if ((float *)(iVar8 + 0xca4) != &local_30) {
      *(float *)(iVar8 + 0xca4) = local_30;
      *(float *)(iVar8 + 0xca8) = local_2c;
      *(float *)(iVar8 + 0xcac) = local_28;
    }
    lVar9 = _atol((char *)(*(int *)(param_2[1] + 0x864) + 8));
    *(undefined4 *)((int)this + (lVar9 + -1) * 4 + 0x53fc) = 0x3f800000;
  }
  iVar8 = __strnicmp(*(char **)(param_2[1] + 0x864),"E:LAUNCH",8);
  if (iVar8 == 0) {
    AthenaHashTable_Lookup(*(void **)((int)this + 0x8ac),&local_30,"LAUNCHPOINT",(undefined1 *)0x0);
    param_1[0x5a] = (int)local_2c;
    iVar8 = param_1[0x69];
    param_1[0x59] = (int)local_30;
    param_1[0x5b] = (int)local_28;
    if ((float *)(iVar8 + 0xca4) != &local_24) {
      *(float *)(iVar8 + 0xca4) = 0.0;
      *(undefined4 *)(iVar8 + 0xca8) = 0x42200000;
      *(undefined4 *)(iVar8 + 0xcac) = 0;
    }
    param_1[200] = 0x19;
    param_1[0xa7] = 0x3b03126f;
    param_1[0xa8] = 5;
    param_1[0x202] = 0x32;
    Ball_RecordBest(param_1,200);
    local_24 = local_30;
    local_20 = local_2c;
    local_1c = local_28;
    Sound_Play3D(*(void **)(*(int *)((int)this + 0x878) + 0x500),local_30,local_2c,local_28);
    AthenaHashTable_Lookup
              (*(void **)((int)this + 0x8ac),&local_18,"EXPLODEHELPER",(undefined1 *)0x0);
    param_1 = (int *)0x0;
    do {
      this_00 = operator_new(0x28);
      pvVar11 = (void *)0x0;
      local_4 = 0;
      if (this_00 != (void *)0x0) {
        pvVar11 = ArenaScoreParticle_ctor(this_00,*(undefined4 *)((int)this + 0x878));
      }
      fVar5 = (float)(int)param_1;
      local_4 = 0xffffffff;
      auVar12 = Wave_Cos(&PTR_PTR_004f7188,fVar5);
      fVar4 = (float10)_DAT_004cf3ec;
      fVar3 = (float10)local_10;
      fVar6 = local_14 + _DAT_004cfecc;
      auVar13 = Wave_Sin(&PTR_PTR_004f7188,fVar5);
      if ((float *)((int)pvVar11 + 8) != &local_24) {
        *(float *)((int)pvVar11 + 8) =
             (float)((float10)auVar13 * (float10)_DAT_004cf3ec + (float10)local_18);
        *(float *)((int)pvVar11 + 0xc) = fVar6;
        *(float *)((int)pvVar11 + 0x10) = (float)((float10)auVar12 * fVar4 + fVar3);
      }
      auVar12 = Wave_Cos(&PTR_PTR_004f7188,fVar5);
      fVar3 = (float10)_DAT_004cf9f8;
      auVar13 = Wave_Sin(&PTR_PTR_004f7188,fVar5);
      pfVar1 = (float *)((int)pvVar11 + 0x14);
      if (pfVar1 != &local_24) {
        *pfVar1 = (float)((float10)auVar13 * (float10)_DAT_004cf9f8);
        *(undefined4 *)((int)pvVar11 + 0x18) = 0;
        *(float *)((int)pvVar11 + 0x1c) = (float)((float10)auVar12 * fVar3);
      }
      iVar8 = RNG_Rand(&PTR_OBJ_VTABLE,0x14,'\0');
      fVar5 = _DAT_004cf310 / (float)(iVar8 + 0x14);
      *pfVar1 = fVar5 * *pfVar1;
      *(float *)((int)pvVar11 + 0x18) = fVar5 * *(float *)((int)pvVar11 + 0x18);
      *(float *)((int)pvVar11 + 0x1c) = fVar5 * *(float *)((int)pvVar11 + 0x1c);
      AthenaList_Append((void *)((int)this + 0x3b00),(int)pvVar11);
      param_1 = param_1 + 5;
    } while ((int)param_1 < 0x168);
  }
  iVar8 = __stricmp(*(char **)(param_2[1] + 0x864),"E:CALLHAMMER");
  if ((iVar8 == 0) && (*(int *)(*(int *)((int)this + 0x878) + 0x23c) != 0)) {
    CreateBonkPopup(*(int *)((int)this + 0x540c));
  }
  iVar8 = __stricmp(*(char **)(param_2[1] + 0x864),"E:HAMMERCHASE");
  if ((iVar8 == 0) && (*(int *)(*(int *)((int)this + 0x878) + 0x23c) != 0)) {
    Hammer_ChaseStart(*(int *)((int)this + 0x540c));
  }
  iVar8 = __stricmp(*(char **)(param_2[1] + 0x864),"E:CATAPULTBOTTOM");
  if ((iVar8 == 0) && (piVar7[0x202] < 1)) {
    piVar7[0x202] = 1000;
    iVar8 = AthenaList_NextIndex((int)this + 0x584c);
    *(undefined4 *)((int)this + iVar8 * 4 + 0x5854) = 0;
    if (*(int *)((int)this + 0x5850) < 1) {
      iVar10 = 0;
    }
    else {
      iVar10 = **(int **)((int)this + 0x5c58);
      *(undefined4 *)((int)this + iVar8 * 4 + 0x5854) = 1;
    }
    while (iVar10 != 0) {
      if (*(int *)(iVar10 + 0x10d4) == *param_2) {
        *(int **)(iVar10 + 0x10ec) = piVar7;
        Catapult_Launch(iVar10);
        Sound_PlayChannel(*(int *)(*(int *)((int)this + 0x878) + 0x464));
      }
      iVar2 = *(int *)((int)this + iVar8 * 4 + 0x5854);
      if (*(int *)((int)this + 0x5850) <= iVar2) break;
      iVar10 = *(int *)(*(int *)((int)this + 0x5c58) + iVar2 * 4);
      *(int *)((int)this + iVar8 * 4 + 0x5854) = iVar2 + 1;
    }
  }
  DispatchCollisionEvents(this,piVar7,param_2);
  ExceptionList = local_c;
  return;
}
