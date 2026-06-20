// Lifter_Update @ 0x0043B330
// Decompiled via GhidraMCP v5.12.0-headless
// Session 2928 - Batch decompilation

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall Lifter_Update(int *param_1)

{
  float *pfVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  int iVar8;
  undefined4 uVar9;
  int iVar10;
  float unaff_EBX;
  void *_Memory;
  int *piVar11;
  float local_c8;
  float local_c4;
  float local_c0;
  undefined4 uStack_bc;
  float fStack_b8;
  float fStack_b4;
  float local_b0;
  float fStack_a8;
  float fStack_a4;
  float fStack_a0;
  float fStack_98;
  float fStack_94;
  float fStack_90;
  float fStack_88;
  float fStack_84;
  float fStack_80;
  float fStack_74;
  float fStack_70;
  float fStack_68;
  float fStack_64;
  undefined4 uStack_60;
  undefined1 auStack_5c [12];
  int local_50 [13];
  void *pvStack_1c;
  int iStack_14;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cbc53;
  pvStack_c = ExceptionList;
  local_b0 = (float)param_1[0x43a];
  fVar4 = (float)param_1[0x43b] * _DAT_004d5c88 + (float)param_1[0x43a];
  ExceptionList = &pvStack_c;
  param_1[0x43a] = (int)fVar4;
  if (_DAT_004cf48c < fVar4) {
    param_1[0x43b] = -0x40800000;
  }
  if ((float)param_1[0x43a] < _DAT_004d5c84) {
    param_1[0x43b] = 0x3f800000;
  }
  piVar11 = param_1 + 0x43c;
  iVar8 = AthenaList_NextIndex((int)piVar11);
  param_1[iVar8 + 0x43e] = 0;
  if (param_1[0x43d] < 1) {
    _Memory = (void *)0x0;
  }
  else {
    _Memory = *(void **)param_1[0x53f];
    param_1[iVar8 + 0x43e] = 1;
  }
  while (_Memory != (void *)0x0) {
    iVar10 = *(int *)((int)_Memory + 4) + -1;
    *(int *)((int)_Memory + 4) = iVar10;
    if (iVar10 < 1) {
      thunk_Gfx_SetRenderState(piVar11,(int)_Memory);
      _free(_Memory);
    }
    iVar10 = param_1[iVar8 + 0x43e];
    if (param_1[0x43d] <= iVar10) break;
    _Memory = *(void **)(param_1[0x53f] + iVar10 * 4);
    param_1[iVar8 + 0x43e] = iVar10 + 1;
  }
  param_1[0x439] = (int)(local_b0 + (float)param_1[0x439]);
  Timer_Init(local_50);
  local_4 = 0;
  Gfx_ScaleX((float)param_1[0x439]);
  pfVar1 = (float *)(param_1 + 0x436);
  if (&local_c8 != pfVar1) {
    local_c8 = *pfVar1;
    local_c4 = (float)param_1[0x437];
    local_c0 = (float)param_1[0x438];
  }
  (**(code **)(local_50[0] + 8))(local_c8,local_c4,local_c0);
  (**(code **)(*param_1 + 0x58))();
  (**(code **)(*param_1 + 0x54))(auStack_5c);
  iVar8 = AthenaList_NextIndex((int)piVar11);
  param_1[iVar8 + 0x43e] = 0;
  if (param_1[0x43d] < 1) {
    piVar11 = (int *)0x0;
  }
  else {
    piVar11 = *(int **)param_1[0x53f];
    param_1[iVar8 + 0x43e] = 1;
  }
  while (piVar11 != (int *)0x0) {
    iVar10 = *piVar11;
    fVar6 = *(float *)(iVar10 + 0x164) - *pfVar1;
    fVar5 = *(float *)(iVar10 + 0x168) - (float)param_1[0x437];
    fVar4 = *(float *)(iVar10 + 0x16c) - (float)param_1[0x438];
    Timer_Init(&uStack_bc);
    iStack_14._0_1_ = 1;
    Gfx_ScaleX(local_c0);
    iVar10 = *(int *)(*piVar11 + 0x1a4);
    fStack_68 = fStack_b4 * fVar6 + fStack_a4 * fVar5 + fStack_94 * fVar4 + fStack_84;
    fStack_64 = local_b0 * fVar6 + fStack_a0 * fVar5 + fStack_90 * fVar4 + fStack_80;
    if ((float *)&stack0xffffff34 != (float *)(iVar10 + 0xca4)) {
      unaff_EBX = *(float *)(iVar10 + 0xca4);
      local_c8 = *(float *)(iVar10 + 0xca8);
      local_c4 = *(float *)(iVar10 + 0xcac);
    }
    fVar7 = unaff_EBX * fStack_b8 + local_c8 * fStack_a8 + local_c4 * fStack_98;
    fStack_74 = fStack_b4 * unaff_EBX + local_c8 * fStack_a4 + local_c4 * fStack_94;
    local_c4 = local_b0 * unaff_EBX + local_c8 * fStack_a0 + local_c4 * fStack_90;
    if ((float *)(iVar10 + 0xca4) != (float *)&stack0xffffff34) {
      *(float *)(iVar10 + 0xca4) = fVar7;
      *(float *)(iVar10 + 0xca8) = fStack_74;
      *(float *)(iVar10 + 0xcac) = local_c4;
    }
    iStack_14 = (uint)iStack_14._1_3_ << 8;
    fVar2 = (float)param_1[0x437];
    fVar3 = (float)param_1[0x438];
    *(float *)(*piVar11 + 0x164) =
         fStack_b8 * fVar6 + fStack_a8 * fVar5 + fStack_98 * fVar4 + fStack_88 + *pfVar1;
    *(float *)(*piVar11 + 0x168) = fStack_68 + fVar2;
    *(float *)(*piVar11 + 0x16c) = fStack_64 + fVar3;
    local_c8 = fStack_74;
    fStack_70 = local_c4;
    Timer_Cleanup(&uStack_bc);
    iVar10 = param_1[iVar8 + 0x43e];
    if (param_1[0x43d] <= iVar10) break;
    piVar11 = *(int **)(param_1[0x53f] + iVar10 * 4);
    param_1[iVar8 + 0x43e] = iVar10 + 1;
    unaff_EBX = fVar7;
  }
  iStack_14 = 0xffffffff;
  uVar9 = Timer_Cleanup(&uStack_60);
  ExceptionList = pvStack_1c;
  return CONCAT31((int3)((uint)uVar9 >> 8),1);
}