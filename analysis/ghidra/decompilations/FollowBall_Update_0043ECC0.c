// FollowBall_Update @ 0x0043ECC0
// Decompiled via GhidraMCP v5.12.0-headless
// Session 2928 - Batch decompilation

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FollowBall_Update(int *param_1)

{
  float *pfVar1;
  char cVar2;
  double dVar3;
  int iVar4;
  void *pvVar5;
  float fVar6;
  int iVar7;
  float fVar8;
  undefined4 uVar9;
  float fVar10;
  float fVar11;
  undefined4 auStack_a4 [2];
  float afStack_9c [2];
  int local_94 [13];
  undefined4 uStack_60;
  float afStack_5c [16];
  void *pvStack_1c;
  int iStack_14;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cbca1;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  Timer_Init(local_94);
  local_4 = 0;
  Gfx_ScaleX(-(float)param_1[0x43c]);
  fVar11 = (float)_DAT_004d5d78 - (float)param_1[0x43e] * _DAT_004cf380;
  Gfx_SetPositionAndRender(fVar11,param_1[0x43e],fVar11);
  (**(code **)(local_94[0] + 0x18))();
  Gfx_SetPosition(0,0,0x425c0000);
  Gfx_ScaleX((float)param_1[0x43c]);
  Gfx_SetPosition(param_1[0x435],param_1[0x436],param_1[0x437]);
  pfVar1 = (float *)(param_1 + 0x438);
  if (pfVar1 != (float *)&stack0xffffff50) {
    *pfVar1 = 0.0;
    param_1[0x439] = 0;
    param_1[0x43a] = 0;
  }
  Matrix_TransformVec3(afStack_9c,pfVar1);
  (**(code **)(*param_1 + 0x58))();
  (**(code **)(*param_1 + 0x54))();
  cVar2 = (char)param_1[0x441];
  param_1[0x43b] = (int)((float)param_1[0x43b] + _DAT_004cf4dc);
  if (cVar2 == '\0') {
    dVar3 = _DAT_004d5d68;
    if (_DAT_004cf368 < (float)param_1[0x43f]) {
      dVar3 = _DAT_004d5d70;
    }
    param_1[0x43c] = (int)((float)param_1[0x43d] * (float)dVar3 + (float)param_1[0x43c]);
  }
  if (cVar2 == '\0') {
    fVar11 = (float)param_1[0x442] * (float)_DAT_004d5d60;
    param_1[0x442] = (int)fVar11;
    if (fVar11 < _DAT_004cf310) {
      param_1[0x442] = 0x3f800000;
    }
  }
  else {
    fVar11 = (float)param_1[0x442] * (float)_DAT_004cf4d0;
    param_1[0x442] = (int)fVar11;
    if (_DAT_004cf3ec < fVar11) {
      param_1[0x442] = 0x42480000;
    }
  }
  if (cVar2 == '\0') {
    fVar11 = (float)param_1[0x43e];
    param_1[0x43e] = (int)((float)param_1[0x43f] + fVar11);
    if ((float)_DAT_004cf3c8 <= (float)param_1[0x43f] + fVar11) {
      fVar11 = (float)_DAT_004d5d58;
      param_1[0x43e] = 0x3f800000;
      param_1[0x43f] = (int)-((fVar11 - (float)param_1[0x440]) * (float)_DAT_004d5d50);
      Timer_Init(&uStack_60);
      iStack_14._0_1_ = 1;
      Gfx_SetPosition(0,0,(float)param_1[0x440] * _DAT_004d0930);
      Gfx_ScaleX((float)param_1[0x43c] - (float)param_1[0x43d] * _DAT_004d039c);
      iVar4 = RNG_Rand(&PTR_OBJ_VTABLE,10,'\x01');
      iVar7 = param_1[0x436];
      fVar11 = (float)iVar4 + (float)param_1[0x437];
      iVar4 = RNG_Rand(&PTR_OBJ_VTABLE,10,'\x01');
      Gfx_SetPosition((float)iVar4 + (float)param_1[0x435],iVar7,fVar11);
      fVar10 = 0.0;
      fVar11 = 0.0;
      fVar6 = 6.238906e-39;
      Matrix_TransformVec3(afStack_5c,(float *)&stack0xffffff4c);
      fVar11 = fVar11 - _DAT_004cfecc;
      uVar9 = 0x20;
      fVar8 = 6.238935e-39;
      pvVar5 = operator_new(0x20);
      iStack_14._0_1_ = 2;
      if (pvVar5 == (void *)0x0) {
        pvVar5 = (void *)0x0;
      }
      else {
        if (&stack0xffffff38 != &stack0xffffff4c) {
          uVar9 = 0;
          fVar6 = fVar10;
          fVar8 = fVar11;
        }
        pvVar5 = RegisterDialog_Tick(pvVar5,*(undefined4 *)(param_1[0x434] + 0x878),fVar6,fVar8,
                                     uVar9);
      }
      iStack_14._0_1_ = 1;
      AthenaList_Append((void *)(param_1[0x434] + 0x3b00),(int)pvVar5);
      iStack_14 = (uint)iStack_14._1_3_ << 8;
      Timer_Cleanup(&uStack_60);
    }
    if ((float)param_1[0x43e] < (float)_DAT_004cf3e0 !=
        ((float)param_1[0x43e] == (float)_DAT_004cf3e0)) {
      param_1[0x43e] = 0x3f000000;
      param_1[0x43f] = (int)(((float)_DAT_004d5d58 - (float)param_1[0x440]) * (float)_DAT_004d5d48);
    }
  }
  iStack_14 = 0xffffffff;
  uVar9 = Timer_Cleanup(auStack_a4);
  ExceptionList = pvStack_1c;
  return CONCAT31((int3)((uint)uVar9 >> 8),1);
}