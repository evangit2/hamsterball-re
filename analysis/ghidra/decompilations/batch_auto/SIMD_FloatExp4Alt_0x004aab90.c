
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl SIMD_FloatExp4Alt(uint *param_1,uint *param_2,uint *param_3)

{
  float fVar1;
  float fVar2;
  uint uVar3;
  float fVar4;
  float fVar5;
  uint uVar6;
  float fVar7;
  float fVar8;
  uint uVar9;
  float fVar10;
  float fVar11;
  uint uVar12;
  uint uVar13;
  uint uVar14;
  uint uVar15;
  float fVar16;
  float fVar17;
  uint uVar18;
  uint uVar19;
  uint uVar20;
  uint uVar21;
  uint uVar22;
  uint uVar23;
  uint uVar24;
  float fVar25;
  float fVar26;
  uint uVar27;
  uint uVar28;
  
  fVar16 = (float)(*param_1 & _DAT_004fb0e0);
  fVar17 = (float)(param_1[1] & uRam004fb0e4);
  fVar25 = (float)(param_1[2] & uRam004fb0e8);
  fVar26 = (float)(param_1[3] & uRam004fb0ec);
  uVar13 = param_1[1] & uRam004fb0d4;
  uVar14 = param_1[2] & uRam004fb0d8;
  uVar15 = param_1[3] & uRam004fb0dc;
  fVar1 = fVar16 * _DAT_004fb0c0 + _DAT_004fb070;
  fVar4 = fVar17 * fRam004fb0c4 + fRam004fb074;
  fVar7 = fVar25 * fRam004fb0c8 + fRam004fb078;
  fVar10 = fVar26 * fRam004fb0cc + fRam004fb07c;
  fVar2 = fVar1 - _DAT_004fb070;
  fVar5 = fVar4 - fRam004fb074;
  fVar8 = fVar7 - fRam004fb078;
  fVar11 = fVar10 - fRam004fb07c;
  fVar16 = (((fVar16 - _DAT_004fb0b0 * fVar2) - _DAT_004fb0a0 * fVar2) - _DAT_004fb090 * fVar2) -
           _DAT_004fb080 * fVar2;
  fVar2 = (((fVar17 - fRam004fb0b4 * fVar5) - fRam004fb0a4 * fVar5) - fRam004fb094 * fVar5) -
          fRam004fb084 * fVar5;
  fVar17 = (((fVar25 - fRam004fb0b8 * fVar8) - fRam004fb0a8 * fVar8) - fRam004fb098 * fVar8) -
           fRam004fb088 * fVar8;
  fVar5 = (((fVar26 - fRam004fb0bc * fVar11) - fRam004fb0ac * fVar11) - fRam004fb09c * fVar11) -
          fRam004fb08c * fVar11;
  fVar25 = fVar16 * fVar16;
  fVar8 = fVar2 * fVar2;
  fVar26 = fVar17 * fVar17;
  fVar11 = fVar5 * fVar5;
  uVar18 = (uint)fVar1 & _DAT_004fb040;
  uVar19 = (uint)fVar4 & uRam004fb044;
  uVar21 = (uint)fVar7 & uRam004fb048;
  uVar23 = (uint)fVar10 & uRam004fb04c;
  uVar3 = uVar18 & _DAT_004fb030;
  uVar6 = uVar19 & uRam004fb034;
  uVar9 = uVar21 & uRam004fb038;
  uVar12 = uVar23 & uRam004fb03c;
  uVar20 = uVar19 - uVar6 & uRam004fb024;
  uVar22 = uVar21 - uVar9 & uRam004fb028;
  uVar24 = uVar23 - uVar12 & uRam004fb02c;
  uVar27 = uVar18 + uVar3 & _DAT_004fb020;
  uVar28 = uVar19 + uVar6 & uRam004fb024;
  uVar21 = uVar21 + uVar9 & uRam004fb028;
  uVar23 = uVar23 + uVar12 & uRam004fb02c;
  uVar19 = -(uint)((float)(uVar3 << 0x1e) != 0.0);
  uVar6 = -(uint)((float)(uVar6 << 0x1e) != 0.0);
  uVar9 = -(uint)((float)(uVar9 << 0x1e) != 0.0);
  uVar12 = -(uint)((float)(uVar12 << 0x1e) != 0.0);
  fVar16 = (((_DAT_004fb060 * fVar25 + _DAT_004fb010) * fVar25 + _DAT_004faff0) * fVar25 +
           _DAT_004fafd0) * fVar16;
  fVar2 = (((fRam004fb064 * fVar8 + fRam004fb014) * fVar8 + fRam004faff4) * fVar8 + fRam004fafd4) *
          fVar2;
  fVar17 = (((fRam004fb068 * fVar26 + fRam004fb018) * fVar26 + fRam004faff8) * fVar26 + fRam004fafd8
           ) * fVar17;
  fVar5 = (((fRam004fb06c * fVar11 + fRam004fb01c) * fVar11 + fRam004faffc) * fVar11 + fRam004fafdc)
          * fVar5;
  fVar1 = (((_DAT_004fb050 * fVar25 + _DAT_004fb000) * fVar25 + _DAT_004fafe0) * fVar25 +
          _DAT_004fafc0) * fVar25 + _DAT_004fafb0;
  fVar4 = (((fRam004fb054 * fVar8 + fRam004fb004) * fVar8 + fRam004fafe4) * fVar8 + fRam004fafc4) *
          fVar8 + fRam004fafb4;
  fVar25 = (((fRam004fb058 * fVar26 + fRam004fb008) * fVar26 + fRam004fafe8) * fVar26 + fRam004fafc8
           ) * fVar26 + fRam004fafb8;
  fVar7 = (((fRam004fb05c * fVar11 + fRam004fb00c) * fVar11 + fRam004fafec) * fVar11 + fRam004fafcc)
          * fVar11 + fRam004fafbc;
  *param_2 = (~uVar19 & (uint)fVar16 | uVar19 & (uint)fVar1) ^
             (uVar18 - uVar3 & _DAT_004fb020) << 0x1e ^ *param_1 & _DAT_004fb0d0;
  param_2[1] = (~uVar6 & (uint)fVar2 | uVar6 & (uint)fVar4) ^ uVar20 << 0x1e ^ uVar13;
  param_2[2] = (~uVar9 & (uint)fVar17 | uVar9 & (uint)fVar25) ^ uVar22 << 0x1e ^ uVar14;
  param_2[3] = (~uVar12 & (uint)fVar5 | uVar12 & (uint)fVar7) ^ uVar24 << 0x1e ^ uVar15;
  *param_3 = (uVar19 & (uint)fVar16 | ~uVar19 & (uint)fVar1) ^ uVar27 << 0x1e;
  param_3[1] = (uVar6 & (uint)fVar2 | ~uVar6 & (uint)fVar4) ^ uVar28 << 0x1e;
  param_3[2] = (uVar9 & (uint)fVar17 | ~uVar9 & (uint)fVar25) ^ uVar21 << 0x1e;
  param_3[3] = (uVar12 & (uint)fVar5 | ~uVar12 & (uint)fVar7) ^ uVar23 << 0x1e;
  return;
}

