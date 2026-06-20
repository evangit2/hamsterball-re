
/* WARNING: Function: __chkstk replaced with injection: alloca_probe */
/* WARNING: Unable to track spacebase fully for stack */

undefined4 __cdecl Audio_CompressFrame(int *param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  code *pcVar6;
  int iVar7;
  undefined4 uVar8;
  uint uVar9;
  int *piVar10;
  uint *puVar11;
  uint *puVar12;
  int iVar13;
  int *piVar14;
  int iVar15;
  int *piVar16;
  undefined1 *puVar17;
  undefined1 *puVar18;
  undefined1 *puVar19;
  undefined1 *puVar20;
  undefined1 *puVar21;
  undefined1 *puVar22;
  undefined1 *puVar23;
  undefined4 *puVar24;
  undefined4 uStack_44;
  int local_24;
  int local_1c;
  uint *local_18;
  int *local_8;
  
  iVar1 = *(int *)(param_1[0x12] + 4);
  uVar2 = *(undefined4 *)(param_1[0x12] + 0x48);
  iVar3 = *(int *)(iVar1 + 0x1c);
  piVar16 = *(int **)(param_2 + 4);
  iVar4 = *(int *)(iVar3 + param_1[8] * 4);
  param_1[10] = iVar4;
  iVar5 = *(int *)(iVar1 + 4);
  uStack_44 = 0x4a8a7d;
  local_18 = (uint *)(&stack0xffffffc0 + iVar5 * -0xc);
  puVar19 = &stack0xffffffc0 + iVar5 * -0x10;
  puVar17 = &stack0xffffffc0 + iVar5 * -0x10;
  (&uStack_44)[-iVar5] = 0x4a8a8d;
  (&uStack_44)[iVar5 * -2] = 0x4a8a9d;
  (&uStack_44)[iVar5 * -3] = 0x4a8aad;
  iVar15 = 0;
  if (0 < *(int *)(iVar1 + 4)) {
    local_8 = piVar16;
    do {
      local_8 = local_8 + 1;
      iVar13 = *(int *)(*(int *)(param_2 + 0x10) + *local_8 * 4);
      *(undefined4 *)(puVar17 + -4) = *(undefined4 *)(*(int *)(param_2 + 8) + *local_8 * 4);
      *(int **)(puVar17 + -8) = param_1;
      pcVar6 = *(code **)(iVar13 + 0x10);
      puVar18 = puVar17 + -0xc;
      *(undefined4 *)(puVar17 + -0xc) = 0x4a8aff;
      uVar9 = (*pcVar6)();
      local_18[-iVar5] = uVar9;
      puVar19 = puVar18 + 8;
      puVar17 = puVar18 + 8;
      *local_18 = (uint)(uVar9 != 0);
      puVar24 = *(undefined4 **)(*param_1 + iVar15 * 4);
      for (uVar9 = (uint)(iVar4 << 2) >> 3; uVar9 != 0; uVar9 = uVar9 - 1) {
        *puVar24 = 0;
        puVar24 = puVar24 + 1;
      }
      for (uVar9 = (uint)(iVar4 << 2) >> 1 & 3; uVar9 != 0; uVar9 = uVar9 - 1) {
        *(undefined1 *)puVar24 = 0;
        puVar24 = (undefined4 *)((int)puVar24 + 1);
      }
      local_18 = local_18 + 1;
      iVar15 = iVar15 + 1;
    } while (iVar15 < *(int *)(iVar1 + 4));
  }
  local_8 = (int *)0x0;
  if (0 < piVar16[0x123]) {
    piVar10 = piVar16 + 0x224;
    do {
      if ((*(int *)(&stack0xffffffc0 + piVar10[-0x100] * 4 + iVar5 * -0xc) != 0) ||
         (*(int *)(&stack0xffffffc0 + *piVar10 * 4 + iVar5 * -0xc) != 0)) {
        *(undefined4 *)(&stack0xffffffc0 + piVar10[-0x100] * 4 + iVar5 * -0xc) = 1;
        *(undefined4 *)(&stack0xffffffc0 + *piVar10 * 4 + iVar5 * -0xc) = 1;
      }
      local_8 = (int *)((int)local_8 + 1);
      piVar10 = piVar10 + 1;
    } while ((int)local_8 < piVar16[0x123]);
  }
  local_8 = (int *)0x0;
  puVar21 = puVar19;
  if (0 < *piVar16) {
    do {
      local_1c = 0;
      if (0 < *(int *)(iVar1 + 4)) {
        iVar15 = 0;
        puVar11 = (uint *)(&stack0xffffffc0 + iVar5 * -8);
        local_18 = (uint *)piVar16;
        do {
          local_18 = local_18 + 1;
          puVar12 = puVar11;
          if ((int *)*local_18 == local_8) {
            local_1c = local_1c + 1;
            puVar12 = puVar11 + 1;
            *puVar11 = (uint)(*(int *)(&stack0xffffffc0 + iVar15 * 4 + iVar5 * -0xc) != 0);
            puVar12[iVar5 + -1] = *(uint *)(*param_1 + iVar15 * 4);
          }
          iVar15 = iVar15 + 1;
          puVar11 = puVar12;
        } while (iVar15 < *(int *)(iVar1 + 4));
      }
      iVar15 = *(int *)(*(int *)(param_2 + 0x14) + (int)local_8 * 4);
      iVar13 = *(int *)(param_2 + 0xc);
      *(int *)(puVar19 + -4) = local_1c;
      *(undefined1 **)(puVar19 + -8) = &stack0xffffffc0 + iVar5 * -8;
      *(undefined1 **)(puVar19 + -0xc) = &stack0xffffffc0 + iVar5 * -4;
      *(undefined4 *)(puVar19 + -0x10) = *(undefined4 *)(iVar13 + (int)local_8 * 4);
      *(int **)(puVar19 + -0x14) = param_1;
      pcVar6 = *(code **)(iVar15 + 0x10);
      puVar20 = puVar19 + -0x18;
      *(undefined4 *)(puVar19 + -0x18) = 0x4a8c2b;
      (*pcVar6)();
      puVar21 = puVar20 + 0x14;
      puVar19 = puVar20 + 0x14;
      local_8 = (int *)((int)local_8 + 1);
    } while ((int)local_8 < *piVar16);
  }
  local_1c = piVar16[0x123];
  if (-1 < local_1c + -1) {
    piVar10 = piVar16 + local_1c + 0x223;
    do {
      piVar14 = *(int **)(*param_1 + piVar10[-0x100] * 4);
      if (0 < iVar4 / 2) {
        iVar15 = *(int *)(*param_1 + *piVar10 * 4) - (int)piVar14;
        local_24 = iVar4 / 2;
        do {
          iVar13 = *piVar14;
          iVar7 = *(int *)(iVar15 + (int)piVar14);
          if (iVar13 < 1) {
            if (iVar7 < 1) {
              *(int *)(iVar15 + (int)piVar14) = iVar13;
              *piVar14 = iVar13 - iVar7;
            }
            else {
              *(int *)(iVar15 + (int)piVar14) = iVar7 + iVar13;
            }
          }
          else if (iVar7 < 1) {
            *(int *)(iVar15 + (int)piVar14) = iVar13;
            *piVar14 = iVar7 + iVar13;
          }
          else {
            *(int *)(iVar15 + (int)piVar14) = iVar13 - iVar7;
          }
          piVar14 = piVar14 + 1;
          local_24 = local_24 + -1;
        } while (local_24 != 0);
      }
      piVar10 = piVar10 + -1;
      local_1c = local_1c + -1;
    } while (local_1c != 0);
  }
  iVar15 = 0;
  puVar23 = puVar21;
  if (0 < *(int *)(iVar1 + 4)) {
    do {
      piVar16 = piVar16 + 1;
      iVar13 = *piVar16;
      iVar7 = *(int *)(*(int *)(param_2 + 0x10) + iVar13 * 4);
      *(undefined4 *)(puVar21 + -4) = *(undefined4 *)(*param_1 + iVar15 * 4);
      *(undefined4 *)(puVar21 + -8) = *(undefined4 *)(&stack0xffffffc0 + iVar15 * 4 + iVar5 * -0x10)
      ;
      *(undefined4 *)(puVar21 + -0xc) = *(undefined4 *)(*(int *)(param_2 + 8) + iVar13 * 4);
      *(int **)(puVar21 + -0x10) = param_1;
      pcVar6 = *(code **)(iVar7 + 0x14);
      puVar22 = puVar21 + -0x14;
      *(undefined4 *)(puVar21 + -0x14) = 0x4a8d06;
      (*pcVar6)();
      puVar21 = puVar22 + 0x10;
      puVar23 = puVar22 + 0x10;
      iVar15 = iVar15 + 1;
    } while (iVar15 < *(int *)(iVar1 + 4));
  }
  iVar15 = 0;
  if (0 < *(int *)(iVar1 + 4)) {
    do {
      uVar8 = *(undefined4 *)(*param_1 + iVar15 * 4);
      *(undefined4 *)(puVar23 + -4) = uVar8;
      *(undefined4 *)(puVar23 + -8) = uVar8;
      *(int *)(puVar23 + -0xc) = iVar4;
      *(undefined4 *)(puVar23 + -0x10) = 0x4a8d32;
      IMDCT_ForwardTransform
                (*(uint *)(puVar23 + -0xc),*(int **)(puVar23 + -8),*(int **)(puVar23 + -4));
      iVar15 = iVar15 + 1;
    } while (iVar15 < *(int *)(iVar1 + 4));
  }
  iVar15 = 0;
  if (*(int *)(iVar1 + 4) < 1) {
    DAT_00535284 = DAT_00535284 + *(int *)(iVar1 + 4);
    return 0;
  }
  do {
    puVar24 = *(undefined4 **)(*param_1 + iVar15 * 4);
    if (*(int *)(&stack0xffffffc0 + iVar15 * 4 + iVar5 * -0xc) == 0) {
      iVar13 = iVar4;
      if (0 < iVar4) {
        for (; iVar13 != 0; iVar13 = iVar13 + -1) {
          *puVar24 = 0;
          puVar24 = puVar24 + 1;
        }
      }
    }
    else {
      iVar13 = param_1[8];
      iVar7 = param_1[7];
      *(int *)(puVar23 + -4) = param_1[9];
      *(int *)(puVar23 + -8) = iVar13;
      *(int *)(puVar23 + -0xc) = iVar7;
      *(int *)(puVar23 + -0x10) = iVar3;
      *(undefined4 *)(puVar23 + -0x14) = uVar2;
      *(undefined4 **)(puVar23 + -0x18) = puVar24;
      *(undefined4 *)(puVar23 + -0x1c) = 0x4a8d78;
      Huffman_CompressBlock
                (*(undefined4 **)(puVar23 + -0x18),*(int **)(puVar23 + -0x14),
                 *(int *)(puVar23 + -0x10),*(int *)(puVar23 + -0xc),*(int *)(puVar23 + -8),
                 *(int *)(puVar23 + -4));
    }
    iVar15 = iVar15 + 1;
  } while (iVar15 < *(int *)(iVar1 + 4));
  DAT_00535284 = DAT_00535284 + *(int *)(iVar1 + 4);
  return 0;
}

