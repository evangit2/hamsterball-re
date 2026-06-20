
/* WARNING: Function: __chkstk replaced with injection: alloca_probe */
/* WARNING: Unable to track spacebase fully for stack */

undefined4 __cdecl Huffman_BuildDecodeTable(undefined4 *param_1,undefined4 *param_2)

{
  char cVar1;
  undefined4 uVar2;
  undefined4 *puVar3;
  uint *puVar4;
  void *pvVar5;
  int iVar6;
  int *piVar7;
  int iVar8;
  uint uVar9;
  uint uVar10;
  int iVar11;
  uint *puVar12;
  undefined4 *puVar13;
  undefined4 *puVar14;
  uint uVar15;
  size_t asStackY_40 [4];
  undefined4 *local_10;
  uint *local_c;
  int local_8;
  
  puVar13 = param_2;
  puVar3 = param_1;
  puVar14 = param_1;
  for (iVar8 = 0xd; iVar8 != 0; iVar8 = iVar8 + -1) {
    *puVar14 = 0;
    puVar14 = puVar14 + 1;
  }
  iVar8 = param_2[1];
  puVar14 = (undefined4 *)0x0;
  if (0 < iVar8) {
    piVar7 = (int *)param_2[2];
    iVar11 = iVar8;
    do {
      if (0 < *piVar7) {
        puVar14 = (undefined4 *)((int)puVar14 + 1);
      }
      piVar7 = piVar7 + 1;
      iVar11 = iVar11 + -1;
    } while (iVar11 != 0);
  }
  param_1[1] = iVar8;
  param_1[2] = puVar14;
  *param_1 = *param_2;
  param_1[0xb] = param_2[4];
  param_1[0xc] = param_2[5];
  asStackY_40[3] = 0x498c48;
  puVar4 = Huffman_AssignCodes(param_2[2],param_2[1],(int)puVar14);
  iVar8 = (int)puVar14 * -4;
  if (puVar4 == (uint *)0x0) {
    *(undefined4 **)(&stack0xffffffd8 + iVar8) = param_1;
    *(undefined4 *)(&stack0xffffffd4 + iVar8) = 0x498c75;
    Huffman_FreeTableExtended(*(undefined4 **)(&stack0xffffffd8 + iVar8));
    return 0xffffffff;
  }
  if (0 < (int)puVar14) {
    puVar12 = puVar4;
    param_1 = puVar14;
    do {
      uVar9 = *puVar12 >> 0x10 | *puVar12 << 0x10;
      uVar9 = (uVar9 >> 8 ^ uVar9 << 8) & 0xff00ff ^ uVar9 << 8;
      uVar9 = (uVar9 >> 4 ^ uVar9 << 4) & 0xf0f0f0f ^ uVar9 << 4;
      uVar9 = (uVar9 >> 2 ^ uVar9 * 4) & 0x33333333 ^ uVar9 << 2;
      *puVar12 = (uVar9 >> 1 ^ uVar9 * 2) & 0x55555555 ^ uVar9 * 2;
      *(uint **)(&stack0xffffffdc + (iVar8 - (int)puVar4) + (int)puVar12) = puVar12;
      puVar12 = puVar12 + 1;
      param_1 = (undefined4 *)((int)param_1 + -1);
    } while (param_1 != (undefined4 *)0x0);
  }
  *(undefined1 **)(&stack0xffffffd8 + iVar8) = &LAB_004987f0;
  *(undefined4 *)(&stack0xffffffd4 + iVar8) = 4;
  *(undefined4 **)(&stack0xffffffd0 + iVar8) = puVar14;
  asStackY_40[3 - (int)puVar14] = (size_t)(&stack0xffffffdc + iVar8);
  asStackY_40[2 - (int)puVar14] = 0x498d21;
  CRT_qsort((undefined1 *)asStackY_40[3 - (int)puVar14],*(uint *)(&stack0xffffffd0 + iVar8),
            *(uint *)(&stack0xffffffd4 + iVar8),*(undefined **)(&stack0xffffffd8 + iVar8));
  iVar11 = (int)puVar14 * -8;
  piVar7 = (int *)(&stack0xffffffdc + iVar11);
  *(undefined4 *)(&stack0xffffffd8 + iVar8) = 0x498d32;
  *(int *)(&stack0xffffffd8 + (int)puVar14 * -8) = (int)puVar14 * 4;
  *(undefined4 *)(&stack0xffffffd4 + (int)puVar14 * -8) = 0x498d3e;
  pvVar5 = _malloc(*(size_t *)(&stack0xffffffd8 + (int)puVar14 * -8));
  puVar3[5] = pvVar5;
  iVar6 = 0;
  if (0 < (int)puVar14) {
    do {
      *(int *)(&stack0xffffffdc +
              (*(int *)(&stack0xffffffdc + iVar6 * 4 + iVar8) - (int)puVar4 >> 2) * 4 + iVar11) =
           iVar6;
      iVar6 = iVar6 + 1;
    } while (iVar6 < (int)puVar14);
    if (0 < (int)puVar14) {
      local_10 = puVar14;
      do {
        *(undefined4 *)(puVar3[5] + *piVar7 * 4) =
             *(undefined4 *)(((int)puVar4 - (int)(&stack0xffffffdc + iVar11)) + (int)piVar7);
        piVar7 = piVar7 + 1;
        local_10 = (undefined4 *)((int)local_10 + -1);
      } while (local_10 != (undefined4 *)0x0);
    }
  }
  *(uint **)(&stack0xffffffd8 + (int)puVar14 * -8) = puVar4;
  *(undefined4 *)(&stack0xffffffd4 + (int)puVar14 * -8) = 0x498da3;
  _free(*(void **)(&stack0xffffffd8 + (int)puVar14 * -8));
  *(undefined4 **)(&stack0xffffffd4 + (int)puVar14 * -8) = puVar3 + 3;
  *(undefined1 **)(&stack0xffffffd0 + (int)puVar14 * -8) = &stack0xffffffdc + iVar11;
  asStackY_40[(int)puVar14 * -2 + 3] = (size_t)puVar14;
  asStackY_40[(int)puVar14 * -2 + 2] = (size_t)param_2;
  asStackY_40[(int)puVar14 * -2 + 1] = 0x498db2;
  piVar7 = Huffman_BuildCodes((int *)asStackY_40[(int)puVar14 * -2 + 2],
                              asStackY_40[(int)puVar14 * -2 + 3],
                              *(int *)(&stack0xffffffd0 + (int)puVar14 * -8),
                              *(int **)(&stack0xffffffd4 + (int)puVar14 * -8));
  asStackY_40[(int)puVar14 * -2 + 1] = (int)puVar14 * 4;
  puVar3[4] = piVar7;
  asStackY_40[(int)puVar14 * -2] = 0x498dbe;
  pvVar5 = _malloc(asStackY_40[(int)puVar14 * -2 + 1]);
  puVar3[6] = pvVar5;
  iVar6 = 0;
  iVar8 = 0;
  if (0 < (int)param_2[1]) {
    do {
      if (0 < *(int *)(param_2[2] + iVar8 * 4)) {
        *(int *)(puVar3[6] + *(int *)(&stack0xffffffdc + iVar6 * 4 + iVar11) * 4) = iVar8;
        iVar6 = iVar6 + 1;
      }
      iVar8 = iVar8 + 1;
    } while (iVar8 < (int)param_2[1]);
  }
  *(int *)(&stack0xffffffd8 + (int)puVar14 * -8) = iVar6;
  *(undefined4 *)(&stack0xffffffd4 + (int)puVar14 * -8) = 0x498df4;
  pvVar5 = _malloc(*(size_t *)(&stack0xffffffd8 + (int)puVar14 * -8));
  puVar3[7] = pvVar5;
  piVar7 = param_2 + 1;
  iVar8 = 0;
  param_2 = (undefined4 *)0x0;
  if (0 < *piVar7) {
    do {
      iVar6 = puVar13[2];
      if (0 < *(int *)(iVar6 + iVar8 * 4)) {
        *(undefined1 *)(*(int *)(&stack0xffffffdc + (int)param_2 * 4 + iVar11) + puVar3[7]) =
             *(undefined1 *)(iVar6 + iVar8 * 4);
        param_2 = (undefined4 *)((int)param_2 + 1);
      }
      iVar8 = iVar8 + 1;
    } while (iVar8 < (int)puVar13[1]);
  }
  iVar8 = 0;
  for (uVar9 = puVar3[2]; uVar9 != 0; uVar9 = uVar9 >> 1) {
    iVar8 = iVar8 + 1;
  }
  puVar3[9] = iVar8 + -4;
  if (iVar8 + -4 < 5) {
    puVar3[9] = 5;
  }
  if (8 < (int)puVar3[9]) {
    puVar3[9] = 8;
  }
  iVar8 = 1 << ((byte)puVar3[9] & 0x1f);
  *(undefined4 *)(&stack0xffffffd8 + (int)puVar14 * -8) = 4;
  *(int *)(&stack0xffffffd4 + (int)puVar14 * -8) = iVar8;
  *(undefined4 *)(&stack0xffffffd0 + (int)puVar14 * -8) = 0x498e7d;
  pvVar5 = _calloc(*(size_t *)(&stack0xffffffd4 + (int)puVar14 * -8),
                   *(size_t *)(&stack0xffffffd8 + (int)puVar14 * -8));
  puVar3[8] = pvVar5;
  puVar3[10] = 0;
  local_8 = 0;
  if (0 < (int)param_2) {
    do {
      iVar11 = (int)*(char *)(puVar3[7] + local_8);
      if ((int)puVar3[10] < iVar11) {
        puVar3[10] = iVar11;
      }
      cVar1 = *(char *)(puVar3[7] + local_8);
      if ((int)cVar1 <= (int)puVar3[9]) {
        uVar9 = *(uint *)(puVar3[5] + local_8 * 4);
        uVar9 = uVar9 >> 0x10 | uVar9 << 0x10;
        uVar9 = (uVar9 >> 8 ^ uVar9 << 8) & 0xff00ff ^ uVar9 << 8;
        uVar9 = (uVar9 >> 4 ^ uVar9 << 4) & 0xf0f0f0f ^ uVar9 << 4;
        uVar9 = (uVar9 >> 2 ^ uVar9 * 4) & 0x33333333 ^ uVar9 << 2;
        iVar11 = 0;
        if (0 < 1 << ((char)puVar3[9] - cVar1 & 0x1fU)) {
          do {
            *(int *)(puVar3[8] +
                    (iVar11 << (*(byte *)(local_8 + puVar3[7]) & 0x1f) |
                    (uVar9 >> 1 ^ uVar9 * 2) & 0x55555555 ^ uVar9 * 2) * 4) = local_8 + 1;
            iVar11 = iVar11 + 1;
          } while (iVar11 < 1 << ((char)puVar3[9] - *(char *)(local_8 + puVar3[7]) & 0x1fU));
        }
      }
      local_8 = local_8 + 1;
    } while (local_8 < (int)param_2);
  }
  uVar2 = puVar3[9];
  param_1 = (undefined4 *)0x0;
  local_10 = (undefined4 *)0x0;
  local_8 = 0;
  if (0 < iVar8) {
    do {
      uVar15 = local_8 << (0x20U - (char)puVar3[9] & 0x1f);
      uVar10 = uVar15 >> 0x10 | uVar15 << 0x10;
      uVar9 = (uVar10 >> 8 ^ uVar10 << 8) & 0xff00ff ^ uVar10 << 8;
      uVar9 = (uVar9 >> 4 ^ uVar9 << 4) & 0xf0f0f0f ^ uVar9 << 4;
      uVar9 = (uVar9 >> 2 ^ uVar9 * 4) & 0x33333333 ^ uVar9 << 2;
      if (*(int *)(puVar3[8] + ((uVar9 >> 1 ^ uVar9 * 2) & 0x55555555 ^ uVar9 * 2) * 4) == 0) {
        iVar11 = (int)param_1 + 1;
        if (iVar11 < (int)param_2) {
          local_c = (uint *)(puVar3[5] + 4 + (int)param_1 * 4);
          do {
            if (uVar15 < *local_c) break;
            param_1 = (undefined4 *)((int)param_1 + 1);
            local_c = local_c + 1;
            iVar11 = iVar11 + 1;
          } while (iVar11 < (int)param_2);
        }
        if ((int)local_10 < (int)param_2) {
          local_c = (uint *)(puVar3[5] + (int)local_10 * 4);
          do {
            if (uVar15 < (*local_c & -2 << (0x1fU - (char)uVar2 & 0x1f))) break;
            local_10 = (undefined4 *)((int)local_10 + 1);
            local_c = local_c + 1;
          } while ((int)local_10 < (int)param_2);
        }
        uVar9 = (int)param_2 - (int)local_10;
        puVar13 = param_1;
        if ((undefined4 *)0x7fff < param_1) {
          puVar13 = (undefined4 *)0x7fff;
        }
        if (0x7fff < uVar9) {
          uVar9 = 0x7fff;
        }
        uVar10 = (uVar10 >> 8 ^ uVar10 << 8) & 0xff00ff ^ uVar10 << 8;
        uVar10 = (uVar10 >> 4 ^ uVar10 << 4) & 0xf0f0f0f ^ uVar10 << 4;
        uVar10 = (uVar10 >> 2 ^ uVar10 * 4) & 0x33333333 ^ uVar10 << 2;
        *(uint *)(puVar3[8] + ((uVar10 >> 1 ^ uVar10 * 2) & 0x55555555 ^ uVar10 * 2) * 4) =
             ((uint)puVar13 | 0xffff0000) << 0xf | uVar9;
      }
      local_8 = local_8 + 1;
    } while (local_8 < iVar8);
  }
  return 0;
}

