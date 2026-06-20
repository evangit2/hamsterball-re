
void Matrix_BuildLookAt(float *param_1,float *param_2,float *param_3,float *param_4)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  
  fVar4 = *param_3 - *param_2;
  fVar5 = param_3[1] - param_2[1];
  fVar6 = param_3[2] - param_2[2];
  Graphics_InitShaderDispatch();
  fVar1 = fVar6 * param_4[1] - fVar5 * param_4[2];
  fVar2 = fVar4 * param_4[2] - fVar6 * *param_4;
  fVar3 = fVar5 * *param_4 - fVar4 * param_4[1];
  Graphics_InitShaderDispatch();
  param_1[4] = fVar2;
  param_1[8] = fVar3;
  fVar7 = fVar5 * fVar3 - fVar6 * fVar2;
  fVar9 = fVar6 * fVar1 - fVar4 * fVar3;
  fVar8 = fVar4 * fVar2 - fVar5 * fVar1;
  *param_1 = fVar1;
  param_1[0xc] = -(fVar3 * param_2[2] + fVar1 * *param_2 + fVar2 * param_2[1]);
  param_1[1] = fVar7;
  param_1[5] = fVar9;
  param_1[9] = fVar8;
  fVar1 = param_2[1];
  fVar2 = *param_2;
  fVar3 = param_2[2];
  param_1[2] = fVar4;
  param_1[6] = fVar5;
  param_1[10] = fVar6;
  param_1[0xd] = -(fVar8 * fVar3 + fVar7 * fVar2 + fVar9 * fVar1);
  param_1[0xe] = -(fVar6 * param_2[2] + fVar4 * *param_2 + fVar5 * param_2[1]);
  param_1[3] = 0.0;
  param_1[7] = 0.0;
  param_1[0xb] = 0.0;
  param_1[0xf] = 1.0;
  return;
}

