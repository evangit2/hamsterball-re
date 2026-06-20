
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall Impossible_Level_Update(int *param_1)

{
  float fVar1;
  float fVar2;
  bool bVar3;
  bool bVar4;
  bool bVar5;
  float fVar6;
  int iVar7;
  undefined2 uVar8;
  float local_5c;
  float local_58;
  float local_54;
  int local_50 [17];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cbbe8;
  local_c = ExceptionList;
  fVar2 = (float)param_1[0x436];
  iVar7 = param_1[0x43c];
  if (iVar7 < 100) {
    iVar7 = iVar7 + -1;
    ExceptionList = &local_c;
    param_1[0x43c] = iVar7;
    if (iVar7 < 1) {
      if (iVar7 == 0) {
        local_54 = (float)param_1[0x437];
        local_5c = (float)param_1[0x435];
        local_58 = (float)param_1[0x436];
        iVar7 = Sound_Play3D(*(void **)(*(int *)(param_1[0x434] + 0x878) + 0x4b8),local_5c,local_58,
                             local_54);
      }
      fVar1 = (float)param_1[0x43a];
      param_1[0x43c] = 0;
      fVar6 = (float)_DAT_004d5c60;
      param_1[0x43a] = (int)(fVar1 * fVar6);
      uVar8 = (undefined2)((uint)iVar7 >> 0x10);
      if (_DAT_004cf3ec < fVar1 * fVar6) {
        param_1[0x43a] = 0x42480000;
      }
      fVar1 = (float)param_1[0x436] - (float)param_1[0x43a];
      bVar3 = NAN(_DAT_004d5c5c);
      bVar4 = fVar1 < _DAT_004d5c5c;
      bVar5 = fVar1 == _DAT_004d5c5c;
      param_1[0x436] = (int)fVar1;
      if (bVar4) {
        ExceptionList = local_c;
        return CONCAT22(uVar8,(ushort)bVar4 << 8 | (ushort)(NAN(fVar1) || bVar3) << 10 |
                              (ushort)bVar5 << 0xe);
      }
      param_1[0x439] = (int)((float)param_1[0x439] - (float)param_1[0x43e] * (float)_DAT_004cf440);
      iVar7 = CONCAT22(uVar8,(ushort)(fVar1 < fVar2) << 8 | (ushort)(NAN(fVar1) || NAN(fVar2)) << 10
                             | (ushort)(fVar1 == fVar2) << 0xe);
      if (fVar1 != fVar2) {
        Timer_Init(local_50);
        local_4 = 0;
        Gfx_ScaleZ((float)param_1[0x439]);
        Gfx_ScaleX(_DAT_004cf44c - (float)param_1[0x438]);
        Vec3_CopyUnlessEqual(&local_5c,param_1 + 0x435);
        (**(code **)(local_50[0] + 8))(local_5c,local_58,local_54);
        (**(code **)(*param_1 + 0x58))();
        (**(code **)(*param_1 + 0x54))(&local_5c);
        local_4 = 0xffffffff;
        iVar7 = Timer_Cleanup(local_50);
      }
    }
  }
  ExceptionList = local_c;
  return CONCAT31((int3)((uint)iVar7 >> 8),1);
}

