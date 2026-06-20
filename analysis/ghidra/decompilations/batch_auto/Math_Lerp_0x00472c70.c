
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 __fastcall Math_Lerp(int param_1)

{
  return (float10)*(float *)(param_1 + 0x24) * (float10)_DAT_004cf3f0 +
         ((float10)*(float *)(param_1 + 0xc) - (float10)*(float *)(param_1 + 0x24)) *
         (float10)*(float *)(param_1 + 0x20);
}

