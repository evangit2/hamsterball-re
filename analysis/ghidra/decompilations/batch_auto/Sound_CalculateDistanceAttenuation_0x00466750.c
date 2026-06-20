
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* Sound_CalculateDistanceAttenuation(this, x, y, z): 3D audio rolloff. Find nearest listener,
   return 0.0-1.0 volume based on linear interpolation between min_rolloff (this+0x914) and
   max_rolloff (this+0x918). See decomp_ball_input_audio.c */

float10 __thiscall
Sound_CalculateDistanceAttenuation(void *param_1,float param_2,float param_3,float param_4)

{
  float *pfVar1;
  int iVar2;
  float10 fVar3;
  float10 fVar4;
  
  iVar2 = *(int *)((int)param_1 + 0x850);
  fVar3 = (float10)_DAT_004d9120;
  if (0 < iVar2) {
    pfVar1 = (float *)((int)param_1 + 0x854);
    do {
      if (SQRT((float10)(param_2 - *pfVar1) * (float10)(param_2 - *pfVar1) +
               (float10)(param_3 - pfVar1[1]) * (float10)(param_3 - pfVar1[1]) +
               ((float10)param_4 - (float10)pfVar1[2]) * ((float10)param_4 - (float10)pfVar1[2])) <=
          fVar3) {
        fVar3 = SQRT((float10)(param_2 - *pfVar1) * (float10)(param_2 - *pfVar1) +
                     (float10)(param_3 - pfVar1[1]) * (float10)(param_3 - pfVar1[1]) +
                     ((float10)param_4 - (float10)pfVar1[2]) *
                     ((float10)param_4 - (float10)pfVar1[2]));
      }
      pfVar1 = pfVar1 + 3;
      iVar2 = iVar2 + -1;
    } while (iVar2 != 0);
  }
  if ((float10)*(float *)((int)param_1 + 0x914) <= fVar3) {
    if (fVar3 <= (float10)*(float *)((int)param_1 + 0x918)) {
      fVar4 = (float10)*(float *)((int)param_1 + 0x918) - (float10)*(float *)((int)param_1 + 0x914);
      if (fVar4 == (float10)_DAT_004cf368) {
        fVar4 = (float10)_DAT_004cf310;
      }
      return (float10)_DAT_004cf310 - (fVar3 - (float10)*(float *)((int)param_1 + 0x914)) / fVar4;
    }
    return (float10)_DAT_004cf368;
  }
  return (float10)_DAT_004cf310;
}

