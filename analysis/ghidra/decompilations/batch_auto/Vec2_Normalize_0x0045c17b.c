
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void Vec2_Normalize(float *param_1,float *param_2)

{
  float fVar1;
  int iVar2;
  float *extraout_ECX;
  float fVar3;
  
  fVar1 = param_2[1] * param_2[1] + *param_2 * *param_2;
  iVar2 = Float_IsBetween(fVar1,1.0);
  if (iVar2 == 0) {
    if (fVar1 <= _DAT_004d8f6c) {
      *param_1 = 0.0;
      param_1[1] = 0.0;
      return;
    }
    fVar3 = (_DAT_004cf310 / SQRT(fVar1)) * extraout_ECX[1];
    *param_1 = (_DAT_004cf310 / SQRT(fVar1)) * *extraout_ECX;
  }
  else {
    if (param_1 == extraout_ECX) {
      return;
    }
    *param_1 = *extraout_ECX;
    fVar3 = extraout_ECX[1];
  }
  param_1[1] = fVar3;
  return;
}

