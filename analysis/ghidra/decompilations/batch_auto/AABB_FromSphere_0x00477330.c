
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl
AABB_FromSphere(float *param_1,float *param_2,float *param_3,float *param_4,float *param_5)

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
  float fVar10;
  float fVar11;
  float fVar12;
  float fVar13;
  float fVar14;
  float fVar15;
  float fVar16;
  float fVar17;
  
  fVar1 = param_1[1];
  fVar2 = param_3[1];
  fVar3 = param_1[2];
  fVar4 = param_3[2];
  fVar5 = *param_1;
  fVar6 = *param_3;
  fVar7 = param_1[1];
  fVar8 = param_3[1];
  fVar9 = param_1[2];
  fVar10 = param_3[2];
  fVar14 = (*param_1 + *param_2) - *param_3;
  fVar12 = (param_2[1] + param_1[1]) - param_3[1];
  fVar13 = (param_2[2] + param_1[2]) - param_3[2];
  fVar16 = *param_1 + *param_2 + *param_3;
  fVar11 = param_2[1] + param_1[1] + param_3[1];
  fVar15 = param_2[2] + param_1[2] + param_3[2];
  fVar17 = *param_1 - *param_3;
  if (fVar14 <= *param_1 - *param_3) {
    fVar17 = fVar14;
  }
  *param_4 = fVar17 - _DAT_004cf524;
  if (fVar1 - fVar2 < fVar12) {
    fVar12 = fVar1 - fVar2;
  }
  param_4[1] = fVar12 - _DAT_004cf524;
  if (fVar3 - fVar4 < fVar13) {
    fVar13 = fVar3 - fVar4;
  }
  param_4[2] = fVar13 - _DAT_004cf524;
  if (fVar16 < fVar5 + fVar6) {
    fVar16 = fVar5 + fVar6;
  }
  *param_5 = fVar16 + _DAT_004cf524;
  if (fVar11 < fVar7 + fVar8) {
    fVar11 = fVar7 + fVar8;
  }
  param_5[1] = fVar11 + _DAT_004cf524;
  if (fVar9 + fVar10 <= fVar15) {
    param_5[2] = fVar15 + _DAT_004cf524;
    return;
  }
  param_5[2] = fVar9 + fVar10 + _DAT_004cf524;
  return;
}

