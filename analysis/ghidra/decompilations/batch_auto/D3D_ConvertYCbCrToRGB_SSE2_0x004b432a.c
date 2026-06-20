
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl
D3D_ConvertYCbCrToRGB_SSE2
          (uint param_1,undefined4 *param_2,undefined4 *param_3,undefined4 *param_4,uint *param_5)

{
  undefined4 uVar1;
  short sVar2;
  short sVar3;
  short sVar4;
  short sVar5;
  ushort uVar6;
  undefined7 uVar7;
  uint uVar8;
  byte bVar9;
  undefined2 uVar10;
  undefined8 uVar11;
  undefined4 uVar12;
  undefined8 uVar13;
  undefined2 uVar15;
  undefined8 uVar14;
  uint uVar16;
  undefined2 uVar21;
  undefined8 uVar17;
  undefined2 uVar20;
  undefined2 uVar22;
  undefined8 uVar18;
  ulonglong uVar19;
  undefined8 uVar23;
  undefined2 uVar25;
  undefined2 uVar26;
  undefined8 uVar24;
  undefined8 uVar27;
  
  uVar8 = param_1 >> 2;
  do {
    uVar12 = *param_2;
    uVar1 = *param_3;
    uVar6 = (ushort)(((uint7)(byte)((uint)uVar12 >> 0x18) << 0x30) >> 0x28);
    uVar7 = CONCAT25(uVar6,CONCAT14((char)((uint)uVar12 >> 0x10),uVar12));
    uVar19 = (ulonglong)
             CONCAT41((int)(CONCAT34((int3)((uint7)uVar7 >> 0x20),uVar12) >> 0x18),
                      (char)((uint)uVar12 >> 8)) & 0xffffffff00ff;
    bVar9 = (byte)uVar12;
    uVar12 = *param_4;
    uVar13 = psubsw((ulonglong)
                    CONCAT52((int5)(CONCAT43((int)(CONCAT34((int3)(CONCAT25((short)(((uint7)(byte)((
                                                  uint)uVar1 >> 0x18) << 0x30) >> 0x28),
                                                  CONCAT14((char)((uint)uVar1 >> 0x10),uVar1)) >>
                                                  0x20),uVar1) >> 0x18),
                                             CONCAT12((char)((uint)uVar1 >> 8),(short)uVar1)) >>
                                   0x10),(short)uVar1) & 0xffffffff00ff00ff,_DAT_004fb270);
    uVar17 = psubsw((ulonglong)
                    CONCAT52(CONCAT41((int)(CONCAT34((int3)(CONCAT25((short)(((uint7)(byte)((uint)
                                                  uVar12 >> 0x18) << 0x30) >> 0x28),
                                                  CONCAT14((char)((uint)uVar12 >> 0x10),uVar12)) >>
                                                  0x20),uVar12) >> 0x18),(char)((uint)uVar12 >> 8)),
                             (ushort)(byte)uVar12) & 0xffffffff00ffffff,_DAT_004fb270);
    uVar20 = (undefined2)((ulonglong)uVar17 >> 0x10);
    uVar15 = (undefined2)((ulonglong)uVar13 >> 0x10);
    uVar14 = pmaddwd(CONCAT44(CONCAT22(uVar20,uVar15),CONCAT22((short)uVar17,(short)uVar13)),
                     _DAT_004fb278);
    uVar11 = psllw(CONCAT62((int6)uVar19,(ushort)bVar9),8);
    uVar25 = (undefined2)((ulonglong)uVar13 >> 0x20);
    uVar21 = (undefined2)((ulonglong)uVar17 >> 0x20);
    uVar26 = (undefined2)((ulonglong)uVar13 >> 0x30);
    uVar22 = (undefined2)((ulonglong)uVar17 >> 0x30);
    uVar24 = pmaddwd(CONCAT26(uVar22,CONCAT24(uVar26,CONCAT22(uVar21,uVar25))),_DAT_004fb278);
    uVar10 = (undefined2)uVar19;
    uVar17 = pmaddwd(CONCAT44(CONCAT22(uVar10,uVar20),(uint)CONCAT12(bVar9,(short)uVar17)),
                     _DAT_004fb280);
    uVar23 = pmaddwd(CONCAT44(CONCAT22(uVar10,uVar15),(uint)CONCAT12(bVar9,(short)uVar13)),
                     _DAT_004fb288);
    uVar10 = (undefined2)((uint7)uVar7 >> 0x20);
    uVar6 = uVar6 >> 8;
    uVar27 = pmaddwd(CONCAT26(uVar6,CONCAT24(uVar26,CONCAT22(uVar10,uVar25))),_DAT_004fb288);
    uVar13 = pmaddwd(CONCAT26(uVar6,CONCAT24(uVar22,CONCAT22(uVar10,uVar21))),_DAT_004fb280);
    uVar18 = packssdw(CONCAT44((int)((int)uVar14 + (uint)(ushort)uVar11) >> 8,(int)uVar17 >> 8),
                      CONCAT44(DAT_005352a0,(int)uVar23 >> 8));
    uVar17 = packssdw(CONCAT44((int)((int)((ulonglong)uVar14 >> 0x20) +
                                    (uint)(ushort)((ulonglong)uVar11 >> 0x10)) >> 8,
                               (int)((longlong)uVar17 >> 0x28)),
                      CONCAT44(DAT_005352a4,(int)((longlong)uVar23 >> 0x28)));
    sVar2 = (short)uVar18;
    sVar3 = (short)((ulonglong)uVar18 >> 0x10);
    sVar4 = (short)((ulonglong)uVar18 >> 0x20);
    sVar5 = (short)((ulonglong)uVar18 >> 0x30);
    uVar16 = CONCAT13((0 < sVar5) * (sVar5 < 0x100) * (char)((ulonglong)uVar18 >> 0x30) -
                      (0xff < sVar5),
                      CONCAT12((0 < sVar4) * (sVar4 < 0x100) * (char)((ulonglong)uVar18 >> 0x20) -
                               (0xff < sVar4),
                               CONCAT11((0 < sVar3) * (sVar3 < 0x100) *
                                        (char)((ulonglong)uVar18 >> 0x10) - (0xff < sVar3),
                                        (0 < sVar2) * (sVar2 < 0x100) * (char)uVar18 -
                                        (0xff < sVar2))));
    sVar2 = (short)uVar17;
    sVar3 = (short)((ulonglong)uVar17 >> 0x10);
    sVar4 = (short)((ulonglong)uVar17 >> 0x20);
    uVar14 = packssdw(CONCAT44((int)((int)uVar24 + (uint)(ushort)((ulonglong)uVar11 >> 0x20)) >> 8,
                               (int)uVar13 >> 8),CONCAT44(DAT_005352a0,(int)uVar27 >> 8));
    uVar19 = CONCAT16((0 < sVar4) * (sVar4 < 0x100) * (char)((ulonglong)uVar17 >> 0x20) -
                      (0xff < sVar4),
                      CONCAT15((0 < sVar3) * (sVar3 < 0x100) * (char)((ulonglong)uVar17 >> 0x10) -
                               (0xff < sVar3),
                               CONCAT14((0 < sVar2) * (sVar2 < 0x100) * (char)uVar17 -
                                        (0xff < sVar2),uVar16))) & _DAT_004fb290;
    uVar11 = packssdw(CONCAT44((int)((int)((ulonglong)uVar24 >> 0x20) +
                                    (uint)(ushort)((ulonglong)uVar11 >> 0x30)) >> 8,
                               (int)((longlong)uVar13 >> 0x28)),
                      CONCAT44(DAT_005352a4,(int)((longlong)uVar27 >> 0x28)));
    sVar2 = (short)uVar14;
    sVar3 = (short)((ulonglong)uVar14 >> 0x10);
    uVar10 = CONCAT11((0 < sVar3) * (sVar3 < 0x100) * (char)((ulonglong)uVar14 >> 0x10) -
                      (0xff < sVar3),(0 < sVar2) * (sVar2 < 0x100) * (char)uVar14 - (0xff < sVar2));
    sVar2 = (short)((ulonglong)uVar14 >> 0x20);
    sVar3 = (short)((ulonglong)uVar14 >> 0x30);
    uVar12 = CONCAT13((0 < sVar3) * (sVar3 < 0x100) * (char)((ulonglong)uVar14 >> 0x30) -
                      (0xff < sVar3),
                      CONCAT12((0 < sVar2) * (sVar2 < 0x100) * (char)((ulonglong)uVar14 >> 0x20) -
                               (0xff < sVar2),uVar10));
    sVar2 = (short)uVar11;
    sVar3 = (short)((ulonglong)uVar11 >> 0x10);
    sVar4 = (short)((ulonglong)uVar11 >> 0x20);
    param_2 = param_2 + 1;
    *param_5 = uVar16 | (uint)(uVar19 >> 8);
    param_4 = param_4 + 1;
    param_5[1] = CONCAT22(uVar10,(short)(uVar19 >> 0x28));
    param_3 = param_3 + 1;
    param_5[2] = (uint)(CONCAT16((0 < sVar4) * (sVar4 < 0x100) * (char)((ulonglong)uVar11 >> 0x20) -
                                 (0xff < sVar4),
                                 CONCAT15((0 < sVar3) * (sVar3 < 0x100) *
                                          (char)((ulonglong)uVar11 >> 0x10) - (0xff < sVar3),
                                          CONCAT14((0 < sVar2) * (sVar2 < 0x100) * (char)uVar11 -
                                                   (0xff < sVar2),uVar12))) >> 0x18) |
                 (uint)(((ulonglong)(ushort)((uint)uVar12 >> 0x10) << 0x30) >> 0x30);
    uVar8 = uVar8 - 1;
    param_5 = param_5 + 3;
  } while (uVar8 != 0);
  return;
}

