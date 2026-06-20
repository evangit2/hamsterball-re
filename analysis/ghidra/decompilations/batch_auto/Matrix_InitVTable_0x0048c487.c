
void __cdecl Matrix_InitVTable(undefined4 *param_1)

{
  *param_1 = &LAB_0048c02b;
  param_1[1] = &LAB_0048c0c9;
  param_1[2] = &LAB_0048c1b3;
  param_1[3] = Matrix_Multiply4x4_InPlace;
  param_1[4] = Matrix_Multiply4x4;
  param_1[5] = &LAB_0048c094;
  param_1[6] = &LAB_0048c154;
  param_1[7] = Vec3_Normalize;
  return;
}

