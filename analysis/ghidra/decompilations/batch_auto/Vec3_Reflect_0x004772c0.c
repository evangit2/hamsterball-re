
void __cdecl Vec3_Reflect(float *param_1,float *param_2,float *param_3)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  
  fVar1 = param_3[1];
  fVar2 = param_3[2];
  fVar5 = param_2[2] * param_3[2] + param_3[1] * param_2[1] + *param_2 * *param_3;
  fVar3 = param_2[2];
  fVar4 = param_2[1];
  *param_1 = (*param_3 + *param_3) * fVar5 - *param_2;
  param_1[1] = (fVar1 + fVar1) * fVar5 - fVar4;
  param_1[2] = (fVar2 + fVar2) * fVar5 - fVar3;
  return;
}

