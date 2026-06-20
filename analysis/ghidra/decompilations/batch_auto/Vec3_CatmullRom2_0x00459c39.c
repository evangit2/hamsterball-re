
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void Vec3_CatmullRom2(float *param_1,float *param_2,float *param_3,float *param_4,float *param_5,
                     float param_6)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  
  fVar6 = _DAT_004cf3f0;
  fVar1 = param_6 * param_6;
  fVar2 = param_6 * fVar1;
  fVar5 = (fVar1 * _DAT_004cf48c - fVar2) - param_6;
  fVar4 = (fVar2 * _DAT_004cf418 - fVar1 * _DAT_004cf55c) + _DAT_004cf48c;
  fVar3 = (fVar1 * _DAT_004cf41c - fVar2 * _DAT_004cf418) + param_6;
  fVar2 = fVar2 - fVar1;
  *param_1 = (fVar2 * *param_5 + fVar3 * *param_4 + fVar4 * *param_3 + fVar5 * *param_2) *
             _DAT_004cf3f0;
  param_1[1] = (fVar5 * param_2[1] + fVar4 * param_3[1] + fVar3 * param_4[1] + fVar2 * param_5[1]) *
               fVar6;
  param_1[2] = (fVar5 * param_2[2] + fVar4 * param_3[2] + fVar3 * param_4[2] + fVar2 * param_5[2]) *
               fVar6;
  return;
}

