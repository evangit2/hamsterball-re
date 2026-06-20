
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

ushort Collision_PointInTriangle(float param_1,float param_2,float *param_3)

{
  float fVar1;
  float fVar2;
  ushort uVar3;
  
  fVar1 = (param_2 - param_3[9]) * (param_3[0x10] - param_3[8]) -
          (param_3[0x11] - param_3[9]) * (param_1 - param_3[8]);
  fVar2 = ((param_2 - param_3[1]) * (param_3[8] - *param_3) -
          (param_3[9] - param_3[1]) * (param_1 - *param_3)) * fVar1;
  uVar3 = (ushort)(fVar2 < _DAT_004cf368) << 8 | (ushort)(NAN(fVar2) || NAN(_DAT_004cf368)) << 10 |
          (ushort)(fVar2 == _DAT_004cf368) << 0xe;
  if (fVar2 >= _DAT_004cf368 && (fVar2 == _DAT_004cf368) == 0) {
    fVar1 = ((param_2 - param_3[0x11]) * (*param_3 - param_3[0x10]) -
            (param_1 - param_3[0x10]) * (param_3[1] - param_3[0x11])) * fVar1;
    uVar3 = (ushort)(fVar1 < _DAT_004cf368) << 8 | (ushort)(NAN(fVar1) || NAN(_DAT_004cf368)) << 10
            | (ushort)(fVar1 == _DAT_004cf368) << 0xe;
    if (fVar1 >= _DAT_004cf368 && (fVar1 == _DAT_004cf368) == 0) {
      return CONCAT11((char)(uVar3 >> 8),1);
    }
  }
  return uVar3;
}

