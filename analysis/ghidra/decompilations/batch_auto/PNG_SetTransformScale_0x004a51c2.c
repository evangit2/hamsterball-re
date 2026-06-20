
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl PNG_SetTransformScale(int param_1,double param_2,double param_3)

{
  if (_DAT_004cf428 < ABS(param_2 * param_3 - _DAT_004cf3c8)) {
    *(byte *)(param_1 + 0x61) = *(byte *)(param_1 + 0x61) | 0x20;
  }
  *(float *)(param_1 + 0x130) = (float)param_3;
  *(float *)(param_1 + 0x134) = (float)param_2;
  return;
}

