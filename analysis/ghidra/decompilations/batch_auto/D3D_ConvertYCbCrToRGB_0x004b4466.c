
void __cdecl
D3D_ConvertYCbCrToRGB(int param_1,int *param_2,int param_3,undefined4 *param_4,int param_5)

{
  int *piVar1;
  int *piVar2;
  byte bVar3;
  byte bVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  undefined4 *puVar11;
  undefined4 *puVar12;
  undefined4 *puVar13;
  uint *puVar14;
  int iVar15;
  byte *pbVar16;
  undefined1 *puVar17;
  uint uVar18;
  int iVar19;
  int iVar20;
  uint uVar21;
  uint local_1c;
  int local_18;
  
  iVar5 = *(int *)(param_1 + 0x1a0);
  iVar6 = *(int *)(param_1 + 0x11c);
  uVar7 = *(uint *)(param_1 + 0x5c);
  iVar8 = *(int *)(iVar5 + 8);
  iVar9 = *(int *)(iVar5 + 0xc);
  iVar10 = *(int *)(iVar5 + 0x10);
  iVar5 = *(int *)(iVar5 + 0x14);
  uVar21 = uVar7 & 0xfffffff8;
  if (DAT_004fb258 == '\0') {
    if (-1 < param_5 + -1) {
      iVar15 = param_3 << 2;
      local_18 = param_5;
      do {
        piVar1 = (int *)(iVar15 + *param_2);
        pbVar16 = *(byte **)(iVar15 + param_2[1]);
        piVar2 = (int *)(iVar15 + param_2[2]);
        iVar15 = iVar15 + 4;
        if (uVar7 != 0) {
          iVar19 = *piVar1 - (int)pbVar16;
          iVar20 = *piVar2 - (int)pbVar16;
          puVar17 = (undefined1 *)*param_4;
          local_1c = uVar7;
          do {
            bVar3 = pbVar16[iVar20];
            uVar21 = (uint)pbVar16[iVar19];
            bVar4 = *pbVar16;
            *puVar17 = *(undefined1 *)(*(int *)((uint)bVar3 * 4 + iVar8) + uVar21 + iVar6);
            puVar17[1] = *(undefined1 *)
                          ((*(int *)((uint)bVar4 * 4 + iVar5) + *(int *)((uint)bVar3 * 4 + iVar10)
                           >> 0x10) + uVar21 + iVar6);
            pbVar16 = pbVar16 + 1;
            local_1c = local_1c - 1;
            puVar17[2] = *(undefined1 *)(*(int *)((uint)bVar4 * 4 + iVar9) + uVar21 + iVar6);
            puVar17 = puVar17 + 3;
          } while (local_1c != 0);
        }
        local_18 = local_18 + -1;
        param_4 = param_4 + 1;
      } while (local_18 != 0);
    }
  }
  else if (-1 < param_5 + -1) {
    iVar15 = param_3 << 2;
    local_1c = param_5;
    do {
      puVar11 = *(undefined4 **)(iVar15 + *param_2);
      puVar12 = *(undefined4 **)(iVar15 + param_2[1]);
      puVar13 = *(undefined4 **)(iVar15 + param_2[2]);
      iVar15 = iVar15 + 4;
      puVar14 = (uint *)*param_4;
      D3D_ConvertYCbCrToRGB_SSE2(uVar21,puVar11,puVar12,puVar13,puVar14);
      if (uVar21 < uVar7) {
        pbVar16 = (byte *)((int)puVar12 + uVar21);
        local_18 = uVar7 - uVar21;
        puVar17 = (undefined1 *)((int)puVar14 + uVar21 * 3);
        do {
          bVar3 = pbVar16[(int)puVar13 - (int)puVar12];
          uVar18 = (uint)pbVar16[(int)puVar11 - (int)puVar12];
          bVar4 = *pbVar16;
          *puVar17 = *(undefined1 *)(*(int *)((uint)bVar3 * 4 + iVar8) + uVar18 + iVar6);
          puVar17[1] = *(undefined1 *)
                        ((*(int *)((uint)bVar4 * 4 + iVar5) + *(int *)((uint)bVar3 * 4 + iVar10) >>
                         0x10) + uVar18 + iVar6);
          pbVar16 = pbVar16 + 1;
          local_18 = local_18 + -1;
          puVar17[2] = *(undefined1 *)(*(int *)((uint)bVar4 * 4 + iVar9) + uVar18 + iVar6);
          puVar17 = puVar17 + 3;
        } while (local_18 != 0);
      }
      local_1c = local_1c + -1;
      param_4 = param_4 + 1;
    } while (local_1c != 0);
  }
  return;
}

