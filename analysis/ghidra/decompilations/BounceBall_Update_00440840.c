// BounceBall_Update @ 0x00440840
// Decompiled via GhidraMCP v5.12.0-headless
// Session 2928 - Batch decompilation

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall BounceBall_Update(int *param_1)

{
  float fVar1;
  float fVar2;
  undefined2 uVar6;
  void *pvVar3;
  int iVar4;
  undefined2 extraout_var;
  undefined4 uVar5;
  float local_68;
  float local_64;
  float local_60;
  float local_5c;
  float local_58;
  float local_54;
  int local_50 [17];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cbd83;
  local_c = ExceptionList;
  fVar2 = (float)param_1[0x439];
  uVar6 = (undefined2)((uint)param_1[0x43a] >> 0x10);
  if (param_1[0x43a] == 0) {
    fVar1 = (float)param_1[0x439] * (float)_DAT_004cf458;
    ExceptionList = &local_c;
    param_1[0x439] = (int)fVar1;
    if (_DAT_004d5dbc < fVar1) {
      param_1[0x439] = 0x42f00000;
      param_1[0x43a] = 1;
      pvVar3 = operator_new(0xc68);
      local_4 = 0;
      if (pvVar3 == (void *)0x0) {
        pvVar3 = (void *)0x0;
      }
      else {
        iVar4 = Level_FindObjectByName(*(void **)(param_1[0x434] + 0x8ac),"BallPath");
        pvVar3 = FollowBall_Ctor(pvVar3,param_1[0x434],iVar4);
      }
      local_4 = 0xffffffff;
      AthenaHashTable_Lookup
                (*(void **)(param_1[0x434] + 0x8ac),&local_68,"FOLLOWBALLSPOT",(undefined1 *)0x0);
      *(float *)((int)pvVar3 + 0x164) = local_68;
      *(float *)((int)pvVar3 + 0x168) = local_64;
      iVar4 = *(int *)((int)pvVar3 + 0x1a4);
      *(float *)((int)pvVar3 + 0x16c) = local_60;
      if ((float *)(iVar4 + 0xca4) != &local_5c) {
        *(float *)(iVar4 + 0xca4) = -3.0;
        *(undefined4 *)(iVar4 + 0xca8) = 0x41200000;
        *(undefined4 *)(iVar4 + 0xcac) = 0;
      }
      *(undefined4 *)((int)pvVar3 + 0x80c) = 0xf;
      AthenaList_Append((void *)(param_1[0x434] + 0x29d4),(int)pvVar3);
      local_5c = local_68;
      local_58 = local_64;
      local_54 = local_60;
      Sound_Play3D(*(void **)(*(int *)(param_1[0x434] + 0x878) + 0x468),local_68,local_64,local_60);
      uVar6 = extraout_var;
    }
  }
  else {
    fVar1 = (float)param_1[0x439] * (float)_DAT_004d5d98;
    ExceptionList = &local_c;
    param_1[0x439] = (int)fVar1;
    if (fVar1 < _DAT_004cf310) {
      param_1[0x439] = 0;
    }
  }
  fVar1 = (float)param_1[0x439];
  uVar5 = CONCAT22(uVar6,(ushort)(fVar1 < fVar2) << 8 | (ushort)(NAN(fVar1) || NAN(fVar2)) << 10 |
                         (ushort)(fVar1 == fVar2) << 0xe);
  if (fVar1 != fVar2) {
    *(undefined1 *)(param_1 + 0x43b) = 1;
  }
  if ((char)param_1[0x43b] != '\0') {
    *(undefined1 *)(param_1 + 0x43b) = 0;
    Timer_Init(local_50);
    local_4 = 1;
    Gfx_ScaleY((float)param_1[0x439]);
    Gfx_ScaleX((float)param_1[0x438]);
    if (&local_68 != (float *)(param_1 + 0x435)) {
      local_68 = (float)param_1[0x435];
      local_64 = (float)param_1[0x436];
      local_60 = (float)param_1[0x437];
    }
    (**(code **)(local_50[0] + 8))(local_68,local_64,local_60);
    (**(code **)(*param_1 + 0x58))();
    (**(code **)(*param_1 + 0x54))(&local_5c);
    local_4 = 0xffffffff;
    uVar5 = Timer_Cleanup(local_50);
  }
  ExceptionList = local_c;
  return CONCAT31((int3)((uint)uVar5 >> 8),1);
}