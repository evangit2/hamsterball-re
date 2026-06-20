
float10 __thiscall Gfx_TransformY(void *param_1,float param_2)

{
  return (float10)param_2 * (float10)*(float *)(*(int *)((int)param_1 + 0x5c) + 0x1f8) +
         (float10)*(int *)((int)param_1 + 0x798);
}

