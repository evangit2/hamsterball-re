
/* WARNING: Function: __chkstk replaced with injection: alloca_probe */
/* WARNING: Unable to track spacebase fully for stack */

void __cdecl
IMDCT_ComputeWithFloor
          (undefined4 *param_1,int param_2,int param_3,undefined4 param_4,int param_5,uint param_6,
          int param_7,int param_8,int param_9)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  uint uVar7;
  int iVar8;
  uint uVar9;
  uint uVar10;
  uint uVar11;
  uint *puVar12;
  int *piVar13;
  longlong lVar14;
  uint auStack_44 [4];
  undefined4 uStack_34;
  int local_8;
  
  iVar3 = param_7;
  iVar4 = param_8 * 0x1000;
  uStack_34 = 0x4aa10d;
  iVar2 = param_6 * -4;
  piVar13 = (int *)(&stack0xffffffd0 + iVar2);
  param_8 = 0;
  if (0 < (int)param_6) {
    do {
      iVar6 = *(int *)((param_5 - (int)(&stack0xffffffd0 + iVar2)) + (int)piVar13);
      auStack_44[4 - param_6] = 0;
      auStack_44[3 - param_6] = (uint)&DAT_00517cc2;
      auStack_44[2 - param_6] = iVar6 >> 0x1f;
      auStack_44[1 - param_6] = iVar6;
      auStack_44[-param_6] = 0x4aa142;
      lVar14 = __allmul(auStack_44[1 - param_6],auStack_44[2 - param_6],auStack_44[3 - param_6],
                        auStack_44[4 - param_6]);
      uVar9 = (uint)((ulonglong)lVar14 >> 0x20);
      if ((lVar14 < 0) || (iVar6 = (int)uVar9 >> 9, 0x7f < iVar6)) {
        for (; param_3 != 0; param_3 = param_3 + -1) {
          *param_1 = 0;
          param_1 = param_1 + 1;
        }
        return;
      }
      *piVar13 = *(int *)(&DAT_004e61d0 + iVar6 * 4) -
                 ((int)((*(int *)(&DAT_004e61d0 + iVar6 * 4) - *(int *)(&DAT_004e61d4 + iVar6 * 4))
                       * (uVar9 & 0x1ff)) >> 9);
      param_8 = param_8 + 1;
      piVar13 = piVar13 + 1;
    } while (param_8 < (int)param_6);
  }
  param_8 = 0;
  if (0 < param_3) {
    do {
      iVar6 = *(int *)(param_2 + param_8 * 4);
      iVar8 = *(int *)(param_9 + iVar6 * 4);
      uVar9 = *(int *)(&stack0xffffffd0 + iVar2) - iVar8 >> 0x1f;
      uVar9 = ((*(int *)(&stack0xffffffd0 + iVar2) - iVar8 ^ uVar9) - uVar9) * 0xb505;
      uVar10 = *(int *)(&stack0xffffffd4 + iVar2) - iVar8 >> 0x1f;
      uVar10 = ((*(int *)(&stack0xffffffd4 + iVar2) - iVar8 ^ uVar10) - uVar10) * 0xb505;
      param_7 = 0;
      local_8 = 3;
      if (3 < (int)param_6) {
        do {
          uVar5 = uVar9 | uVar10;
          uVar7 = (uint)(byte)(&DAT_004e6448)[uVar5 >> 0x19];
          if ((uVar7 == 0) && (uVar7 = (uint)(byte)(&DAT_004e6488)[uVar5 >> 0x13], uVar7 == 0)) {
            uVar7 = (uint)(byte)(&DAT_004e64c8)[uVar5 >> 0x10];
          }
          uVar5 = (int)(auStack_44[(local_8 + 4) - param_6] - iVar8) >> 0x1f;
          uVar9 = (uVar9 >> ((byte)uVar7 & 0x1f)) *
                  ((auStack_44[(local_8 + 4) - param_6] - iVar8 ^ uVar5) - uVar5);
          uVar5 = *(int *)(&stack0xffffffd0 + local_8 * 4 + iVar2) - iVar8 >> 0x1f;
          uVar10 = (uVar10 >> ((byte)uVar7 & 0x1f)) *
                   ((*(int *)(&stack0xffffffd0 + local_8 * 4 + iVar2) - iVar8 ^ uVar5) - uVar5);
          param_7 = param_7 + uVar7;
          local_8 = local_8 + 2;
        } while (local_8 < (int)param_6);
      }
      uVar5 = uVar9 | uVar10;
      uVar7 = (uint)(byte)(&DAT_004e6448)[uVar5 >> 0x19];
      if ((uVar7 == 0) && (uVar7 = (uint)(byte)(&DAT_004e6488)[uVar5 >> 0x13], uVar7 == 0)) {
        uVar7 = (uint)(byte)(&DAT_004e64c8)[uVar5 >> 0x10];
      }
      uVar10 = uVar10 >> ((byte)uVar7 & 0x1f);
      uVar9 = uVar9 >> ((byte)uVar7 & 0x1f);
      if ((param_6 & 1) == 0) {
        uVar5 = (uVar7 + param_7) * 2 + param_6 * -0xd;
        uVar9 = (uVar10 * uVar10 >> 0x10) * (0x4000 - iVar8) +
                (uVar9 * uVar9 >> 0x10) * (iVar8 + 0x4000) >> 0xe;
      }
      else {
        uVar5 = (int)(auStack_44[(local_8 + 4) - param_6] - iVar8) >> 0x1f;
        uVar9 = ((auStack_44[(local_8 + 4) - param_6] - iVar8 ^ uVar5) - uVar5) * uVar9;
        uVar11 = uVar9 | uVar10 << 0xe;
        uVar5 = (uint)(byte)(&DAT_004e6448)[uVar11 >> 0x19];
        if ((uVar5 == 0) && (uVar5 = (uint)(byte)(&DAT_004e6488)[uVar11 >> 0x13], uVar5 == 0)) {
          uVar5 = (uint)(byte)(&DAT_004e64c8)[uVar11 >> 0x10];
        }
        uVar10 = (uVar10 << 0xe) >> ((byte)uVar5 & 0x1f);
        uVar9 = uVar9 >> ((byte)uVar5 & 0x1f);
        uVar5 = param_6 + (uVar5 + ((int)(param_6 + 1) >> 1) * -0xe + param_7 + uVar7) * 2;
        uVar9 = ((0x4000 - (iVar8 * iVar8 >> 0xe)) * (uVar10 * uVar10 >> 0x10) >> 0xe) +
                (uVar9 * uVar9 >> 0x10);
      }
      if ((uVar9 & 0xffff0000) == 0) {
        for (; (uVar9 != 0 && (-1 < (char)(uVar9 >> 8))); uVar9 = uVar9 << 1) {
          uVar5 = uVar5 - 1;
        }
      }
      else {
        uVar9 = uVar9 >> 1;
        uVar5 = uVar5 + 1;
      }
      uVar10 = (int)uVar9 >> 9 & 0x3f;
      iVar8 = iVar4 - ((*(int *)(&DAT_004e5fc8 + uVar10 * 4) -
                       ((int)(*(int *)(&DAT_004e60d0 + uVar10 * 4) * (uVar9 & 0x3ff)) >> 10)) *
                       *(int *)(&DAT_004fabc8 + (uVar5 & 1) * 4) >>
                      ((char)((int)uVar5 >> 1) + 0x15U & 0x1f)) * iVar3;
      uVar9 = iVar8 >> 9;
      if ((int)uVar9 < 0) {
        iVar8 = 0x7fffffff;
      }
      else if ((int)uVar9 < 0x460) {
        iVar8 = *(int *)(&DAT_004e5f48 + (uVar9 & 0x1f) * 4) *
                *(int *)(&DAT_004e5eb8 + (iVar8 >> 0xe) * 4);
      }
      else {
        iVar8 = 0;
      }
      iVar1 = param_1[param_8];
      auStack_44[4 - param_6] = iVar8 >> 0x1f;
      auStack_44[3 - param_6] = iVar8;
      auStack_44[2 - param_6] = iVar1 >> 0x1f;
      auStack_44[1 - param_6] = iVar1;
      auStack_44[-param_6] = 0x4aa42b;
      lVar14 = __allmul(auStack_44[1 - param_6],auStack_44[2 - param_6],auStack_44[3 - param_6],
                        auStack_44[4 - param_6]);
      param_1[param_8] = (uint)lVar14 >> 0xf | (int)((ulonglong)lVar14 >> 0x20) << 0x11;
      iVar1 = param_8 * 4;
      param_8 = param_8 + 1;
      if (*(int *)(param_2 + 4 + iVar1) == iVar6) {
        puVar12 = param_1 + param_8;
        do {
          uVar9 = *puVar12;
          auStack_44[4 - param_6] = iVar8 >> 0x1f;
          auStack_44[3 - param_6] = iVar8;
          auStack_44[2 - param_6] = (int)uVar9 >> 0x1f;
          auStack_44[1 - param_6] = uVar9;
          auStack_44[-param_6] = 0x4aa460;
          lVar14 = __allmul(auStack_44[1 - param_6],auStack_44[2 - param_6],auStack_44[3 - param_6],
                            auStack_44[4 - param_6]);
          *puVar12 = (uint)lVar14 >> 0xf | (int)((ulonglong)lVar14 >> 0x20) << 0x11;
          piVar13 = (int *)((param_2 - (int)param_1) + 4 + (int)puVar12);
          puVar12 = puVar12 + 1;
          param_8 = param_8 + 1;
        } while (*piVar13 == iVar6);
      }
    } while (param_8 < param_3);
  }
  return;
}

