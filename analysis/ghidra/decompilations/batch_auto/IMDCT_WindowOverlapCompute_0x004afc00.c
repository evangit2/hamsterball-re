
void __thiscall IMDCT_WindowOverlapCompute(void *this,int *param_1,byte param_2)

{
  uint *puVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  int in_EAX;
  uint uVar6;
  uint uVar7;
  int iVar8;
  uint uVar9;
  int iVar10;
  int iVar11;
  int *piVar12;
  int *piVar13;
  uint uVar14;
  uint uVar15;
  uint *puVar16;
  longlong lVar17;
  longlong lVar18;
  int local_1c;
  int local_14;
  int local_c;
  int local_4;
  
  uVar14 = 0;
  piVar2 = param_1 + (in_EAX >> 1);
  if ((int)this < 4) {
    puVar16 = &DAT_004e80b8;
  }
  else {
    puVar16 = &DAT_004e70b0 + ((int)this >> 1);
  }
  puVar1 = puVar16 + 0x400;
  piVar12 = piVar2;
  do {
    uVar6 = ((uint)(byte)(&DAT_004fb25c)[(int)uVar14 >> 4 & 0xf] |
            (uint)(byte)(&DAT_004fb25c)[uVar14 & 0xf] << 4) << 4 |
            (uint)(byte)(&DAT_004fb25c)[(int)uVar14 >> 8];
    uVar15 = uVar14 + 1;
    iVar11 = piVar2[((int)(uVar6 ^ 0xfff) >> (param_2 & 0x1f)) + -1];
    iVar8 = (piVar2 + ((int)(uVar6 ^ 0xfff) >> (param_2 & 0x1f)) + -1)[1];
    iVar3 = piVar2[(int)uVar6 >> (param_2 & 0x1f)];
    uVar9 = iVar11 + iVar3;
    iVar4 = (piVar2 + ((int)uVar6 >> (param_2 & 0x1f)))[1];
    uVar7 = iVar4 - iVar8;
    uVar6 = puVar16[1];
    lVar17 = __allmul(uVar6,(int)uVar6 >> 0x1f,uVar9,(int)uVar9 >> 0x1f);
    uVar5 = *puVar16;
    lVar18 = __allmul(uVar5,(int)uVar5 >> 0x1f,uVar7,(int)uVar7 >> 0x1f);
    local_1c = (int)((ulonglong)lVar17 >> 0x20);
    local_1c = (int)((ulonglong)lVar18 >> 0x20) + local_1c;
    lVar17 = __allmul(uVar6,(int)uVar6 >> 0x1f,uVar7,(int)uVar7 >> 0x1f);
    lVar18 = __allmul(uVar5,(int)uVar5 >> 0x1f,uVar9,(int)uVar9 >> 0x1f);
    iVar10 = (int)((ulonglong)lVar17 >> 0x20) - (int)((ulonglong)lVar18 >> 0x20);
    puVar16 = puVar16 + (int)this;
    iVar8 = iVar8 + iVar4 >> 1;
    iVar11 = iVar11 - iVar3 >> 1;
    *param_1 = local_1c + iVar8;
    param_1[1] = iVar11 + iVar10;
    piVar12[-2] = iVar8 - local_1c;
    piVar12[-1] = iVar10 - iVar11;
    piVar13 = piVar12 + -4;
    uVar6 = ((uint)(byte)(&DAT_004fb25c)[(int)uVar15 >> 4 & 0xf] |
            (uint)(byte)(&DAT_004fb25c)[uVar15 & 0xf] << 4) << 4 |
            (uint)(byte)(&DAT_004fb25c)[(int)uVar15 >> 8];
    uVar14 = uVar14 + 2;
    iVar11 = piVar2[((int)(uVar6 ^ 0xfff) >> (param_2 & 0x1f)) + -1];
    iVar8 = (piVar2 + ((int)(uVar6 ^ 0xfff) >> (param_2 & 0x1f)) + -1)[1];
    iVar3 = piVar2[(int)uVar6 >> (param_2 & 0x1f)];
    uVar9 = iVar11 + iVar3;
    iVar4 = (piVar2 + ((int)uVar6 >> (param_2 & 0x1f)))[1];
    uVar7 = iVar4 - iVar8;
    uVar6 = puVar16[1];
    lVar17 = __allmul(uVar6,(int)uVar6 >> 0x1f,uVar9,(int)uVar9 >> 0x1f);
    uVar5 = *puVar16;
    lVar18 = __allmul(uVar5,(int)uVar5 >> 0x1f,uVar7,(int)uVar7 >> 0x1f);
    local_c = (int)((ulonglong)lVar17 >> 0x20);
    local_c = (int)((ulonglong)lVar18 >> 0x20) + local_c;
    lVar17 = __allmul(uVar6,(int)uVar6 >> 0x1f,uVar7,(int)uVar7 >> 0x1f);
    lVar18 = __allmul(uVar5,(int)uVar5 >> 0x1f,uVar9,(int)uVar9 >> 0x1f);
    iVar10 = (int)((ulonglong)lVar17 >> 0x20) - (int)((ulonglong)lVar18 >> 0x20);
    puVar16 = puVar16 + (int)this;
    iVar8 = iVar8 + iVar4 >> 1;
    iVar11 = iVar11 - iVar3 >> 1;
    param_1[2] = local_c + iVar8;
    param_1[3] = iVar11 + iVar10;
    *piVar13 = iVar8 - local_c;
    piVar12[-3] = iVar10 - iVar11;
    param_1 = param_1 + 4;
    piVar12 = piVar13;
  } while (puVar16 < puVar1);
  do {
    uVar6 = ((uint)(byte)(&DAT_004fb25c)[(int)uVar14 >> 4 & 0xf] |
            (uint)(byte)(&DAT_004fb25c)[uVar14 & 0xf] << 4) << 4 |
            (uint)(byte)(&DAT_004fb25c)[(int)uVar14 >> 8];
    uVar15 = uVar14 + 1;
    iVar11 = piVar2[((int)(uVar6 ^ 0xfff) >> (param_2 & 0x1f)) + -1];
    iVar8 = (piVar2 + ((int)(uVar6 ^ 0xfff) >> (param_2 & 0x1f)) + -1)[1];
    iVar3 = piVar2[(int)uVar6 >> (param_2 & 0x1f)];
    uVar7 = iVar3 + iVar11;
    iVar4 = (piVar2 + ((int)uVar6 >> (param_2 & 0x1f)))[1];
    uVar9 = iVar4 - iVar8;
    puVar16 = puVar16 + -(int)this;
    uVar6 = *puVar16;
    lVar17 = __allmul(uVar7,(int)uVar7 >> 0x1f,uVar6,(int)uVar6 >> 0x1f);
    uVar5 = puVar16[1];
    lVar18 = __allmul(uVar5,(int)uVar5 >> 0x1f,uVar9,(int)uVar9 >> 0x1f);
    local_4 = (int)((ulonglong)lVar17 >> 0x20);
    local_4 = (int)((ulonglong)lVar18 >> 0x20) + local_4;
    lVar17 = __allmul(uVar9,(int)uVar9 >> 0x1f,uVar6,(int)uVar6 >> 0x1f);
    lVar18 = __allmul(uVar5,(int)uVar5 >> 0x1f,uVar7,(int)uVar7 >> 0x1f);
    iVar10 = (int)((ulonglong)lVar17 >> 0x20) - (int)((ulonglong)lVar18 >> 0x20);
    iVar8 = iVar4 + iVar8 >> 1;
    iVar11 = iVar11 - iVar3 >> 1;
    *param_1 = local_4 + iVar8;
    param_1[1] = iVar11 + iVar10;
    piVar13[-2] = iVar8 - local_4;
    piVar13[-1] = iVar10 - iVar11;
    piVar12 = piVar13 + -4;
    uVar6 = ((uint)(byte)(&DAT_004fb25c)[(int)uVar15 >> 4 & 0xf] |
            (uint)(byte)(&DAT_004fb25c)[uVar15 & 0xf] << 4) << 4 |
            (uint)(byte)(&DAT_004fb25c)[(int)uVar15 >> 8];
    uVar14 = uVar14 + 2;
    iVar11 = piVar2[((int)(uVar6 ^ 0xfff) >> (param_2 & 0x1f)) + -1];
    iVar8 = (piVar2 + ((int)(uVar6 ^ 0xfff) >> (param_2 & 0x1f)) + -1)[1];
    iVar3 = piVar2[(int)uVar6 >> (param_2 & 0x1f)];
    uVar7 = iVar3 + iVar11;
    iVar4 = (piVar2 + ((int)uVar6 >> (param_2 & 0x1f)))[1];
    uVar9 = iVar4 - iVar8;
    puVar16 = puVar16 + -(int)this;
    uVar6 = *puVar16;
    lVar17 = __allmul(uVar7,(int)uVar7 >> 0x1f,uVar6,(int)uVar6 >> 0x1f);
    uVar5 = puVar16[1];
    lVar18 = __allmul(uVar5,(int)uVar5 >> 0x1f,uVar9,(int)uVar9 >> 0x1f);
    local_14 = (int)((ulonglong)lVar17 >> 0x20);
    local_14 = (int)((ulonglong)lVar18 >> 0x20) + local_14;
    lVar17 = __allmul(uVar9,(int)uVar9 >> 0x1f,uVar6,(int)uVar6 >> 0x1f);
    lVar18 = __allmul(uVar5,(int)uVar5 >> 0x1f,uVar7,(int)uVar7 >> 0x1f);
    iVar10 = (int)((ulonglong)lVar17 >> 0x20) - (int)((ulonglong)lVar18 >> 0x20);
    iVar8 = iVar4 + iVar8 >> 1;
    iVar11 = iVar11 - iVar3 >> 1;
    param_1[2] = local_14 + iVar8;
    param_1[3] = iVar11 + iVar10;
    param_1 = param_1 + 4;
    *piVar12 = iVar8 - local_14;
    piVar13[-3] = iVar10 - iVar11;
    piVar13 = piVar12;
  } while (param_1 < piVar12);
  return;
}

