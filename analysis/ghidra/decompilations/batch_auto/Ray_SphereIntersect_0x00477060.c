
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __cdecl Ray_SphereIntersect(float *param_1,float *param_2,float *param_3,float *param_4)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  
  fVar1 = *param_3 - *param_1;
  fVar2 = param_3[1] - param_1[1];
  fVar3 = param_3[2] - param_1[2];
  fVar4 = fVar2 * fVar2 + fVar1 * fVar1 + fVar3 * fVar3;
  fVar5 = _DAT_004cf368;
  if (fVar4 < _DAT_004cf368 == (fVar4 == _DAT_004cf368)) {
    fVar5 = SQRT(fVar4);
  }
  fVar1 = fVar1 * *param_2 + fVar2 * param_2[1] + fVar3 * param_2[2];
  fVar2 = _DAT_004cf310 - (fVar5 * fVar5 - fVar1 * fVar1);
  if (fVar2 >= _DAT_004cf368) {
    *param_4 = fVar1 - SQRT(fVar2);
    return CONCAT31((int3)((uint)param_4 >> 8),1);
  }
  return CONCAT22((short)((uint)param_2 >> 0x10),
                  (ushort)(fVar2 < _DAT_004cf368) << 8 |
                  (ushort)(NAN(fVar2) || NAN(_DAT_004cf368)) << 10 |
                  (ushort)(fVar2 == _DAT_004cf368) << 0xe);
}

