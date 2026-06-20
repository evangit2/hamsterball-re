/*
 * Function: Matrix_TransformVec3
 * Address: 0x0040a0b0
 * Signature: void Matrix_TransformVec3(float *param_1,float *param_2)
 *
 * Patterns: matrix math. Calls: Matrix_TransformVec3. Offsets: 0, Lines: 26
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void Matrix_TransformVec3(float *param_1,float *param_2)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  
  fVar1 = param_1[9];
  fVar2 = param_2[2];
  fVar3 = param_1[1];
  fVar4 = *param_2;
  fVar5 = param_1[5];
  fVar6 = param_1[10];
  fVar7 = param_1[2];
  fVar8 = *param_2;
  fVar9 = param_1[6];
  *param_2 = *param_1 * *param_2 + param_1[4] * param_2[1] + param_1[8] * param_2[2];
  param_2[2] = fVar9 * param_2[1] + fVar7 * fVar8 + fVar6 * param_2[2];
  param_2[1] = fVar5 * param_2[1] + fVar3 * fVar4 + fVar1 * fVar2;
  return;
}
