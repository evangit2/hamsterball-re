/*
 * Function: Vec3_Length
 * Address: 0x00401a60
 * Signature: Vec3_Length(...)
 *
 * Patterns: none identified. Calls: Vec3_Length, SQRT. Offsets: 0, Lines: 13
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 __fastcall Vec3_Length(float *param_1)

{
  float10 fVar1;
  
  fVar1 = (float10)param_1[2] * (float10)param_1[2] +
          (float10)param_1[1] * (float10)param_1[1] + (float10)*param_1 * (float10)*param_1;
  if (fVar1 < (float10)_DAT_004cf368 != (fVar1 == (float10)_DAT_004cf368)) {
    return (float10)_DAT_004cf368;
  }
  return SQRT(fVar1);
}
