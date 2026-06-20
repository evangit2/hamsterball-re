
void __cdecl IMDCT_ForwardTransform(uint param_1,int *param_2,int *param_3)

{
  int iVar1;
  int iVar2;
  void *this;
  uint uVar3;
  int iVar4;
  int *piVar5;
  byte bVar6;
  int *piVar7;
  int *piVar8;
  int iVar9;
  int *piVar10;
  int *piVar11;
  int iVar12;
  uint uVar13;
  int iVar14;
  int *piVar15;
  int iVar16;
  int *piVar17;
  longlong lVar18;
  longlong lVar19;
  int local_94;
  int local_8c;
  int local_84;
  int local_78;
  int local_70;
  int local_68;
  int local_64;
  int *local_5c;
  int local_54;
  int local_4c;
  int local_44;
  int local_34;
  int local_2c;
  int local_24;
  int local_1c;
  int local_14;
  int local_c;
  
  iVar16 = (int)param_1 >> 1;
  iVar12 = (int)param_1 >> 2;
  iVar2 = 6;
  uVar3 = param_1 & 0x40;
  while (uVar3 == 0) {
    iVar2 = iVar2 + 1;
    uVar3 = param_1 & 1 << ((byte)iVar2 & 0x1f);
  }
  bVar6 = (byte)(0xd - iVar2);
  this = (void *)(2 << (bVar6 & 0x1f));
  piVar8 = param_2 + iVar16 + -7;
  piVar5 = param_3 + iVar12 + iVar16;
  piVar7 = &DAT_004e70b0;
  piVar10 = piVar5;
  do {
    local_78 = (int)((ulonglong)((longlong)*piVar7 * (longlong)piVar8[4]) >> 0x20);
    piVar10[-2] = ((int)((ulonglong)((longlong)piVar8[6] * (longlong)piVar7[1]) >> 0x20) + local_78)
                  * 2;
    piVar15 = piVar10 + -4;
    local_70 = (int)((ulonglong)((longlong)*piVar7 * (longlong)piVar8[6]) >> 0x20);
    piVar10[-1] = local_70 * 2 +
                  (int)((ulonglong)((longlong)piVar7[1] * (longlong)piVar8[4]) >> 0x20) * -2;
    piVar11 = piVar7 + (int)this;
    local_84 = (int)((ulonglong)((longlong)piVar7[(int)this] * (longlong)*piVar8) >> 0x20);
    *piVar15 = ((int)((ulonglong)((longlong)piVar8[2] * (longlong)piVar11[1]) >> 0x20) + local_84) *
               2;
    local_8c = (int)((ulonglong)((longlong)piVar8[2] * (longlong)*piVar11) >> 0x20);
    piVar10[-3] = local_8c * 2 +
                  (int)((ulonglong)((longlong)piVar11[1] * (longlong)*piVar8) >> 0x20) * -2;
    piVar8 = piVar8 + -8;
    piVar7 = piVar11 + (int)this;
    piVar10 = piVar15;
  } while (param_2 + iVar12 <= piVar8);
  do {
    local_78 = (int)((ulonglong)((longlong)piVar7[1] * (longlong)piVar8[4]) >> 0x20);
    piVar15[-2] = ((int)((ulonglong)((longlong)*piVar7 * (longlong)piVar8[6]) >> 0x20) + local_78) *
                  2;
    local_70 = (int)((ulonglong)((longlong)piVar8[6] * (longlong)piVar7[1]) >> 0x20);
    piVar15[-1] = local_70 * 2 +
                  (int)((ulonglong)((longlong)*piVar7 * (longlong)piVar8[4]) >> 0x20) * -2;
    piVar10 = piVar7 + -(int)this;
    local_84 = (int)((ulonglong)((longlong)piVar10[1] * (longlong)*piVar8) >> 0x20);
    piVar15[-4] = ((int)((ulonglong)((longlong)piVar8[2] * (longlong)*piVar10) >> 0x20) + local_84)
                  * 2;
    iVar14 = *piVar8;
    local_8c = (int)((ulonglong)((longlong)piVar8[2] * (longlong)piVar10[1]) >> 0x20);
    piVar8 = piVar8 + -8;
    piVar7 = piVar10 + -(int)this;
    piVar15[-3] = local_8c * 2 +
                  (int)((ulonglong)((longlong)*piVar10 * (longlong)iVar14) >> 0x20) * -2;
    piVar15 = piVar15 + -4;
  } while (param_2 <= piVar8);
  piVar8 = param_2 + iVar16 + -8;
  piVar7 = &DAT_004e70b0;
  piVar10 = piVar5;
  do {
    piVar15 = piVar7 + (int)this;
    local_78 = (int)((ulonglong)((longlong)piVar7[(int)this] * (longlong)piVar8[6]) >> 0x20);
    *piVar10 = local_78 * 2 +
               (int)((ulonglong)((longlong)piVar8[4] * (longlong)piVar15[1]) >> 0x20) * -2;
    local_70 = (int)((ulonglong)((longlong)*piVar15 * (longlong)piVar8[4]) >> 0x20);
    piVar10[1] = ((int)((ulonglong)((longlong)piVar15[1] * (longlong)piVar8[6]) >> 0x20) + local_70)
                 * 2;
    piVar7 = piVar15 + (int)this;
    local_84 = (int)((ulonglong)((longlong)piVar8[2] * (longlong)piVar15[(int)this]) >> 0x20);
    piVar10[2] = local_84 * 2 +
                 (int)((ulonglong)((longlong)piVar7[1] * (longlong)*piVar8) >> 0x20) * -2;
    local_8c = (int)((ulonglong)((longlong)*piVar7 * (longlong)*piVar8) >> 0x20);
    piVar10[3] = ((int)((ulonglong)((longlong)piVar8[2] * (longlong)piVar7[1]) >> 0x20) + local_8c)
                 * 2;
    piVar8 = piVar8 + -8;
    piVar10 = piVar10 + 4;
  } while (param_2 + iVar12 <= piVar8);
  do {
    piVar7 = piVar7 + -(int)this;
    local_78 = (int)((ulonglong)((longlong)piVar7[1] * (longlong)piVar8[6]) >> 0x20);
    *piVar10 = local_78 * 2 +
               (int)((ulonglong)((longlong)*piVar7 * (longlong)piVar8[4]) >> 0x20) * -2;
    local_70 = (int)((ulonglong)((longlong)piVar8[4] * (longlong)piVar7[1]) >> 0x20);
    piVar10[1] = ((int)((ulonglong)((longlong)*piVar7 * (longlong)piVar8[6]) >> 0x20) + local_70) *
                 2;
    piVar7 = piVar7 + -(int)this;
    local_84 = (int)((ulonglong)((longlong)piVar8[2] * (longlong)piVar7[1]) >> 0x20);
    piVar10[2] = local_84 * 2 +
                 (int)((ulonglong)((longlong)*piVar7 * (longlong)*piVar8) >> 0x20) * -2;
    local_8c = (int)((ulonglong)((longlong)piVar7[1] * (longlong)*piVar8) >> 0x20);
    piVar10[3] = ((int)((ulonglong)((longlong)piVar8[2] * (longlong)*piVar7) >> 0x20) + local_8c) *
                 2;
    piVar8 = piVar8 + -8;
    piVar10 = piVar10 + 4;
  } while (param_2 <= piVar8);
  IMDCT_ButterflyPass(param_3 + iVar16,iVar16,0xd - iVar2);
  IMDCT_WindowOverlapCompute(this,param_3,bVar6);
  iVar2 = (int)this >> 2;
  if (iVar2 == 0) {
    iVar2 = 0;
    piVar15 = &DAT_004e80b8;
    iVar14 = 0x7fffffff;
    local_5c = &DAT_004e70b8;
    piVar8 = piVar5;
    piVar10 = param_3 + 3;
    piVar7 = piVar5;
    do {
      iVar9 = *piVar15;
      iVar1 = piVar15[1];
      iVar4 = iVar9 - iVar2 >> 2;
      uVar13 = iVar2 + iVar4;
      iVar2 = iVar1 - iVar14 >> 2;
      uVar3 = iVar14 + iVar2;
      piVar15 = piVar15 + 2;
      piVar17 = piVar7 + -4;
      lVar18 = __allmul(piVar10[-3],piVar10[-3] >> 0x1f,uVar13,(int)uVar13 >> 0x1f);
      lVar19 = __allmul(-piVar10[-2],-piVar10[-2] >> 0x1f,uVar3,(int)uVar3 >> 0x1f);
      local_c = (int)((ulonglong)lVar18 >> 0x20);
      piVar7[-1] = ((int)((ulonglong)lVar19 >> 0x20) + local_c) * 2;
      lVar18 = __allmul(-piVar10[-2],-piVar10[-2] >> 0x1f,uVar13,(int)uVar13 >> 0x1f);
      lVar19 = __allmul(piVar10[-3],piVar10[-3] >> 0x1f,uVar3,(int)uVar3 >> 0x1f);
      *piVar8 = (int)((ulonglong)lVar18 >> 0x20) * 2 + (int)((ulonglong)lVar19 >> 0x20) * -2;
      uVar3 = iVar9 - iVar4;
      uVar13 = iVar1 - iVar2;
      lVar18 = __allmul(piVar10[-1],piVar10[-1] >> 0x1f,uVar3,(int)uVar3 >> 0x1f);
      lVar19 = __allmul(-*piVar10,-*piVar10 >> 0x1f,uVar13,(int)uVar13 >> 0x1f);
      local_1c = (int)((ulonglong)lVar18 >> 0x20);
      piVar7[-2] = ((int)((ulonglong)lVar19 >> 0x20) + local_1c) * 2;
      lVar18 = __allmul(-*piVar10,-*piVar10 >> 0x1f,uVar3,(int)uVar3 >> 0x1f);
      lVar19 = __allmul(piVar10[-1],piVar10[-1] >> 0x1f,uVar13,(int)uVar13 >> 0x1f);
      piVar8[1] = (int)((ulonglong)lVar18 >> 0x20) * 2 + (int)((ulonglong)lVar19 >> 0x20) * -2;
      iVar2 = *local_5c;
      iVar14 = local_5c[1];
      local_5c = local_5c + 2;
      iVar4 = iVar2 - iVar9 >> 2;
      uVar3 = iVar9 + iVar4;
      iVar9 = iVar14 - iVar1 >> 2;
      uVar13 = iVar1 + iVar9;
      lVar18 = __allmul(piVar10[1],piVar10[1] >> 0x1f,uVar3,(int)uVar3 >> 0x1f);
      lVar19 = __allmul(-piVar10[2],-piVar10[2] >> 0x1f,uVar13,(int)uVar13 >> 0x1f);
      local_34 = (int)((ulonglong)lVar18 >> 0x20);
      piVar7[-3] = ((int)((ulonglong)lVar19 >> 0x20) + local_34) * 2;
      lVar18 = __allmul(-piVar10[2],-piVar10[2] >> 0x1f,uVar3,(int)uVar3 >> 0x1f);
      lVar19 = __allmul(piVar10[1],piVar10[1] >> 0x1f,uVar13,(int)uVar13 >> 0x1f);
      local_24 = (int)((ulonglong)lVar18 >> 0x20);
      piVar8[2] = local_24 * 2 + (int)((ulonglong)lVar19 >> 0x20) * -2;
      uVar3 = iVar2 - iVar4;
      uVar13 = iVar14 - iVar9;
      lVar18 = __allmul(piVar10[3],piVar10[3] >> 0x1f,uVar3,(int)uVar3 >> 0x1f);
      lVar19 = __allmul(-piVar10[4],-piVar10[4] >> 0x1f,uVar13,(int)uVar13 >> 0x1f);
      local_14 = (int)((ulonglong)lVar18 >> 0x20);
      *piVar17 = ((int)((ulonglong)lVar19 >> 0x20) + local_14) * 2;
      lVar18 = __allmul(-piVar10[4],-piVar10[4] >> 0x1f,uVar3,(int)uVar3 >> 0x1f);
      lVar19 = __allmul(piVar10[3],piVar10[3] >> 0x1f,uVar13,(int)uVar13 >> 0x1f);
      local_94 = (int)((ulonglong)lVar18 >> 0x20);
      piVar8[3] = local_94 * 2 + (int)((ulonglong)lVar19 >> 0x20) * -2;
      piVar11 = piVar10 + 5;
      piVar8 = piVar8 + 4;
      piVar10 = piVar10 + 8;
      piVar7 = piVar17;
    } while (piVar11 < piVar17);
  }
  else if (iVar2 == 1) {
    iVar2 = 0;
    piVar15 = &DAT_004e80b8;
    local_64 = 0x3fffffff;
    local_5c = &DAT_004e70b8;
    piVar8 = piVar5;
    piVar10 = param_3 + 3;
    piVar7 = piVar5;
    do {
      iVar14 = *piVar15;
      uVar3 = iVar2 + (iVar14 >> 1);
      iVar2 = piVar15[1];
      uVar13 = local_64 + (iVar2 >> 1);
      piVar17 = piVar7 + -4;
      lVar18 = __allmul(piVar10[-3],piVar10[-3] >> 0x1f,uVar3,(int)uVar3 >> 0x1f);
      lVar19 = __allmul(-piVar10[-2],-piVar10[-2] >> 0x1f,uVar13,(int)uVar13 >> 0x1f);
      local_78 = (int)((ulonglong)lVar18 >> 0x20);
      piVar7[-1] = ((int)((ulonglong)lVar19 >> 0x20) + local_78) * 2;
      lVar18 = __allmul(-piVar10[-2],-piVar10[-2] >> 0x1f,uVar3,(int)uVar3 >> 0x1f);
      lVar19 = __allmul(piVar10[-3],piVar10[-3] >> 0x1f,uVar13,(int)uVar13 >> 0x1f);
      local_70 = (int)((ulonglong)lVar18 >> 0x20);
      *piVar8 = local_70 * 2 + (int)((ulonglong)lVar19 >> 0x20) * -2;
      iVar9 = *local_5c;
      uVar13 = (iVar14 >> 1) + (iVar9 >> 1);
      iVar14 = local_5c[1];
      uVar3 = (iVar2 >> 1) + (iVar14 >> 1);
      lVar18 = __allmul(piVar10[-1],piVar10[-1] >> 0x1f,uVar13,(int)uVar13 >> 0x1f);
      lVar19 = __allmul(-*piVar10,-*piVar10 >> 0x1f,uVar3,(int)uVar3 >> 0x1f);
      local_14 = (int)((ulonglong)lVar18 >> 0x20);
      piVar7[-2] = ((int)((ulonglong)lVar19 >> 0x20) + local_14) * 2;
      lVar18 = __allmul(-*piVar10,-*piVar10 >> 0x1f,uVar13,(int)uVar13 >> 0x1f);
      lVar19 = __allmul(piVar10[-1],piVar10[-1] >> 0x1f,uVar3,(int)uVar3 >> 0x1f);
      local_24 = (int)((ulonglong)lVar18 >> 0x20);
      piVar8[1] = local_24 * 2 + (int)((ulonglong)lVar19 >> 0x20) * -2;
      iVar1 = piVar15[2];
      uVar3 = (iVar9 >> 1) + (iVar1 >> 1);
      iVar9 = piVar15[3];
      uVar13 = (iVar14 >> 1) + (iVar9 >> 1);
      piVar15 = piVar15 + 4;
      lVar18 = __allmul(piVar10[1],piVar10[1] >> 0x1f,uVar3,(int)uVar3 >> 0x1f);
      lVar19 = __allmul(-piVar10[2],-piVar10[2] >> 0x1f,uVar13,(int)uVar13 >> 0x1f);
      local_34 = (int)((ulonglong)lVar18 >> 0x20);
      piVar7[-3] = ((int)((ulonglong)lVar19 >> 0x20) + local_34) * 2;
      lVar18 = __allmul(-piVar10[2],-piVar10[2] >> 0x1f,uVar3,(int)uVar3 >> 0x1f);
      lVar19 = __allmul(piVar10[1],piVar10[1] >> 0x1f,uVar13,(int)uVar13 >> 0x1f);
      local_2c = (int)((ulonglong)lVar18 >> 0x20);
      piVar8[2] = local_2c * 2 + (int)((ulonglong)lVar19 >> 0x20) * -2;
      iVar2 = local_5c[2] >> 1;
      uVar13 = (iVar1 >> 1) + iVar2;
      local_64 = local_5c[3] >> 1;
      uVar3 = (iVar9 >> 1) + local_64;
      local_5c = local_5c + 4;
      lVar18 = __allmul(piVar10[3],piVar10[3] >> 0x1f,uVar13,(int)uVar13 >> 0x1f);
      lVar19 = __allmul(-piVar10[4],-piVar10[4] >> 0x1f,uVar3,(int)uVar3 >> 0x1f);
      local_1c = (int)((ulonglong)lVar18 >> 0x20);
      *piVar17 = ((int)((ulonglong)lVar19 >> 0x20) + local_1c) * 2;
      lVar18 = __allmul(-piVar10[4],-piVar10[4] >> 0x1f,uVar13,(int)uVar13 >> 0x1f);
      lVar19 = __allmul(piVar10[3],piVar10[3] >> 0x1f,uVar3,(int)uVar3 >> 0x1f);
      local_c = (int)((ulonglong)lVar18 >> 0x20);
      piVar8[3] = local_c * 2 + (int)((ulonglong)lVar19 >> 0x20) * -2;
      piVar11 = piVar10 + 5;
      piVar8 = piVar8 + 4;
      piVar10 = piVar10 + 8;
      piVar7 = piVar17;
    } while (piVar11 < piVar17);
  }
  else {
    if (iVar2 < 4) {
      piVar8 = &DAT_004e80b8;
    }
    else {
      piVar8 = &DAT_004e70b0 + ((int)this >> 3);
    }
    piVar10 = piVar5;
    piVar7 = param_3 + 3;
    piVar15 = piVar5;
    do {
      local_78 = (int)((ulonglong)((longlong)piVar7[-3] * (longlong)*piVar8) >> 0x20);
      piVar15[-1] = ((int)((ulonglong)((longlong)-piVar7[-2] * (longlong)piVar8[1]) >> 0x20) +
                    local_78) * 2;
      piVar17 = piVar15 + -4;
      local_70 = (int)((ulonglong)((longlong)-piVar7[-2] * (longlong)*piVar8) >> 0x20);
      *piVar10 = local_70 * 2 +
                 (int)((ulonglong)((longlong)piVar7[-3] * (longlong)piVar8[1]) >> 0x20) * -2;
      piVar11 = piVar8 + iVar2;
      local_84 = (int)((ulonglong)((longlong)piVar7[-1] * (longlong)piVar8[iVar2]) >> 0x20);
      piVar15[-2] = ((int)((ulonglong)((longlong)-*piVar7 * (longlong)piVar11[1]) >> 0x20) +
                    local_84) * 2;
      local_8c = (int)((ulonglong)((longlong)-*piVar7 * (longlong)*piVar11) >> 0x20);
      piVar10[1] = local_8c * 2 +
                   (int)((ulonglong)((longlong)piVar7[-1] * (longlong)piVar11[1]) >> 0x20) * -2;
      piVar8 = piVar11 + iVar2;
      local_54 = (int)((ulonglong)((longlong)piVar7[1] * (longlong)piVar11[iVar2]) >> 0x20);
      piVar15[-3] = ((int)((ulonglong)((longlong)-piVar7[2] * (longlong)piVar8[1]) >> 0x20) +
                    local_54) * 2;
      local_68 = (int)((ulonglong)((longlong)-piVar7[2] * (longlong)*piVar8) >> 0x20);
      piVar10[2] = local_68 * 2 +
                   (int)((ulonglong)((longlong)piVar7[1] * (longlong)piVar8[1]) >> 0x20) * -2;
      piVar8 = piVar8 + iVar2;
      local_4c = (int)((ulonglong)((longlong)piVar7[3] * (longlong)*piVar8) >> 0x20);
      *piVar17 = ((int)((ulonglong)((longlong)-piVar7[4] * (longlong)piVar8[1]) >> 0x20) + local_4c)
                 * 2;
      local_44 = (int)((ulonglong)((longlong)-piVar7[4] * (longlong)*piVar8) >> 0x20);
      piVar10[3] = local_44 * 2 +
                   (int)((ulonglong)((longlong)piVar7[3] * (longlong)piVar8[1]) >> 0x20) * -2;
      piVar11 = piVar7 + 5;
      piVar8 = piVar8 + iVar2;
      piVar10 = piVar10 + 4;
      piVar7 = piVar7 + 8;
      piVar15 = piVar17;
    } while (piVar11 < piVar17);
  }
  piVar8 = piVar5;
  piVar10 = param_3 + iVar12 + 2;
  piVar7 = piVar5 + (2 - iVar16);
  do {
    iVar2 = piVar8[-1];
    piVar7[-3] = iVar2;
    piVar17 = piVar8 + -4;
    piVar10[-2] = -iVar2;
    iVar2 = piVar8[-2];
    piVar7[-4] = iVar2;
    piVar10[-1] = -iVar2;
    iVar2 = piVar8[-3];
    piVar7[-5] = iVar2;
    *piVar10 = -iVar2;
    iVar2 = *piVar17;
    piVar7[-6] = iVar2;
    piVar10[1] = -iVar2;
    piVar15 = piVar10 + 2;
    piVar8 = piVar17;
    piVar11 = piVar5;
    piVar10 = piVar10 + 4;
    piVar7 = piVar7 + -4;
  } while (piVar15 < piVar17);
  do {
    piVar8 = piVar5 + -4;
    *piVar8 = piVar11[3];
    piVar5[-3] = piVar11[2];
    piVar5[-2] = piVar11[1];
    piVar5[-1] = *piVar11;
    piVar5 = piVar8;
    piVar11 = piVar11 + 4;
  } while (param_3 + iVar16 < piVar8);
  return;
}

