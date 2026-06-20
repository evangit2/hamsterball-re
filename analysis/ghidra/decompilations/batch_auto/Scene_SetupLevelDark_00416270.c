/*
 * Function: Scene_SetupLevelDark
 * Address: 0x00416270
 * Signature: Scene_SetupLevelDark(...)
 *
 * Patterns: allocates, vtable dispatch, SEH frame, matrix math, collision, scene, level. Calls: Scene_SetupLevelDark, operator_new, MeshWorld_ctor, CollisionLevel_ctorWithLevel, Level_InitScene, AthenaList_NextIndex, Matrix_Scale4x4, Matrix_Identity. Offsets: 30, Lines: 187
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall Scene_SetupLevelDark(int *param_1)

{
  int iVar1;
  int *piVar2;
  float fVar3;
  void *pvVar4;
  int iVar5;
  int iVar6;
  undefined4 uStack_34;
  undefined4 uStack_30;
  undefined4 uStack_2c;
  undefined4 uStack_28;
  undefined4 uStack_24;
  undefined4 uStack_20;
  float fStack_1c;
  void *pvStack_18;
  undefined4 uStack_14;
  float fStack_10;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c9bcc;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  pvVar4 = operator_new(0x10d0);
  local_4 = 0;
  if (pvVar4 == (void *)0x0) {
    pvVar4 = (void *)0x0;
  }
  else {
    pvVar4 = MeshWorld_ctor(pvVar4,*(undefined4 *)(param_1[0x21e] + 0x174),"levels\\leveldark");
  }
  local_4 = 0xffffffff;
  param_1[0x22b] = (int)pvVar4;
  pvVar4 = operator_new(0x10d0);
  local_4 = 1;
  if (pvVar4 == (void *)0x0) {
    pvVar4 = (void *)0x0;
  }
  else {
    pvVar4 = CollisionLevel_ctorWithLevel(pvVar4,param_1[0x22b]);
  }
  local_4 = 0xffffffff;
  param_1[0x22c] = (int)pvVar4;
  Level_InitScene((int)param_1);
  (**(code **)(*param_1 + 0x80))();
  iVar5 = AthenaList_NextIndex((int)(param_1 + 0xb7b));
  param_1[iVar5 + 0xb7d] = 0;
  if (param_1[0xb7c] < 1) {
    iVar6 = 0;
  }
  else {
    iVar6 = *(int *)param_1[0xc7e];
    param_1[iVar5 + 0xb7d] = 1;
  }
  while (iVar6 != 0) {
    *(undefined1 *)(iVar6 + 0xc80) = 1;
    Matrix_Scale4x4(&uStack_34,0x3f800000,0x3f800000,0,0x3f800000);
    fVar3 = (float)_DAT_004cf3c8;
    *(undefined4 *)(iVar6 + 500) = uStack_28;
    *(undefined4 *)(iVar6 + 0x1ec) = uStack_30;
    *(undefined4 *)(iVar6 + 0x1f0) = uStack_2c;
    *(undefined4 *)(iVar6 + 0x1f8) = uStack_24;
    *(bool *)(iVar6 + 0x204) = *(float *)(iVar6 + 0x1c8) != fVar3;
    Matrix_Identity(&uStack_34);
    Matrix_Scale4x4(&uStack_20,0x3f800000,0x3f800000,0,0x3f800000);
    *(float *)(iVar6 + 0x1c8) = fStack_10;
    fVar3 = (float)_DAT_004cf3c8;
    *(void **)(iVar6 + 0x1c0) = pvStack_18;
    *(float *)(iVar6 + 0x1bc) = fStack_1c;
    *(undefined4 *)(iVar6 + 0x1c4) = uStack_14;
    *(bool *)(iVar6 + 0x204) = fStack_10 != fVar3;
    Matrix_Identity(&uStack_20);
    iVar1 = param_1[iVar5 + 0xb7d];
    if (param_1[0xb7c] <= iVar1) break;
    iVar6 = *(int *)(param_1[0xc7e] + iVar1 * 4);
    param_1[iVar5 + 0xb7d] = iVar1 + 1;
  }
  pvVar4 = operator_new(0xd4);
  local_4 = 2;
  if (pvVar4 == (void *)0x0) {
    pvVar4 = (void *)0x0;
  }
  else {
    pvVar4 = SceneObject_ctor(pvVar4,*(undefined4 *)(param_1[0x21e] + 0x174));
  }
  param_1[0x10db] = (int)pvVar4;
  *(undefined4 *)((int)pvVar4 + 0xd0) = 1;
  iVar5 = param_1[0x10db];
  local_4 = 0xffffffff;
  iVar6 = Vec3_Init(&uStack_20,0x41200000,0x41200000,0);
  *(undefined4 *)(iVar5 + 0x94) = *(undefined4 *)(iVar6 + 4);
  *(undefined4 *)(iVar5 + 0x98) = *(undefined4 *)(iVar6 + 8);
  *(undefined4 *)(iVar5 + 0x9c) = *(undefined4 *)(iVar6 + 0xc);
  *(undefined4 *)(iVar5 + 0xa0) = *(undefined4 *)(iVar6 + 0x10);
  Matrix_Identity(&uStack_20);
  iVar5 = *(int *)(param_1[0x21e] + 0x5dc);
  uStack_34 = *(undefined4 *)(iVar5 + 0x164);
  (**(code **)(*(int *)param_1[0x10db] + 4))
            (uStack_34,*(float *)(iVar5 + 0x168) + _DAT_004cf528,*(undefined4 *)(iVar5 + 0x16c));
  piVar2 = (int *)param_1[0x10db];
  piVar2[0x33] = 0x43c80000;
  (**(code **)(*piVar2 + 0xc))();
  Scene_RegisterObject(*(void **)(param_1[0x21e] + 0x174),0,(int *)param_1[0x10db]);
  Matrix_Scale4x4(&uStack_2c,0x3f800000,0x3f800000,0,0x3f800000);
  iVar5 = *(int *)(param_1[0x21e] + 0x5dc);
  *(undefined4 *)(iVar5 + 0x1d0) = uStack_24;
  *(undefined4 *)(iVar5 + 0x1d4) = uStack_20;
  *(undefined4 *)(iVar5 + 0x1cc) = uStack_28;
  *(float *)(iVar5 + 0x1d8) = fStack_1c;
  *(bool *)(iVar5 + 0x204) = *(float *)(iVar5 + 0x1c8) != (float)_DAT_004cf3c8;
  Matrix_Identity(&uStack_2c);
  Matrix_Scale4x4(&uStack_2c,0x3f800000,0x3f800000,0,0x3f800000);
  iVar5 = *(int *)(param_1[0x21e] + 0x5dc);
  *(undefined4 *)(iVar5 + 0x1c0) = uStack_24;
  *(undefined4 *)(iVar5 + 0x1c4) = uStack_20;
  *(undefined4 *)(iVar5 + 0x1bc) = uStack_28;
  *(float *)(iVar5 + 0x1c8) = fStack_1c;
  *(bool *)(iVar5 + 0x204) = fStack_1c != (float)_DAT_004cf3c8;
  Matrix_Identity(&uStack_2c);
  Matrix_Scale4x4(&uStack_2c,0x3f800000,0x3f800000,0,0x3f800000);
  iVar5 = *(int *)(param_1[0x21e] + 0x5dc);
  *(undefined4 *)(iVar5 + 0x1f0) = uStack_24;
  *(undefined4 *)(iVar5 + 500) = uStack_20;
  *(undefined4 *)(iVar5 + 0x1ec) = uStack_28;
  *(float *)(iVar5 + 0x1f8) = fStack_1c;
  *(bool *)(iVar5 + 0x204) = *(float *)(iVar5 + 0x1c8) != (float)_DAT_004cf3c8;
  Matrix_Identity(&uStack_2c);
  if (*(char *)(param_1[0x21e] + 0x677) == '\0') {
    Matrix_Scale4x4(&uStack_2c,0x3f800000,0x3f800000,0,0x3f800000);
    iVar5 = *(int *)(param_1[0x21e] + 0x67c);
    *(undefined4 *)(iVar5 + 0x1d0) = uStack_24;
    *(undefined4 *)(iVar5 + 0x1d4) = uStack_20;
    *(undefined4 *)(iVar5 + 0x1cc) = uStack_28;
    *(float *)(iVar5 + 0x1d8) = fStack_1c;
    *(bool *)(iVar5 + 0x204) = *(float *)(iVar5 + 0x1c8) != (float)_DAT_004cf3c8;
    Matrix_Identity(&uStack_2c);
    Matrix_Scale4x4(&uStack_2c,0x3f800000,0x3f800000,0,0x3f800000);
    iVar5 = *(int *)(param_1[0x21e] + 0x67c);
    *(undefined4 *)(iVar5 + 0x1c0) = uStack_24;
    *(undefined4 *)(iVar5 + 0x1c4) = uStack_20;
    *(undefined4 *)(iVar5 + 0x1bc) = uStack_28;
    *(float *)(iVar5 + 0x1c8) = fStack_1c;
    *(bool *)(iVar5 + 0x204) = fStack_1c != (float)_DAT_004cf3c8;
    Matrix_Identity(&uStack_2c);
    Matrix_Scale4x4(&uStack_2c,0x3f800000,0x3f800000,0,0x3f800000);
    iVar5 = *(int *)(param_1[0x21e] + 0x67c);
    *(undefined4 *)(iVar5 + 0x1f0) = uStack_24;
    *(undefined4 *)(iVar5 + 500) = uStack_20;
    *(undefined4 *)(iVar5 + 0x1ec) = uStack_28;
    *(float *)(iVar5 + 0x1f8) = fStack_1c;
    *(bool *)(iVar5 + 0x204) = *(float *)(iVar5 + 0x1c8) != (float)_DAT_004cf3c8;
    Matrix_Identity(&uStack_2c);
    pvVar4 = operator_new(0xd4);
    fStack_10 = 4.2039e-45;
    if (pvVar4 == (void *)0x0) {
      pvVar4 = (void *)0x0;
    }
    else {
      pvVar4 = SceneObject_ctor(pvVar4,*(undefined4 *)(param_1[0x21e] + 0x174));
    }
    param_1[0x10dc] = (int)pvVar4;
    *(undefined4 *)((int)pvVar4 + 0xd0) = 1;
    iVar5 = param_1[0x10dc];
    fStack_10 = -NAN;
    iVar6 = Vec3_Init(&uStack_2c,0x41200000,0x41200000,0);
    *(undefined4 *)(iVar5 + 0x94) = *(undefined4 *)(iVar6 + 4);
    *(undefined4 *)(iVar5 + 0x98) = *(undefined4 *)(iVar6 + 8);
    *(undefined4 *)(iVar5 + 0x9c) = *(undefined4 *)(iVar6 + 0xc);
    *(undefined4 *)(iVar5 + 0xa0) = *(undefined4 *)(iVar6 + 0x10);
    Matrix_Identity(&uStack_2c);
    iVar5 = *(int *)(param_1[0x21e] + 0x67c);
    (**(code **)(*(int *)param_1[0x10dc] + 4))
              (*(undefined4 *)(iVar5 + 0x164),*(float *)(iVar5 + 0x168) + _DAT_004cf528,
               *(undefined4 *)(iVar5 + 0x16c));
    piVar2 = (int *)param_1[0x10dc];
    piVar2[0x33] = 0x43c80000;
    (**(code **)(*piVar2 + 0xc))();
    Scene_RegisterObject(*(void **)(param_1[0x21e] + 0x174),1,(int *)param_1[0x10dc]);
  }
  ExceptionList = pvStack_18;
  return;
}
