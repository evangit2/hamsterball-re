
void __thiscall Path_GetPosition(void *this,float *param_1,float param_2)

{
  float10 fVar1;
  float10 fVar2;
  
  fVar1 = Spline_EvalCubic(this,param_2,(int)this + 0x30);
  fVar2 = Spline_EvalCubic(this,param_2,(int)this + 0x40);
  *param_1 = (float)fVar1;
  param_1[1] = 0.0;
  param_1[2] = (float)fVar2;
  return;
}

