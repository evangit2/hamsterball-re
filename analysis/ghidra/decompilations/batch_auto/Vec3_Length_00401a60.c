/*
 * Function: Vec3_Length
 * Address: 0x00401A60
 * Signature: float10 __fastcall Vec3_Length(float *param_1)
 *
 * Description:
 * Computes the Euclidean length (magnitude) of a 3D vector.
 * Returns sqrt(x² + y² + z²) where x,y,z are the three float components at
 * param_1[0], param_1[1], param_1[2].
 *
 * Includes a safety check: if the sum of squares is negative (which can happen
 * with denormalized floats or NaN), it returns _DAT_004cf368 (which is 0.0f)
 * instead of attempting SQRT of a negative number.
 *
 * The result is computed in extended precision (float10 = 80-bit x87 FPU).
 *
 * Cross-references (5 call sites):
 *   - 0x43FA34, 0x43FAB0: in FollowBall_Update area (AI ball direction)
 *   - 0x440276: BounceBall_Update area
 *   - 0x466EF9, 0x466F1E: Transform operations
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

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
