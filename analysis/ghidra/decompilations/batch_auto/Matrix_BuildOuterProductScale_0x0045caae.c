
void Matrix_BuildOuterProductScale(float *param_1,float *param_2)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float local_14;
  float local_10;
  float local_c;
  float local_8;
  
  D3D_ThunkIndirect();
  fVar1 = local_c * param_2[2] + local_8 * param_2[3] + local_14 * *param_2 + local_10 * param_2[1];
  fVar4 = -local_14;
  fVar3 = -local_10;
  fVar2 = -local_c;
  fVar5 = -local_8;
  *param_1 = fVar4 * *param_2 + fVar1;
  param_1[4] = fVar3 * *param_2;
  param_1[8] = fVar2 * *param_2;
  param_1[0xc] = fVar5 * *param_2;
  param_1[1] = fVar4 * param_2[1];
  param_1[5] = fVar3 * param_2[1] + fVar1;
  param_1[9] = fVar2 * param_2[1];
  param_1[0xd] = fVar5 * param_2[1];
  param_1[2] = fVar4 * param_2[2];
  param_1[6] = fVar3 * param_2[2];
  param_1[10] = fVar2 * param_2[2] + fVar1;
  param_1[0xe] = fVar5 * param_2[2];
  param_1[3] = fVar4 * param_2[3];
  param_1[7] = fVar3 * param_2[3];
  param_1[0xb] = fVar2 * param_2[3];
  param_1[0xf] = fVar5 * param_2[3] + fVar1;
  return;
}

