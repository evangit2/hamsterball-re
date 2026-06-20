
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl SIMD_FloatExp4(uint *param_1,uint *param_2,uint *param_3)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  uint uVar9;
  float fVar10;
  float fVar11;
  float fVar12;
  uint uVar13;
  float fVar14;
  float fVar15;
  float fVar16;
  uint uVar17;
  float fVar18;
  float fVar19;
  float fVar20;
  uint uVar21;
  uint uVar22;
  uint uVar23;
  uint uVar24;
  uint uVar25;
  uint uVar26;
  uint uVar27;
  uint uVar28;
  
  fVar6 = (float)(*param_1 & _DAT_004faec0);
  fVar10 = (float)(param_1[1] & uRam004faec4);
  fVar14 = (float)(param_1[2] & uRam004faec8);
  fVar18 = (float)(param_1[3] & uRam004faecc);
  uVar22 = param_1[1] & uRam004faeb4;
  uVar23 = param_1[2] & uRam004faeb8;
  uVar24 = param_1[3] & uRam004faebc;
  fVar7 = fVar6 * _DAT_004faea0 + _DAT_004fae50;
  fVar11 = fVar10 * fRam004faea4 + fRam004fae54;
  fVar15 = fVar14 * fRam004faea8 + fRam004fae58;
  fVar19 = fVar18 * fRam004faeac + fRam004fae5c;
  fVar8 = fVar7 - _DAT_004fae50;
  fVar12 = fVar11 - fRam004fae54;
  fVar16 = fVar15 - fRam004fae58;
  fVar20 = fVar19 - fRam004fae5c;
  fVar6 = (((fVar6 - _DAT_004fae90 * fVar8) - _DAT_004fae80 * fVar8) - _DAT_004fae70 * fVar8) -
          _DAT_004fae60 * fVar8;
  fVar8 = (((fVar10 - fRam004fae94 * fVar12) - fRam004fae84 * fVar12) - fRam004fae74 * fVar12) -
          fRam004fae64 * fVar12;
  fVar10 = (((fVar14 - fRam004fae98 * fVar16) - fRam004fae88 * fVar16) - fRam004fae78 * fVar16) -
           fRam004fae68 * fVar16;
  fVar12 = (((fVar18 - fRam004fae9c * fVar20) - fRam004fae8c * fVar20) - fRam004fae7c * fVar20) -
           fRam004fae6c * fVar20;
  fVar14 = fVar6 * fVar6;
  fVar16 = fVar8 * fVar8;
  fVar18 = fVar10 * fVar10;
  fVar20 = fVar12 * fVar12;
  uVar25 = (uint)fVar7 & _DAT_004fae20;
  uVar26 = (uint)fVar11 & uRam004fae24;
  uVar27 = (uint)fVar15 & uRam004fae28;
  uVar28 = (uint)fVar19 & uRam004fae2c;
  uVar9 = uVar25 & _DAT_004fae10;
  uVar13 = uVar26 & uRam004fae14;
  uVar17 = uVar27 & uRam004fae18;
  uVar21 = uVar28 & uRam004fae1c;
  uVar1 = uVar26 - uVar13 & _DAT_004fae00;
  uVar2 = uVar27 - uVar17 & _DAT_004fae00;
  uVar3 = uVar28 - uVar21 & _DAT_004fae00;
  uVar4 = uVar25 + uVar9 & _DAT_004fae00;
  uVar26 = uVar26 + uVar13 & _DAT_004fae00;
  uVar27 = uVar27 + uVar17 & _DAT_004fae00;
  uVar28 = uVar28 + uVar21 & _DAT_004fae00;
  uVar5 = -(uint)((float)(uVar9 << 0x1e) != 0.0);
  uVar13 = -(uint)((float)(uVar13 << 0x1e) != 0.0);
  uVar17 = -(uint)((float)(uVar17 << 0x1e) != 0.0);
  uVar21 = -(uint)((float)(uVar21 << 0x1e) != 0.0);
  fVar6 = (((_DAT_004fae40 * fVar14 + _DAT_004fadf0) * fVar14 + _DAT_004fadd0) * fVar14 +
          _DAT_004fadb0) * fVar6;
  fVar8 = (((fRam004fae44 * fVar16 + fRam004fadf4) * fVar16 + fRam004fadd4) * fVar16 + fRam004fadb4)
          * fVar8;
  fVar10 = (((fRam004fae48 * fVar18 + fRam004fadf8) * fVar18 + fRam004fadd8) * fVar18 + fRam004fadb8
           ) * fVar10;
  fVar12 = (((fRam004fae4c * fVar20 + fRam004fadfc) * fVar20 + fRam004faddc) * fVar20 + fRam004fadbc
           ) * fVar12;
  fVar7 = (((_DAT_004fae30 * fVar14 + _DAT_004fade0) * fVar14 + _DAT_004fadc0) * fVar14 +
          _DAT_004fada0) * fVar14 + _DAT_004fad90;
  fVar11 = (((fRam004fae34 * fVar16 + fRam004fade4) * fVar16 + fRam004fadc4) * fVar16 + fRam004fada4
           ) * fVar16 + fRam004fad94;
  fVar14 = (((fRam004fae38 * fVar18 + fRam004fade8) * fVar18 + fRam004fadc8) * fVar18 + fRam004fada8
           ) * fVar18 + fRam004fad98;
  fVar15 = (((fRam004fae3c * fVar20 + fRam004fadec) * fVar20 + fRam004fadcc) * fVar20 + fRam004fadac
           ) * fVar20 + fRam004fad9c;
  *param_2 = (~uVar5 & (uint)fVar6 | uVar5 & (uint)fVar7) ^
             (uVar25 - uVar9 & _DAT_004fae00) << 0x1e ^ *param_1 & _DAT_004faeb0;
  param_2[1] = (~uVar13 & (uint)fVar8 | uVar13 & (uint)fVar11) ^ uVar1 << 0x1e ^ uVar22;
  param_2[2] = (~uVar17 & (uint)fVar10 | uVar17 & (uint)fVar14) ^ uVar2 << 0x1e ^ uVar23;
  param_2[3] = (~uVar21 & (uint)fVar12 | uVar21 & (uint)fVar15) ^ uVar3 << 0x1e ^ uVar24;
  *param_3 = (uVar5 & (uint)fVar6 | ~uVar5 & (uint)fVar7) ^ uVar4 << 0x1e;
  param_3[1] = (uVar13 & (uint)fVar8 | ~uVar13 & (uint)fVar11) ^ uVar26 << 0x1e;
  param_3[2] = (uVar17 & (uint)fVar10 | ~uVar17 & (uint)fVar14) ^ uVar27 << 0x1e;
  param_3[3] = (uVar21 & (uint)fVar12 | ~uVar21 & (uint)fVar15) ^ uVar28 << 0x1e;
  return;
}

