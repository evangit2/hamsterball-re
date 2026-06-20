/*
 * Function: Matrix_TransformVec3
 * Address: 0x00401D60
 * Signature: void Matrix_TransformVec3(float *param_1, float *param_2)
 *
 * Description:
 * Transforms a 3D vector by a 4x4 matrix, in-place. The result overwrites the input.
 * This is a standard matrix-vector multiplication: result = M × v
 *
 * Matrix layout (4x4, row-major, stored as 16 floats):
 *   [ M0  M1  M2  M3  ]     [ v.x ]
 *   [ M4  M5  M6  M7  ]  ×  [ v.y ]
 *   [ M8  M9  M10 M11 ]     [ v.z ]
 *   [ M12 M13 M14 M15]       [ 1.0 ]
 *
 * The function accesses:
 *   M[0]=*param_1, M[1]=param_1[1], M[2]=param_1[2], M[3]=param_1[3]
 *   M[4]=param_1[4], M[5]=param_1[5], M[6]=param_1[6], M[7]=param_1[7]
 *   M[8]=param_1[8], M[9]=param_1[9], M[10]=param_1[10], M[11]=param_1[11]
 *   M[12]=param_1[12], M[13]=param_1[13], M[14]=param_1[14]
 *
 * Result components:
 *   v.x = M[0]*v.x + M[4]*v.y + M[8]*v.z  + M[12]
 *   v.y = M[1]*v.x + M[5]*v.y + M[9]*v.z  + M[13]
 *   v.z = M[2]*v.x + M[6]*v.y + M[10]*v.z + M[14]
 *
 * Note: The w-component (M[15]) is not used — this is a 3D affine transform
 * (translation in last column, no perspective divide).
 *
 * Cross-references (15 call sites):
 *   - TowerLevel_Ctor (6 calls) — building tower geometry transforms
 *   - Gear_Render — rendering gear teeth rotations
 *   - FollowBall_Update (2 calls) — AI ball direction transforms
 *   - Scene_Render3DObjects — transforming objects for rendering
 *   - Various collision/spatial functions (5 calls)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void Matrix_TransformVec3(float *param_1,float *param_2)

{
  float fVar1, fVar2, fVar3, fVar4, fVar5, fVar6, fVar7, fVar8, fVar9, fVar10, fVar11;
  
  fVar1 = param_1[9];
  fVar2 = param_2[2];
  fVar3 = param_1[1];
  fVar4 = *param_2;
  fVar5 = param_1[5];
  fVar6 = param_1[0xd];
  fVar7 = param_1[10];
  fVar8 = param_1[2];
  fVar9 = *param_2;
  fVar10 = param_1[6];
  fVar11 = param_1[0xe];
  *param_2 = *param_1 * *param_2 + param_1[4] * param_2[1] + param_1[8] * param_2[2] + param_1[0xc];
  param_2[2] = fVar10 * param_2[1] + fVar8 * fVar9 + fVar7 * param_2[2] + fVar11;
  param_2[1] = fVar5 * param_2[1] + fVar3 * fVar4 + fVar1 * fVar2 + fVar6;
  return;
}
