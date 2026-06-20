/*
 * Function: Matrix_TransformVec3_v2
 * Address: 0x0040a0b0
 *
 * Description:
 *
Second variant of Matrix_TransformVec3 — transforms a 3D vector by a 4x4 matrix
in-place. This variant does NOT include the translation column (M[12], M[13], M[14])
in the computation, making it a pure rotation/scale transform (no translation).

Cross-refs: 5 calls — Scene_LoadLevel4, MeshNode operations, collision functions.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */


void Matrix_TransformVec3(float *param_1,float *param_2)
{
  float fVar1, fVar2, fVar3, fVar4, fVar5, fVar6, fVar7, fVar8, fVar9;
  fVar1 = param_1[9]; fVar2 = param_2[2]; fVar3 = param_1[1];
  fVar4 = *param_2; fVar5 = param_1[5]; fVar6 = param_1[10];
  fVar7 = param_1[2]; fVar8 = *param_2; fVar9 = param_1[6];
  *param_2 = *param_1 * *param_2 + param_1[4] * param_2[1] + param_1[8] * param_2[2];
  param_2[2] = fVar9 * param_2[1] + fVar7 * fVar8 + fVar6 * param_2[2];
  param_2[1] = fVar5 * param_2[1] + fVar3 * fVar4 + fVar1 * fVar2;
  return;
}

