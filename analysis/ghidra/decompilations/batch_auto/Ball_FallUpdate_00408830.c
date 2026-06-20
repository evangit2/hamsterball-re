/*
 * Function: Ball_FallUpdate
 * Address: 0x00408830
 * Signature: Ball_FallUpdate(...)
 *
 * Patterns: allocates, vtable dispatch, SEH frame, collision, ball. Calls: Ball_FallUpdate, AthenaList_Init, AthenaList_Append, operator_new, SpatialTree_ctor, CollisionNode_ctor, ABS, SQRT. Offsets: 41, Lines: 165
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall Ball_FallUpdate(int param_1)

{
  int iVar1;
  int iVar2;
  float fVar3;
  float fVar4;
  uint uVar5;
  undefined4 *puVar6;
  undefined4 *puVar7;
  int iVar8;
  int iVar9;
  ulonglong uVar10;
  undefined4 uVar11;
  float fStack_460;
  float fStack_45c;
  undefined4 *puStack_458;
  void *pvStack_454;
  undefined4 uStack_450;
  undefined4 uStack_44c;
  undefined4 uStack_448;
  void *apvStack_444 [2];
  undefined4 local_43c;
  undefined4 local_438;
  undefined4 local_434;
  undefined1 local_424 [1024];
  void *pvStack_24;
  undefined4 uStack_1c;
  void *pvStack_c;
  undefined1 *puStack_8;
  int local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c9457;
  pvStack_c = ExceptionList;
  uVar5 = *(int *)(param_1 + 0x80c) - 1;
  ExceptionList = &pvStack_c;
  *(uint *)(param_1 + 0x80c) = uVar5 & ((int)uVar5 < 1) - 1;
  AthenaList_Init(local_424,0);
  local_4 = 0;
  AthenaList_Append(local_424,*(int *)(*(int *)(param_1 + 0x14) + 0x8b0));
  *(undefined4 *)(*(int *)(param_1 + 0x1a4) + 0xc70) = *(undefined4 *)(param_1 + 0x188);
  local_43c = *(undefined4 *)(param_1 + 0x170);
  local_434 = *(undefined4 *)(param_1 + 0x178);
  local_438 = *(undefined4 *)(param_1 + 0x174);
  *(undefined4 *)(param_1 + 0x178) = 0;
  *(undefined4 *)(param_1 + 0x174) = 0;
  *(undefined4 *)(param_1 + 0x170) = 0;
  (**(code **)(**(int **)(param_1 + 0x1a4) + 0x18))();
  pvStack_454 = operator_new(0x20);
  local_4._0_1_ = 1;
  if (pvStack_454 == (void *)0x0) {
    apvStack_444[0] = (void *)0x0;
  }
  else {
    apvStack_444[0] = SpatialTree_ctor(pvStack_454,local_424);
  }
  *(undefined4 *)((int)apvStack_444[0] + 0x10) = 10;
  *(undefined4 *)((int)apvStack_444[0] + 0xc) = *(undefined4 *)(param_1 + 0x278);
  local_4._0_1_ = 0;
  if ((*(int *)(param_1 + 0x80c) < 1) && (*(int *)(param_1 + 0xc60) == 0x3f800000)) {
    (**(code **)(**(int **)(param_1 + 0x1a4) + 0x14))();
  }
  pvStack_454 = operator_new(0x14);
  local_4._0_1_ = 2;
  if (pvStack_454 == (void *)0x0) {
    puVar6 = (undefined4 *)0x0;
  }
  else {
    puVar6 = CollisionNode_ctor(pvStack_454,*(int *)(param_1 + 0x14) + 0x29d4);
  }
  local_4 = (uint)local_4._1_3_ << 8;
  if ((*(char *)(param_1 + 0x324) == '\0') && (*(int *)(param_1 + 0xc60) == 0x3f800000)) {
    puVar6[3] = *(undefined4 *)(param_1 + 0x27c);
    (**(code **)(**(int **)(param_1 + 0x1a4) + 0x14))();
  }
  *(undefined4 *)(*(int *)(param_1 + 0x1a4) + 0xc78) = *(undefined4 *)(param_1 + 0x284);
  iVar1 = *(int *)(param_1 + 0x1a4);
  if (&fStack_460 != (float *)(iVar1 + 0xca4)) {
    fStack_460 = *(float *)(iVar1 + 0xca4);
    fStack_45c = *(float *)(iVar1 + 0xca8);
    puStack_458 = *(undefined4 **)(iVar1 + 0xcac);
  }
  if ((ABS(fStack_45c) < (float)_DAT_004cf3c8) &&
     ((fVar3 = fStack_45c * fStack_45c +
               fStack_460 * fStack_460 + (float)puStack_458 * (float)puStack_458,
      fVar3 < _DAT_004cf368 != (fVar3 == _DAT_004cf368) || (SQRT(fVar3) < _DAT_004cf418)))) {
    Vec3_NormalizeAndScale(&fStack_460,3.0);
    if ((float *)(iVar1 + 0xca4) != &fStack_460) {
      *(float *)(iVar1 + 0xca4) = fStack_460;
      *(float *)(iVar1 + 0xca8) = fStack_45c;
      *(undefined4 **)(iVar1 + 0xcac) = puStack_458;
    }
  }
  uStack_448 = *(undefined4 *)(param_1 + 0x284);
  uStack_450 = *(undefined4 *)(param_1 + 0x284);
  uStack_44c = *(undefined4 *)(param_1 + 0x284);
  fStack_460 = *(float *)(param_1 + 0x164);
  puStack_458 = *(undefined4 **)(param_1 + 0x16c);
  fStack_45c = *(float *)(param_1 + 0x168);
  puVar7 = (undefined4 *)(**(code **)(**(int **)(param_1 + 0x1a4) + 4))();
  fStack_45c = (float)puVar7[2];
  uVar11 = *puVar7;
  fStack_460 = (float)puVar7[1];
  if ((*(char *)(param_1 + 0x2cc) != '\0') || (*(char *)(param_1 + 0x2f9) != '\0')) {
    fStack_45c = *(float *)(param_1 + 0x16c);
    uVar11 = *(undefined4 *)(param_1 + 0x164);
    fStack_460 = *(float *)(param_1 + 0x168);
  }
  fVar3 = fStack_460;
  (**(code **)(**(int **)(param_1 + 0x1a4) + 0x18))();
  if (*(char *)(param_1 + 0x324) == '\0') {
    iVar8 = AthenaList_NextIndex(*(int *)(param_1 + 0x1a4) + 0x848);
    iVar1 = *(int *)(param_1 + 0x1a4);
    *(undefined4 *)(iVar1 + 0x850 + iVar8 * 4) = 0;
    if (*(int *)(iVar1 + 0x84c) < 1) {
      iVar9 = 0;
    }
    else {
      iVar9 = **(int **)(iVar1 + 0xc54);
      *(undefined4 *)(iVar1 + 0x850 + iVar8 * 4) = 1;
    }
    while (iVar9 != 0) {
      (**(code **)(**(int **)(param_1 + 0x14) + 0x74))();
      iVar1 = *(int *)(param_1 + 0x1a4);
      iVar2 = *(int *)(iVar1 + 0x850 + iVar8 * 4);
      if (*(int *)(iVar1 + 0x84c) <= iVar2) break;
      iVar9 = *(int *)(*(int *)(iVar1 + 0xc54) + iVar2 * 4);
      *(int *)(iVar1 + 0x850 + iVar8 * 4) = iVar2 + 1;
    }
  }
  iVar1 = *(int *)(param_1 + 0x1a4);
  fVar4 = (_DAT_004d8e00 * _DAT_004cf55c * _DAT_004d8e04) /
          (*(float *)(iVar1 + 0xc78) / _DAT_004d8dfc);
  *(float *)(iVar1 + 0xc64) = fVar4;
  pvStack_454 = (void *)(fVar4 * *(float *)(iVar1 + 0xc90));
  if ((void **)(iVar1 + 0xc98) != apvStack_444) {
    *(void **)(iVar1 + 0xc98) = (void *)(fVar4 * *(float *)(iVar1 + 0xc8c));
    *(void **)(iVar1 + 0xc9c) = pvStack_454;
    *(float *)(iVar1 + 0xca0) = fVar4 * *(float *)(iVar1 + 0xc94);
  }
  (**(code **)*puStack_458)();
  if (puVar6 != (undefined4 *)0x0) {
    (**(code **)*puVar6)();
  }
  *(float *)(param_1 + 0x16c) = fVar3;
  *(undefined4 **)(param_1 + 0x164) = &uStack_450;
  *(undefined4 *)(param_1 + 0x168) = uVar11;
  Collision_GradientEval_Stub();
  uVar10 = __ftol2();
  fVar3 = (float)_DAT_004cf3c8;
  *(int *)(param_1 + 8) = (int)uVar10;
  if ((*(float *)(param_1 + 0xc60) != fVar3) || (*(char *)(param_1 + 0x768) == '\0')) {
    fVar3 = *(float *)(param_1 + 0xc60) - _DAT_004cf448;
    *(float *)(param_1 + 0xc60) = fVar3;
    if (fVar3 < _DAT_004cf368 != (fVar3 == _DAT_004cf368)) {
      *(undefined1 *)(param_1 + 0x2e8) = 1;
    }
  }
  uStack_1c = 0xffffffff;
  Vec3List_Free(&local_43c);
  ExceptionList = pvStack_24;
  return;
}
