
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl Segment_ClosestPoint(float *param_1,float *param_2,float *param_3,float *param_4)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float *pfVar4;
  float local_18;
  float local_14;
  float local_10;
  float local_c;
  float local_8;
  float local_4;
  
  pfVar4 = param_2;
  local_c = *param_4 - *param_2;
  local_8 = param_4[1] - param_2[1];
  local_4 = param_4[2] - param_2[2];
  local_18 = *param_3 - *param_2;
  local_14 = param_3[1] - param_2[1];
  local_10 = param_3[2] - param_2[2];
  fVar3 = local_18 * local_18 + local_14 * local_14 + local_10 * local_10;
  if (fVar3 < _DAT_004cf368 == (fVar3 == _DAT_004cf368)) {
    param_2 = (float *)SQRT(fVar3);
  }
  else {
    param_2 = (float *)0x0;
  }
  Vec3_NormalizeAndScale(&local_18,1.0);
  fVar3 = local_18 * local_c + local_14 * local_8 + local_10 * local_4;
  if (_DAT_004cf368 <= fVar3) {
    if (fVar3 <= (float)param_2) {
      fVar1 = pfVar4[2];
      fVar2 = pfVar4[1];
      *param_1 = local_18 * fVar3 + *pfVar4;
      param_1[1] = local_14 * fVar3 + fVar2;
      param_1[2] = local_10 * fVar3 + fVar1;
    }
    else if (param_1 != param_3) {
      *param_1 = *param_3;
      param_1[1] = param_3[1];
      param_1[2] = param_3[2];
      return;
    }
  }
  else if (param_1 != pfVar4) {
    *param_1 = *pfVar4;
    param_1[1] = pfVar4[1];
    param_1[2] = pfVar4[2];
    return;
  }
  return;
}

