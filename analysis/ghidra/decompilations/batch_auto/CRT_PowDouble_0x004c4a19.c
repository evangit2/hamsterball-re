
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 __cdecl CRT_PowDouble(double param_1,int param_2,uint param_3)

{
  byte bVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  int iVar8;
  ushort uVar9;
  double dVar10;
  double dVar11;
  double dVar12;
  ushort uVar18;
  ushort in_XMM1_Wh;
  undefined1 auVar13 [16];
  undefined1 auVar14 [16];
  undefined1 auVar15 [16];
  undefined1 auVar16 [16];
  undefined1 auVar17 [16];
  longlong lVar19;
  ulonglong in_XMM2_Qb;
  undefined1 auVar20 [16];
  longlong lVar23;
  undefined1 auVar21 [16];
  undefined1 auVar22 [16];
  undefined1 in_XMM3 [16];
  undefined1 auVar24 [16];
  undefined1 auVar25 [16];
  ulonglong uVar26;
  double dVar27;
  double dVar28;
  double dVar29;
  double dVar30;
  double dVar31;
  double dVar32;
  double dVar33;
  
  dVar10 = (double)((ulonglong)param_1 >> 0x2c);
  uVar9 = (ushort)((ulonglong)param_1 >> 0x30);
  uVar2 = (SUB82(dVar10,0) & 0xff) + 1 & 0x1fe;
  dVar32 = (double)((ulonglong)DAT_004ee290 & (ulonglong)param_1 | (ulonglong)DAT_004ee2a0) *
           *(double *)(&DAT_004eaa00 + uVar2 * 4);
  dVar29 = *(double *)(&DAT_004eaa00 + uVar2 * 4);
  dVar11 = *(double *)(&DAT_004eae10 + uVar2 * 8);
  dVar31 = *(double *)(&UNK_004eae18 + uVar2 * 8);
  uVar2 = 0x7fef - uVar9;
  iVar5 = SUB84(param_1,0);
  uVar6 = (uint)((ulonglong)param_1 >> 0x20);
  dVar30 = param_1;
  if ((uVar9 - 0x10 | uVar2) < 0x80000000) {
    uVar2 = 0;
    uVar7 = 0x3fe7f;
LAB_004c4a96:
    uVar3 = ((ushort)((ulonglong)dVar32 >> 0x26) & 0xff) + 1 & 0x1fe;
    dVar27 = (double)((ulonglong)dVar30 & (ulonglong)DAT_004ee290 | (ulonglong)DAT_004ee2a0);
    dVar12 = (double)(DAT_004ee2e8 & (ulonglong)dVar27);
    auVar25._8_8_ = in_XMM3._8_8_ >> 0x1f;
    dVar27 = dVar27 - dVar12;
    uVar4 = ((ushort)((ulonglong)(dVar32 * *(double *)(&DAT_004eb620 + uVar3 * 4)) >> 0x1f) & 0x1ff)
            + 1 & 0x3fe;
    dVar28 = dVar29 * *(double *)(&DAT_004eb620 + uVar3 * 4) *
             *(double *)(&DAT_004ec240 + uVar4 * 4);
    dVar32 = dVar32 * *(double *)(&DAT_004eb620 + uVar3 * 4) *
             *(double *)(&DAT_004ec240 + uVar4 * 4);
    dVar30 = dVar11 + *(double *)(&DAT_004eba30 + uVar3 * 8) +
             (double)(int)((longlong)dVar10 - (ulonglong)uVar7 >> 8) +
             *(double *)(&DAT_004eca50 + uVar4 * 8);
    dVar10 = dVar31 + *(double *)(&DAT_004eba38 + uVar3 * 8) +
             *(double *)(&DAT_004eca58 + uVar4 * 8);
    dVar11 = (double)(DAT_004ee2e8 & (ulonglong)dVar28);
    dVar28 = dVar28 - dVar11;
    dVar33 = DAT_004ee2f0 + dVar32;
    dVar31 = dVar30 + dVar33;
    dVar29 = (double)CONCAT44(param_3,param_2);
    uVar18 = (ushort)(param_3 >> 0x10);
    dVar30 = dVar30 - dVar31;
    auVar25._0_8_ = DAT_004ee2e8;
    uVar3 = (uint)(ushort)((ulonglong)dVar31 >> 0x30);
    dVar11 = (((dVar32 - dVar11 * dVar12) - dVar12 * dVar28) - dVar11 * dVar27) - dVar27 * dVar28;
    dVar12 = dVar33 - dVar11;
    dVar32 = dVar31 - dVar11;
    uVar7 = uVar18 & 0x7ff0;
    if (uVar7 < 0x7ff0) {
      iVar5 = (uVar7 - 0x3ff0) + (uVar3 & 0x7ff0);
      if ((0x40a0U - iVar5 | iVar5 - 0x3c70U) < 0x80000000) {
LAB_004c4be8:
        dVar27 = (double)(DAT_004ee2e8 & (ulonglong)dVar29);
        dVar28 = (double)(DAT_004ee2e8 & (ulonglong)dVar32);
        dVar11 = (dVar30 + dVar33) - (dVar11 - (dVar31 - dVar32));
        dVar31 = dVar27 * dVar28 * 128.0;
        uVar6 = (uint)ROUND(dVar31);
        dVar29 = dVar27 * (dVar32 - dVar28) + dVar28 * (dVar29 - dVar27) +
                 (dVar32 - dVar28) * (dVar29 - dVar27);
        if (0 < (int)(0x1ff7f - uVar6 | uVar6 + 0x1e1ff)) {
          iVar5 = (uVar6 & 0x7f) * 0x10;
          auVar13._0_8_ = (double)((ulonglong)((uVar2 + uVar6 & 0xffffff80) + 0x1ff80) << 0x2d);
          auVar13._8_4_ = 0;
          auVar13._12_4_ = (int)((ulonglong)auVar13._0_8_ >> 0x20);
          dVar30 = *(double *)(&DAT_004eda80 + iVar5) * auVar13._0_8_;
          dVar29 = (dVar31 - ((dVar31 + _DAT_004ee280) - _DAT_004ee280)) * 0.0078125 +
                   dVar29 + ((_DAT_004eda60 * dVar12 + _DAT_004eda70 * dVar12 * dVar12) *
                             dVar12 * dVar12 +
                             _UNK_004eda68 * dVar12 + _UNK_004eda78 * dVar12 * dVar12 +
                            dVar10 + dVar11) * (double)CONCAT44(param_3,param_2);
          return (float10)(dVar29 * dVar29 * dVar30 *
                           (_DAT_004ee2d0 + _DAT_004ee2c0 * dVar29) * dVar29 * dVar29 +
                           *(double *)(&UNK_004eda88 + iVar5) * auVar13._8_8_ +
                           (_UNK_004ee2d8 + _UNK_004ee2c8 * dVar29) * dVar29 * dVar29 * dVar30 +
                           DAT_004ee2e0 * dVar29 * dVar30 + dVar30);
        }
        if ((int)uVar6 < 1) {
          if ((int)uVar6 < -0x3fdff) {
LAB_004c547a:
            dVar29 = (double)((ulonglong)(DAT_004ee318 * DAT_004ee318) | (ulonglong)uVar2 << 0x2d);
            iVar8 = 0x19;
            goto LAB_004c5221;
          }
          uVar2 = uVar2 + 0x80;
          uVar7 = (uVar6 & 0xffffff80) + 0x3fe80;
          uVar9 = 0;
        }
        else {
          if (0x3ffff < uVar6) goto LAB_004c549d;
          uVar2 = uVar2 + 0x3ff00;
          uVar7 = uVar6 - 0x80 & 0xffffff80;
          uVar9 = 0x3ff0;
        }
        iVar5 = (uVar6 & 0x7f) * 0x10;
        uVar6 = ((int)-(uVar7 - 0x1ff80) >> 7) + 2;
        auVar17._0_8_ = (double)((ulonglong)uVar7 << 0x2d);
        auVar17._8_4_ = 0;
        auVar17._12_4_ = (int)((ulonglong)auVar17._0_8_ >> 0x20);
        dVar32 = *(double *)(&DAT_004eda80 + iVar5) * auVar17._0_8_;
        dVar11 = (dVar31 - ((dVar31 + _DAT_004ee280) - _DAT_004ee280)) * 0.0078125 +
                 dVar29 + ((_DAT_004eda60 * dVar12 + _DAT_004eda70 * dVar12 * dVar12) *
                           dVar12 * dVar12 +
                           _UNK_004eda68 * dVar12 + _UNK_004eda78 * dVar12 * dVar12 +
                          dVar10 + dVar11) * (double)CONCAT44(param_3,param_2);
        dVar29 = (double)((ulonglong)uVar2 << 0x2d);
        uVar26 = (ulonglong)(uVar6 + (uVar6 & 0x20));
        dVar31 = (double)(-1L << uVar26 & (ulonglong)dVar32);
        dVar11 = dVar11 * dVar11 * dVar32 *
                 (_DAT_004ee2d0 + _DAT_004ee2c0 * dVar11) * dVar11 * dVar11 +
                 *(double *)(&UNK_004eda88 + iVar5) * auVar17._8_8_ +
                 (_UNK_004ee2d8 + _UNK_004ee2c8 * dVar11) * dVar11 * dVar11 * dVar32 +
                 DAT_004ee2e0 * dVar11 * dVar32;
        dVar30 = (double)((ulonglong)(dVar31 + dVar11) & -1L << uVar26);
        dVar11 = dVar11 + (dVar31 - dVar30) + (dVar32 - dVar31);
        if ((int)(uVar7 - 0x1ff80) < 1) {
          dVar29 = dVar11 * dVar29 + dVar30 * dVar29;
          dVar29 = dVar29 + (double)((ulonglong)uVar9 << 0x30) * dVar29;
          iVar8 = 0x18;
          if ((((ushort)((ulonglong)dVar29 >> 0x30) & 0x7ff0) != 0x7ff0) &&
             (iVar8 = 0x19, ((ulonglong)dVar29 & 0x7ff0000000000000) != 0)) {
            return (float10)dVar29;
          }
        }
        else {
          dVar29 = (dVar11 + dVar30) * dVar29;
          dVar29 = dVar29 + (double)((ulonglong)uVar9 << 0x30) * dVar29;
          iVar8 = 0x18;
          if ((((ushort)((ulonglong)dVar29 >> 0x30) & 0x7ff0) != 0x7ff0) &&
             (iVar8 = 0x19, ((ulonglong)dVar29 & 0x7ff0000000000000) != 0)) {
            return (float10)dVar29;
          }
        }
      }
      else {
        dVar31 = dVar29 * dVar32;
        uVar6 = (ushort)((ulonglong)dVar31 >> 0x30) & 0x7ff0;
        uVar7 = uVar6 - 0x3c70;
        if ((0x40a0 - uVar6 | uVar7) < 0x80000000) goto LAB_004c4be8;
        if (0x7fffffff < uVar7) {
          return (float10)(double)((ulonglong)(uVar2 | 0x1ff80) << 0x2d);
        }
        if (((uVar18 ^ (uVar9 & 0x7ff0) + 0xc010) & 0x8000) != 0) goto LAB_004c547a;
LAB_004c549d:
        iVar8 = 0x18;
        dVar29 = DAT_004ee300;
        if (uVar2 != 0) {
          dVar29 = DAT_004ee308;
        }
      }
      goto LAB_004c5221;
    }
    dVar29 = (double)CONCAT44(param_3,param_2);
    auVar20._8_8_ = in_XMM2_Qb;
    auVar20._0_8_ = dVar29;
    dVar11 = (double)CONCAT44(uVar2,uVar3);
    if (iVar5 != 0) goto LAB_004c512c;
    if (uVar6 == 0x3ff00000) goto LAB_004c520a;
    dVar11 = (double)CONCAT44(uVar2,uVar6);
    if (uVar6 != 0xbff00000) goto LAB_004c512c;
    auVar15._0_4_ = -(uint)((int)((ulonglong)DAT_004ee290 & (ulonglong)dVar29) == 0);
    auVar15._4_4_ = -(uint)((int)(((ulonglong)DAT_004ee290 & (ulonglong)dVar29) >> 0x20) == 0);
    auVar15._8_4_ = -(uint)((int)(auVar25._8_8_ & in_XMM2_Qb) == 0);
    auVar15._12_4_ = -(uint)((int)((auVar25._8_8_ & in_XMM2_Qb) >> 0x20) == 0);
    if ((ushort)((ushort)(SUB161(auVar15 >> 7,0) & 1) | (ushort)(SUB161(auVar15 >> 0xf,0) & 1) << 1
                 | (ushort)(SUB161(auVar15 >> 0x17,0) & 1) << 2 |
                 (ushort)(SUB161(auVar15 >> 0x1f,0) & 1) << 3 |
                 (ushort)(SUB161(auVar15 >> 0x27,0) & 1) << 4 |
                 (ushort)(SUB161(auVar15 >> 0x2f,0) & 1) << 5 |
                 (ushort)(SUB161(auVar15 >> 0x37,0) & 1) << 6 |
                 (ushort)(SUB161(auVar15 >> 0x3f,0) & 1) << 7 |
                 (ushort)(SUB161(auVar15 >> 0x47,0) & 1) << 8 |
                 (ushort)(SUB161(auVar15 >> 0x4f,0) & 1) << 9 |
                 (ushort)(SUB161(auVar15 >> 0x57,0) & 1) << 10 |
                 (ushort)(SUB161(auVar15 >> 0x5f,0) & 1) << 0xb |
                 (ushort)((byte)(auVar15._12_4_ >> 7) & 1) << 0xc |
                 (ushort)((byte)(auVar15._12_4_ >> 0xf) & 1) << 0xd |
                 (ushort)((byte)(auVar15._12_4_ >> 0x17) & 1) << 0xe |
                (ushort)(byte)(auVar15._12_4_ >> 0x1f) << 0xf) == 0xff) {
      return (float10)1;
    }
  }
  else {
    auVar20._4_4_ = param_3;
    auVar20._0_4_ = param_2;
    auVar25._8_8_ = in_XMM3._8_8_;
    auVar25._0_8_ = CONCAT44(DAT_004ee2b0._4_4_,(undefined4)DAT_004ee2b0);
    auVar20._8_8_ = in_XMM2_Qb;
    uVar7 = param_3 & DAT_004ee2b0._4_4_;
    if (uVar7 < 0x7ff00000) {
      if (param_2 == 0 && uVar7 == 0) {
        iVar8 = 0x1a;
        dVar29 = DAT_004ee2a0;
        if (iVar5 != 0 || ((ulonglong)param_1 & 0x7fffffff00000000) != 0) {
          iVar8 = 0x1d;
          if (((uVar6 & 0x7fffffff) < 0x7ff00001) &&
             (((uVar6 & 0x7fffffff) < 0x7ff00000 || (iVar5 == 0)))) {
            return (float10)DAT_004ee2a0;
          }
        }
        goto LAB_004c5221;
      }
      if ((int)uVar2 < 0) {
        auVar24._0_8_ = CONCAT44(DAT_004ee2b0._4_4_,(undefined4)DAT_004ee2b0) << 0x34;
        auVar24._8_8_ = auVar25._8_8_ << 0x34;
        iVar8 = (uVar7 >> 0x14) - 0x3f3;
        auVar25 = (undefined1  [16])0x0;
        uVar18 = (in_XMM1_Wh & in_XMM3._14_2_) >> 4;
        lVar19 = SUB168(auVar20 | auVar24,0) <<
                 (ulonglong)
                 CONCAT22((ushort)(-1 < iVar8) * (short)((uint)iVar8 >> 0x10),
                          (ushort)(-1 < (short)iVar8) * (short)iVar8);
        lVar23 = SUB168(auVar20 | auVar24,8) << (ulonglong)(ushort)((-1 < (short)uVar18) * uVar18);
        auVar21._0_4_ = -(uint)((int)lVar19 == 0);
        auVar21._4_4_ = -(uint)((int)((ulonglong)lVar19 >> 0x20) == 0);
        auVar21._8_4_ = -(uint)((int)lVar23 == 0);
        auVar21._12_4_ = -(uint)((int)((ulonglong)lVar23 >> 0x20) == 0);
        bVar1 = SUB161(auVar21 >> 7,0) & 1 | (SUB161(auVar21 >> 0xf,0) & 1) << 1 |
                (SUB161(auVar21 >> 0x17,0) & 1) << 2 | (SUB161(auVar21 >> 0x1f,0) & 1) << 3 |
                (SUB161(auVar21 >> 0x27,0) & 1) << 4 | (SUB161(auVar21 >> 0x2f,0) & 1) << 5 |
                (SUB161(auVar21 >> 0x37,0) & 1) << 6 | SUB161(auVar21 >> 0x3f,0) << 7;
        uVar7 = 0x7fef - uVar2 & 0x7fff;
        if (0x7fef < uVar7) {
          auVar14._0_4_ = -(uint)((int)((ulonglong)DAT_004ee290 & (ulonglong)param_1) == 0);
          auVar14._4_4_ =
               -(uint)((int)(((ulonglong)DAT_004ee290 & (ulonglong)param_1) >> 0x20) == 0);
          auVar14._8_4_ = 0xffffffff;
          auVar14._12_4_ = 0xffffffff;
          if ((byte)(SUB161(auVar14 >> 7,0) & 1 | (SUB161(auVar14 >> 0xf,0) & 1) << 1 |
                     (SUB161(auVar14 >> 0x17,0) & 1) << 2 | (SUB161(auVar14 >> 0x1f,0) & 1) << 3 |
                     (SUB161(auVar14 >> 0x27,0) & 1) << 4 | (SUB161(auVar14 >> 0x2f,0) & 1) << 5 |
                     (SUB161(auVar14 >> 0x37,0) & 1) << 6 | SUB161(auVar14 >> 0x3f,0) << 7) == 0xff)
          {
            if (((ulonglong)param_1 & 0x8000000000000000) != 0) {
              if ((bVar1 != 0xff) ||
                 (lVar19 = CONCAT44(param_3,param_2) <<
                           (ulonglong)(((param_3 & DAT_004ee2b0._4_4_) >> 0x14) - 0x3f4),
                 lVar23 = auVar21._8_8_ << (ulonglong)(_UNK_004ee2bc >> 0x14),
                 auVar22._0_4_ = -(uint)((int)lVar19 == 0),
                 auVar22._4_4_ = -(uint)((int)((ulonglong)lVar19 >> 0x20) == 0),
                 auVar22._8_4_ = -(uint)((int)lVar23 == 0),
                 auVar22._12_4_ = -(uint)((int)((ulonglong)lVar23 >> 0x20) == 0),
                 (byte)(SUB161(auVar22 >> 7,0) & 1 | (SUB161(auVar22 >> 0xf,0) & 1) << 1 |
                        (SUB161(auVar22 >> 0x17,0) & 1) << 2 | (SUB161(auVar22 >> 0x1f,0) & 1) << 3
                        | (SUB161(auVar22 >> 0x27,0) & 1) << 4 |
                        (SUB161(auVar22 >> 0x2f,0) & 1) << 5 | (SUB161(auVar22 >> 0x37,0) & 1) << 6
                       | SUB161(auVar22 >> 0x3f,0) << 7) == 0xff)) {
                if ((param_3 & 0x80000000) != 0) {
                  return (float10)0;
                }
                goto LAB_004c519e;
              }
              if ((param_3 & 0x80000000) == 0) {
                return (float10)DAT_004ee308;
              }
              goto LAB_004c5039;
            }
            if ((param_3 & 0x80000000) != 0) {
              return (float10)0;
            }
            goto LAB_004c519e;
          }
          goto LAB_004c507f;
        }
        if (bVar1 == 0xff) {
          auVar25._8_8_ = 0;
          auVar25._0_8_ = DAT_004ee310;
          lVar19 = CONCAT44(param_3,param_2) <<
                   (ulonglong)(((param_3 & DAT_004ee2b0._4_4_) >> 0x14) - 0x3f4);
          auVar20._0_4_ = -(uint)((int)lVar19 == SUB84(DAT_004ee310,0));
          auVar20._4_4_ =
               -(uint)((int)((ulonglong)lVar19 >> 0x20) == (int)((ulonglong)DAT_004ee310 >> 0x20));
          auVar20._8_4_ = -(uint)(auVar21._8_4_ == 0);
          auVar20._12_4_ = -(uint)(auVar21._12_4_ == 0);
          uVar2 = (ushort)((ushort)(SUB161(auVar20 >> 7,0) & 1) |
                           (ushort)(SUB161(auVar20 >> 0xf,0) & 1) << 1 |
                           (ushort)(SUB161(auVar20 >> 0x17,0) & 1) << 2 |
                           (ushort)(SUB161(auVar20 >> 0x1f,0) & 1) << 3 |
                           (ushort)(SUB161(auVar20 >> 0x27,0) & 1) << 4 |
                           (ushort)(SUB161(auVar20 >> 0x2f,0) & 1) << 5 |
                           (ushort)(SUB161(auVar20 >> 0x37,0) & 1) << 6 |
                          (ushort)(SUB161(auVar20 >> 0x3f,0) & 1) << 7) + 0x3ff01 & 0x40000;
          if (0xf < uVar7) {
            uVar7 = 0xbfe7f;
            in_XMM3._8_8_ = 0;
            in_XMM3._0_8_ = DAT_004ee290;
            in_XMM2_Qb = auVar20._8_8_;
            goto LAB_004c4a96;
          }
          goto LAB_004c4eb4;
        }
        dVar29 = (double)((ulonglong)param_1 >> 0x20);
        in_XMM2_Qb = auVar21._8_8_ >> 0x20;
        uVar2 = 0;
        uVar7 = 0;
        if (iVar5 != 0 || ((ulonglong)param_1 & 0x7fffffff00000000) != 0) {
          iVar8 = 0x1c;
          dVar29 = DAT_004ee320;
          goto LAB_004c5221;
        }
LAB_004c4f3a:
        in_XMM3 = auVar25;
        dVar30 = dVar10;
        if ((uVar7 & 0x7fffffff) == 0) {
          if ((param_3 & 0x80000000) == 0) {
            if ((uVar7 & uVar2 << 0xd) == 0) {
              return (float10)0;
            }
LAB_004c5039:
            return (float10)DAT_004ee310;
          }
          dVar29 = (double)((ulonglong)(uVar7 & uVar2 << 0xd | 0x7ff00000) << 0x20);
          iVar8 = 0x1b;
          goto LAB_004c5221;
        }
      }
      else {
        uVar2 = 0;
LAB_004c4eb4:
        in_XMM2_Qb = auVar20._8_8_;
        dVar10 = param_1 * 1.8446744073709552e+19;
        uVar7 = uVar6;
        in_XMM3 = auVar25;
        dVar30 = dVar10;
        dVar32 = DAT_004ee290;
        dVar29 = DAT_004ee2a0;
        if (iVar5 == 0) goto LAB_004c4f3a;
      }
      dVar10 = (double)(((ulonglong)dVar30 & CONCAT44(DAT_004ee2b0._4_4_,(undefined4)DAT_004ee2b0))
                       >> 0x2c);
      uVar7 = (SUB82(dVar10,0) & 0xff) + 1 & 0x1fe;
      dVar32 = (double)((ulonglong)dVar32 & (ulonglong)dVar30 | (ulonglong)dVar29) *
               *(double *)(&DAT_004eaa00 + uVar7 * 4);
      dVar29 = *(double *)(&DAT_004eaa00 + uVar7 * 4);
      dVar11 = *(double *)(&DAT_004eae10 + uVar7 * 8);
      dVar31 = *(double *)(&UNK_004eae18 + uVar7 * 8);
      uVar7 = 0x43e7f;
      goto LAB_004c4a96;
    }
    dVar11 = param_1;
    if ((0x7fefffff < (uVar6 & 0x7fffffff)) && ((0x7ff00000 < (uVar6 & 0x7fffffff) || (iVar5 != 0)))
       ) {
LAB_004c507f:
      dVar29 = param_1 + param_1;
      iVar8 = 0x3ee;
      goto LAB_004c5221;
    }
LAB_004c512c:
    dVar29 = auVar20._0_8_;
    uVar26 = auVar25._8_8_ & auVar20._8_8_;
    auVar16._0_4_ = -(uint)((int)((ulonglong)DAT_004ee290 & (ulonglong)dVar29) == 0);
    auVar16._4_4_ = -(uint)((int)(((ulonglong)DAT_004ee290 & (ulonglong)dVar29) >> 0x20) == 0);
    auVar16._8_4_ = -(uint)((int)uVar26 == 0);
    auVar16._12_4_ = -(uint)((int)(uVar26 >> 0x20) == 0);
    if ((byte)(SUB161(auVar16 >> 7,0) & 1 | (SUB161(auVar16 >> 0xf,0) & 1) << 1 |
               (SUB161(auVar16 >> 0x17,0) & 1) << 2 | (SUB161(auVar16 >> 0x1f,0) & 1) << 3 |
               (SUB161(auVar16 >> 0x27,0) & 1) << 4 | (SUB161(auVar16 >> 0x2f,0) & 1) << 5 |
               (SUB161(auVar16 >> 0x37,0) & 1) << 6 | SUB161(auVar16 >> 0x3f,0) << 7) == 0xff) {
      if (dVar11 != -1.0) {
        if ((auVar20._6_2_ & 0x8000) == 0) {
          if ((uVar9 & 0x7ff0) < 0x3ff0) {
            return (float10)0;
          }
        }
        else if (0x3fef < (uVar9 & 0x7ff0)) {
          return (float10)0;
        }
LAB_004c519e:
        return (float10)DAT_004ee300;
      }
LAB_004c520a:
      dVar29 = (double)CONCAT44(param_3,param_2) + DAT_004ee320;
      iVar8 = 0x1c;
      goto LAB_004c5221;
    }
  }
  dVar29 = dVar29 + dVar29;
  iVar8 = 0x3ee;
LAB_004c5221:
  FPU_WriteMathError(&param_1,(undefined8 *)&param_2,(undefined8 *)&stack0xfffffff4,iVar8);
  return (float10)dVar29;
}

