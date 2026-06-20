
void Triangle_Interpolate2D
               (float *param_1,float *param_2,float *param_3,float *param_4,float param_5,
               float param_6)

{
  *param_1 = (*param_4 - *param_2) * param_6 + (*param_3 - *param_2) * param_5 + *param_2;
  param_1[1] = (param_4[1] - param_2[1]) * param_6 + (param_3[1] - param_2[1]) * param_5 +
               param_2[1];
  return;
}

