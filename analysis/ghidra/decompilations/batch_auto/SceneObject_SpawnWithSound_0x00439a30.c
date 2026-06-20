
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall SceneObject_SpawnWithSound(int *param_1)

{
  float *pfVar1;
  float fVar2;
  float local_5c;
  int local_58;
  int local_54;
  int local_50 [13];
  void *pvStack_1c;
  undefined4 uStack_14;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cbbe8;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  AthenaList_Append((void *)(param_1[0x434] + 0x2578),(int)param_1);
  fVar2 = (float)param_1[0x43e] * _DAT_004d024c;
  param_1[0x43a] = 0x3f800000;
  param_1[0x43c] = 0x32;
  param_1[0x439] = (int)fVar2;
  param_1[0x436] = (int)((float)param_1[0x436] - _DAT_004cf3e8);
  Timer_Init(local_50);
  local_4 = 0;
  Gfx_ScaleZ((float)param_1[0x439]);
  Gfx_ScaleX(_DAT_004cf44c - (float)param_1[0x438]);
  pfVar1 = (float *)(param_1 + 0x435);
  if (&local_5c != pfVar1) {
    local_5c = *pfVar1;
    local_58 = param_1[0x436];
    local_54 = param_1[0x437];
  }
  (**(code **)(local_50[0] + 8))(local_5c,local_58,local_54);
  (**(code **)(*param_1 + 0x58))();
  (**(code **)(*param_1 + 0x54))(&local_5c);
  Sound_Play3D(*(void **)(*(int *)(param_1[0x434] + 0x878) + 0x4b4),*pfVar1,(float)param_1[0x436],
               (float)param_1[0x437]);
  uStack_14 = 0xffffffff;
  Timer_Cleanup((undefined4 *)&stack0xffffffa0);
  ExceptionList = pvStack_1c;
  return;
}

