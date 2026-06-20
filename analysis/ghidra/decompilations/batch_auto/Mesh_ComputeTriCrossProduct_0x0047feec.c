
float10 __cdecl
Mesh_ComputeTriCrossProduct
          (int param_1,int param_2,int param_3,int param_4,int param_5,int param_6,int param_7,
          int param_8)

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
  float fVar18;
  float fVar19;
  float fVar20;
  float fVar21;
  float fVar22;
  float fVar23;
  float fVar24;
  float *pfVar25;
  float *pfVar26;
  float *pfVar27;
  
  pfVar25 = (float *)(param_1 * param_8 + param_7);
  pfVar26 = (float *)(param_2 * param_8 + param_7);
  fVar1 = *pfVar25;
  pfVar27 = (float *)(param_3 * param_8 + param_7);
  fVar2 = *pfVar26;
  fVar3 = pfVar25[1];
  fVar4 = pfVar26[1];
  fVar5 = pfVar25[2];
  fVar6 = pfVar26[2];
  fVar7 = *pfVar25;
  fVar8 = *pfVar27;
  fVar9 = pfVar25[1];
  pfVar26 = (float *)(param_4 * param_8 + param_7);
  fVar10 = pfVar27[1];
  fVar11 = pfVar25[2];
  fVar12 = pfVar27[2];
  pfVar25 = (float *)(param_5 * param_8 + param_7);
  pfVar27 = (float *)(param_6 * param_8 + param_7);
  Graphics_InitShaderDispatch();
  fVar13 = *pfVar26;
  fVar14 = *pfVar25;
  fVar15 = pfVar26[1];
  fVar16 = pfVar25[1];
  fVar17 = pfVar26[2];
  fVar18 = pfVar25[2];
  fVar19 = *pfVar26;
  fVar20 = *pfVar27;
  fVar21 = pfVar26[1];
  fVar22 = pfVar27[1];
  fVar23 = pfVar26[2];
  fVar24 = pfVar27[2];
  Graphics_InitShaderDispatch();
  return (float10)((fVar23 - fVar24) * (fVar15 - fVar16) - (fVar21 - fVar22) * (fVar17 - fVar18)) *
         (float10)((fVar11 - fVar12) * (fVar3 - fVar4) - (fVar9 - fVar10) * (fVar5 - fVar6)) +
         (float10)((fVar17 - fVar18) * (fVar19 - fVar20) - (fVar23 - fVar24) * (fVar13 - fVar14)) *
         (float10)((fVar5 - fVar6) * (fVar7 - fVar8) - (fVar11 - fVar12) * (fVar1 - fVar2)) +
         (float10)((fVar21 - fVar22) * (fVar13 - fVar14) - (fVar15 - fVar16) * (fVar19 - fVar20)) *
         (float10)((fVar9 - fVar10) * (fVar1 - fVar2) - (fVar7 - fVar8) * (fVar3 - fVar4));
}

