
void __cdecl
D3D_ConvertYCbCrAlphaToRGB(int param_1,int *param_2,int param_3,undefined4 *param_4,int param_5)

{
  int *piVar1;
  int *piVar2;
  int *piVar3;
  byte bVar4;
  byte bVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  undefined1 *puVar12;
  byte *pbVar13;
  int iVar14;
  int iVar15;
  uint uVar16;
  int iVar17;
  int iVar18;
  int local_c;
  int local_8;
  
  iVar6 = *(int *)(param_1 + 0x1a0);
  iVar7 = *(int *)(param_1 + 0x5c);
  iVar8 = *(int *)(param_1 + 0x11c);
  iVar9 = *(int *)(iVar6 + 8);
  iVar10 = *(int *)(iVar6 + 0xc);
  iVar11 = *(int *)(iVar6 + 0x10);
  iVar6 = *(int *)(iVar6 + 0x14);
  if (-1 < param_5 + -1) {
    iVar14 = param_3 << 2;
    local_c = param_5;
    do {
      piVar1 = (int *)(iVar14 + *param_2);
      pbVar13 = *(byte **)(iVar14 + param_2[1]);
      piVar2 = (int *)(iVar14 + param_2[2]);
      piVar3 = (int *)(iVar14 + param_2[3]);
      iVar14 = iVar14 + 4;
      if (iVar7 != 0) {
        iVar17 = *piVar1 - (int)pbVar13;
        iVar18 = *piVar2 - (int)pbVar13;
        iVar15 = *piVar3 - (int)pbVar13;
        puVar12 = (undefined1 *)*param_4;
        local_8 = iVar7;
        do {
          uVar16 = (uint)pbVar13[iVar17];
          bVar4 = *pbVar13;
          bVar5 = pbVar13[iVar18];
          *puVar12 = *(undefined1 *)(((iVar8 - *(int *)((uint)bVar5 * 4 + iVar9)) - uVar16) + 0xff);
          puVar12[1] = *(undefined1 *)
                        (((iVar8 - (*(int *)((uint)bVar4 * 4 + iVar6) +
                                    *(int *)((uint)bVar5 * 4 + iVar11) >> 0x10)) - uVar16) + 0xff);
          puVar12[2] = *(undefined1 *)
                        (((iVar8 - *(int *)((uint)bVar4 * 4 + iVar10)) - uVar16) + 0xff);
          puVar12[3] = pbVar13[iVar15];
          pbVar13 = pbVar13 + 1;
          local_8 = local_8 + -1;
          puVar12 = puVar12 + 4;
        } while (local_8 != 0);
      }
      local_c = local_c + -1;
      param_4 = param_4 + 1;
    } while (local_c != 0);
  }
  return;
}

