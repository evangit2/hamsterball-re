
float10 Vec2_Distance(float param_1,float param_2,float param_3,float param_4)

{
  return SQRT(((float10)param_3 - (float10)param_1) * ((float10)param_3 - (float10)param_1) +
              ((float10)param_4 - (float10)param_2) * ((float10)param_4 - (float10)param_2));
}

