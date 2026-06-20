/*
 * Function: Vec3_Distance
 * Address: 0x00401D20
 * Signature: float10 Vec3_Distance(float x1, float y1, float z1, float x2, float y2, float z2)
 *
 * Description:
 * Computes the 3D Euclidean distance between two points:
 *   result = sqrt((x2-x1)² + (y2-y1)² + (z2-z1)²)
 *
 * The function takes 6 float parameters (two sets of XYZ coordinates) and
 * returns the distance in extended precision (float10 = 80-bit x87 FPU).
 * No safety check for negative values (unlike Vec3_Length).
 *
 * Cross-references (2 call sites):
 *   - 0x43FC62: FollowBall_Update area — AI distance checks
 *   - 0x44008B: BounceBall_Update area — collision/bounce distance
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

float10 Vec3_Distance(float param_1,float param_2,float param_3,float param_4,float param_5,
                     float param_6)

{
  return SQRT(((float10)param_4 - (float10)param_1) * ((float10)param_4 - (float10)param_1) +
              ((float10)param_5 - (float10)param_2) * ((float10)param_5 - (float10)param_2) +
              ((float10)param_6 - (float10)param_3) * ((float10)param_6 - (float10)param_3));
}
