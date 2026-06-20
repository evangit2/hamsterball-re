/*
 * Function: RumbleBoard_Neon_Init
 * Address: 0x00416f40
 * Signature: RumbleBoard_Neon_Init(...)
 *
 * Patterns: allocates, vtable dispatch, SEH frame, matrix math, collision, camera, scene, board, level. Calls: RumbleBoard_Neon_Init, operator_new, MeshWorld_ctor, CollisionLevel_ctorWithLevel, CameraLookAt, AthenaList_NextIndex, Matrix_Scale4x4, Matrix_Identity. Offsets: 26, Lines: 150
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall RumbleBoard_Neon_Init(int *param_1)

{
  int iVar1;
  int *piVar2;
  float fVar3;
  void *pvVar4;
  int iVar5;
  int iVar6;
  undefined4 uStack_48;
  undefined4 uStack_44;
  undefined4 uStack_40;
  undefined4 uStack_3c;
  undefined4 uStack_38;
  undefined4 uStack_34;
  undefined4 uStack_30;
  undefined4 uStack_2c;
  undefined4 uStack_28;
  float fStack_24;
  undefined4 uStack_20;
  undefined4 uStack_1c;
  void *pvStack_18;
  undefined4 uStack_14;
  undefined4 uStack_10;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c9c51;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  pvVar4 = operator_new(0x10d0);
  iVar6 = 0;
  local_4 = 0;
  if (pvVar4 == (void *)0x0) {
    pvVar4 = (void *)0x0;
  }
  else {
    pvVar4 = MeshWorld_ctor(pvVar4,*(undefined4 *)(param_1[0x21e] + 0x174),"levels\\arena-neon");
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
  CameraLookAt(param_1);
  (**(code **)(*param_1 + 0x80))();
  iVar5 = AthenaList_NextIndex((int)(param_1 + 0xa75));
  param_1[iVar5 + 0xa77] = 0;
  if (0 < param_1[0xa76]) {
    iVar6 = *(int *)param_1[0xb78];
    param_1[iVar5 + 0xa77] = 1;
  }
  while (iVar6 != 0) {
    Matrix_Scale4x4(&uStack_48,0x3f800000,0x3f800000,0,0x3f800000);
    fVar3 = (float)_DAT_004cf3c8;
    *(undefined4 *)(iVar6 + 0x1d4) = uStack_3c;
    *(undefined4 *)(iVar6 + 0x1cc) = uStack_44;
    *(undefined4 *)(iVar6 + 0x1d0) = uStack_40;
    *(undefined4 *)(iVar6 + 0x1d8) = uStack_38;
    *(bool *)(iVar6 + 0x204) = *(float *)(iVar6 + 0x1c8) != fVar3;
    Matrix_Identity(&uStack_48);
    Matrix_Scale4x4(&uStack_34,0x3f800000,0x3f800000,0,0x3f800000);
    *(float *)(iVar6 + 0x1c8) = fStack_24;
    fVar3 = (float)_DAT_004cf3c8;
    *(undefined4 *)(iVar6 + 0x1c0) = uStack_2c;
    *(undefined4 *)(iVar6 + 0x1bc) = uStack_30;
    *(undefined4 *)(iVar6 + 0x1c4) = uStack_28;
    *(bool *)(iVar6 + 0x204) = fStack_24 != fVar3;
    Matrix_Identity(&uStack_34);
    Matrix_Scale4x4(&uStack_20,0x3f800000,0x3f800000,0,0x3f800000);
    fVar3 = (float)_DAT_004cf3c8;
    *(void **)(iVar6 + 0x1f0) = pvStack_18;
    *(undefined4 *)(iVar6 + 0x1ec) = uStack_1c;
    *(undefined4 *)(iVar6 + 500) = uStack_14;
    *(undefined4 *)(iVar6 + 0x1f8) = uStack_10;
    *(bool *)(iVar6 + 0x204) = *(float *)(iVar6 + 0x1c8) != fVar3;
    Matrix_Identity(&uStack_20);
    iVar1 = param_1[iVar5 + 0xa77];
    if (param_1[0xa76] <= iVar1) break;
    iVar6 = *(int *)(param_1[0xb78] + iVar1 * 4);
    param_1[iVar5 + 0xa77] = iVar1 + 1;
  }
  iVar6 = AthenaList_NextIndex((int)(param_1 + 0xb7b));
  param_1[iVar6 + 0xb7d] = 0;
  if (param_1[0xb7c] < 1) {
    iVar5 = 0;
  }
  else {
    iVar5 = *(int *)param_1[0xc7e];
    param_1[iVar6 + 0xb7d] = 1;
  }
  while (iVar5 != 0) {
    *(undefined1 *)(iVar5 + 0xc80) = 1;
    Matrix_Scale4x4(&uStack_20,0x3f800000,0x3f800000,0,0x3f800000);
    fVar3 = (float)_DAT_004cf3c8;
    *(undefined4 *)(iVar5 + 0x1ec) = uStack_1c;
    *(undefined4 *)(iVar5 + 0x1f8) = uStack_10;
    *(void **)(iVar5 + 0x1f0) = pvStack_18;
    *(undefined4 *)(iVar5 + 500) = uStack_14;
    *(bool *)(iVar5 + 0x204) = *(float *)(iVar5 + 0x1c8) != fVar3;
    Matrix_Identity(&uStack_20);
    Matrix_Scale4x4(&uStack_34,0x3f800000,0x3f800000,0,0x3f800000);
    *(float *)(iVar5 + 0x1c8) = fStack_24;
    fVar3 = (float)_DAT_004cf3c8;
    *(undefined4 *)(iVar5 + 0x1c4) = uStack_28;
    *(undefined4 *)(iVar5 + 0x1bc) = uStack_30;
    *(undefined4 *)(iVar5 + 0x1c0) = uStack_2c;
    *(bool *)(iVar5 + 0x204) = fStack_24 != fVar3;
    Matrix_Identity(&uStack_34);
    iVar1 = param_1[iVar6 + 0xb7d];
    if (param_1[0xb7c] <= iVar1) break;
    iVar5 = *(int *)(param_1[0xc7e] + iVar1 * 4);
    param_1[iVar6 + 0xb7d] = iVar1 + 1;
  }
  pvVar4 = operator_new(0xd4);
  local_4 = 2;
  if (pvVar4 == (void *)0x0) {
    pvVar4 = (void *)0x0;
  }
  else {
    pvVar4 = SceneObject_ctor(pvVar4,*(undefined4 *)(param_1[0x21e] + 0x174));
  }
  param_1[0x11f9] = (int)pvVar4;
  *(undefined4 *)((int)pvVar4 + 0xd0) = 1;
  iVar6 = param_1[0x11f9];
  local_4 = 0xffffffff;
  iVar5 = Vec3_Init(&uStack_20,0x41200000,0x41200000,0);
  *(undefined4 *)(iVar6 + 0x94) = *(undefined4 *)(iVar5 + 4);
  *(undefined4 *)(iVar6 + 0x98) = *(undefined4 *)(iVar5 + 8);
  *(undefined4 *)(iVar6 + 0x9c) = *(undefined4 *)(iVar5 + 0xc);
  *(undefined4 *)(iVar6 + 0xa0) = *(undefined4 *)(iVar5 + 0x10);
  Matrix_Identity(&uStack_20);
  (**(code **)(*(int *)param_1[0x11f9] + 4))(0,0,0);
  piVar2 = (int *)param_1[0x11f9];
  piVar2[0x33] = 0x43c80000;
  (**(code **)(*piVar2 + 0xc))();
  Scene_RegisterObject(*(void **)(param_1[0x21e] + 0x174),0,(int *)param_1[0x11f9]);
  ExceptionList = pvStack_18;
  return;
}
