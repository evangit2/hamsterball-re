
void __cdecl Huffman_BuildTable(int param_1,int param_2,int *param_3)

{
  char cVar1;
  int iVar2;
  int iVar3;
  char *pcVar4;
  int *piVar5;
  undefined4 *puVar6;
  uint uVar7;
  uint uVar8;
  int iVar9;
  undefined1 *puVar10;
  int iVar11;
  undefined1 *puVar12;
  int aiStack_520 [257];
  char local_11c [260];
  byte *local_18;
  int local_14;
  int local_10;
  uint local_c;
  int local_8;
  
  if (*param_3 == 0) {
    iVar2 = (*(code *)**(undefined4 **)(param_1 + 4))(param_1,1,0x5d4);
    *param_3 = iVar2;
  }
  iVar2 = *param_3;
  iVar3 = 0;
  *(int *)(iVar2 + 0xd0) = param_2;
  local_8 = 0;
  param_3 = (int *)0x1;
  do {
    uVar7 = (uint)*(byte *)((int)param_3 + param_2);
    local_c = uVar7;
    if (uVar7 != 0) {
      pcVar4 = local_11c + iVar3;
      for (uVar8 = (uint)(*(byte *)((int)param_3 + param_2) >> 2); uVar8 != 0; uVar8 = uVar8 - 1) {
        *(uint *)pcVar4 =
             CONCAT22(CONCAT11((char)param_3,(char)param_3),CONCAT11((char)param_3,(char)param_3));
        pcVar4 = pcVar4 + 4;
      }
      for (uVar8 = uVar7 & 3; uVar8 != 0; uVar8 = uVar8 - 1) {
        *pcVar4 = (char)param_3;
        pcVar4 = pcVar4 + 1;
      }
      iVar3 = local_8 + uVar7;
      local_8 = iVar3;
    }
    param_3 = (int *)((int)param_3 + 1);
  } while ((int)param_3 < 0x11);
  local_11c[iVar3] = '\0';
  iVar11 = (int)local_11c[0];
  iVar3 = 0;
  iVar9 = 0;
  if (local_11c[0] != '\0') {
    pcVar4 = local_11c;
    do {
      cVar1 = *pcVar4;
      while (cVar1 == iVar11) {
        cVar1 = local_11c[iVar3 + 1];
        aiStack_520[iVar3] = iVar9;
        iVar3 = iVar3 + 1;
        iVar9 = iVar9 + 1;
      }
      iVar9 = iVar9 << 1;
      pcVar4 = local_11c + iVar3;
      iVar11 = iVar11 + 1;
    } while (*pcVar4 != '\0');
  }
  iVar3 = 0;
  iVar9 = 1;
  piVar5 = (int *)(iVar2 + 0x48);
  do {
    if (*(char *)(iVar9 + param_2) == '\0') {
      *piVar5 = -1;
    }
    else {
      iVar11 = aiStack_520[iVar3];
      piVar5[0x12] = iVar3;
      piVar5[-0x11] = iVar11;
      iVar3 = iVar3 + (uint)*(byte *)(iVar9 + param_2);
      *piVar5 = aiStack_520[iVar3 + -1];
    }
    iVar9 = iVar9 + 1;
    piVar5 = piVar5 + 1;
  } while (iVar9 < 0x11);
  local_8 = 0;
  *(undefined4 *)(iVar2 + 0x88) = 0xfffff;
  puVar6 = (undefined4 *)(iVar2 + 0xd4);
  for (iVar3 = 0x100; iVar3 != 0; iVar3 = iVar3 + -1) {
    *puVar6 = 0;
    puVar6 = puVar6 + 1;
  }
  param_3 = (int *)0x1;
  local_10 = 7;
  do {
    local_18 = (byte *)((int)param_3 + param_2);
    local_14 = 1;
    if (*local_18 != 0) {
      iVar3 = 1 << ((byte)local_10 & 0x1f);
      puVar10 = (undefined1 *)(local_8 + 0x11 + param_2);
      do {
        iVar9 = aiStack_520[local_8] << ((byte)local_10 & 0x1f);
        if (0 < iVar3) {
          puVar12 = (undefined1 *)(iVar9 + 0x4d4 + iVar2);
          puVar6 = (undefined4 *)(iVar2 + 0xd4 + iVar9 * 4);
          local_c = iVar3;
          do {
            *puVar6 = param_3;
            puVar6 = puVar6 + 1;
            *puVar12 = *puVar10;
            puVar12 = puVar12 + 1;
            local_c = local_c + -1;
          } while (local_c != 0);
        }
        local_14 = local_14 + 1;
        local_8 = local_8 + 1;
        puVar10 = puVar10 + 1;
      } while (local_14 <= (int)(uint)*local_18);
    }
    param_3 = (int *)((int)param_3 + 1);
    local_10 = local_10 + -1;
  } while (-1 < local_10);
  return;
}

