
void __cdecl Vec3_ProjectOntoRay(float *param_1,float *param_2,float *param_3,float *param_4)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float local_18;
  float local_14;
  float local_10;
  float local_c;
  float local_8;
  float local_4;
  
  local_c = *param_4 - *param_2;
  local_8 = param_4[1] - param_2[1];
  local_4 = param_4[2] - param_2[2];
  local_18 = *param_3 - *param_2;
  local_14 = param_3[1] - param_2[1];
  local_10 = param_3[2] - param_2[2];
  Vec3_NormalizeAndScale(&local_18,1.0);
  fVar3 = local_18 * local_c + local_14 * local_8 + local_10 * local_4;
  fVar1 = param_2[2];
  fVar2 = param_2[1];
  *param_1 = local_18 * fVar3 + *param_2;
  param_1[1] = local_14 * fVar3 + fVar2;
  param_1[2] = fVar3 * local_10 + fVar1;
  return;
}

