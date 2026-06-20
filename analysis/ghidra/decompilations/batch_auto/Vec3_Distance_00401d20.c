/*
 * Function: Vec3_Distance
 * Address: 0x00401d20
 * Signature: float10 Vec3_Distance(float param_1,float param_2,float param_3,float param_4,float param_5,
                     float param_6)
 *
 * Patterns: none identified. Calls: Vec3_Distance, SQRT. Offsets: 0, Lines: 7
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
