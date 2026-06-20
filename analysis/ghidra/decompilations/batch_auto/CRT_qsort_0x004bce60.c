
void __cdecl CRT_qsort(undefined1 *param_1,uint param_2,uint param_3,undefined *param_4)

{
  undefined1 uVar1;
  uint uVar2;
  undefined1 *puVar3;
  int iVar4;
  undefined1 *puVar5;
  undefined1 *puVar6;
  undefined1 *puVar7;
  undefined1 *local_100;
  undefined1 *local_fc;
  int local_f8;
  uint uStack_f4;
  undefined4 auStack_f0 [30];
  undefined4 auStack_78 [30];
  
  if ((param_2 < 2) || (param_3 == 0)) {
    return;
  }
  local_f8 = 0;
  local_100 = param_1;
  local_fc = param_1 + (param_2 - 1) * param_3;
LAB_004bcea5:
  while (uVar2 = (uint)((int)local_fc - (int)local_100) / param_3 + 1, 8 < uVar2) {
    puVar3 = local_100 + (uVar2 >> 1) * param_3;
    iVar4 = (*(code *)param_4)(local_100,puVar3);
    if ((0 < iVar4) && (local_100 != puVar3)) {
      puVar5 = puVar3;
      uVar2 = param_3;
      do {
        uVar1 = puVar5[(int)local_100 - (int)puVar3];
        puVar5[(int)local_100 - (int)puVar3] = *puVar5;
        *puVar5 = uVar1;
        puVar5 = puVar5 + 1;
        uVar2 = uVar2 - 1;
      } while (uVar2 != 0);
    }
    iVar4 = (*(code *)param_4)(local_100,local_fc);
    if ((0 < iVar4) && (local_100 != local_fc)) {
      puVar5 = local_fc;
      uVar2 = param_3;
      do {
        uVar1 = puVar5[(int)local_100 - (int)local_fc];
        puVar5[(int)local_100 - (int)local_fc] = *puVar5;
        *puVar5 = uVar1;
        puVar5 = puVar5 + 1;
        uVar2 = uVar2 - 1;
      } while (uVar2 != 0);
    }
    iVar4 = (*(code *)param_4)(puVar3,local_fc);
    puVar5 = local_100;
    puVar7 = local_fc;
    if ((0 < iVar4) && (puVar3 != local_fc)) {
      puVar6 = local_fc;
      uVar2 = param_3;
      do {
        uVar1 = puVar6[(int)puVar3 - (int)local_fc];
        puVar6[(int)puVar3 - (int)local_fc] = *puVar6;
        *puVar6 = uVar1;
        puVar6 = puVar6 + 1;
        uVar2 = uVar2 - 1;
      } while (uVar2 != 0);
    }
LAB_004bcf90:
    if (puVar5 < puVar3) {
      do {
        puVar5 = puVar5 + param_3;
        if (puVar3 <= puVar5) goto LAB_004bcfb0;
        iVar4 = (*(code *)param_4)(puVar5,puVar3);
      } while (iVar4 < 1);
      if (puVar3 <= puVar5) goto LAB_004bcfb0;
    }
    else {
LAB_004bcfb0:
      do {
        puVar5 = puVar5 + param_3;
        if (local_fc < puVar5) break;
        iVar4 = (*(code *)param_4)(puVar5,puVar3);
      } while (iVar4 < 1);
    }
    do {
      puVar7 = puVar7 + -param_3;
      if (puVar7 <= puVar3) break;
      iVar4 = (*(code *)param_4)(puVar7,puVar3);
    } while (0 < iVar4);
    if (puVar5 <= puVar7) {
      if (puVar5 != puVar7) {
        uStack_f4 = param_3;
        puVar6 = puVar7;
        do {
          uVar1 = puVar6[(int)puVar5 - (int)puVar7];
          puVar6[(int)puVar5 - (int)puVar7] = *puVar6;
          *puVar6 = uVar1;
          puVar6 = puVar6 + 1;
          uStack_f4 = uStack_f4 - 1;
        } while (uStack_f4 != 0);
      }
      if (puVar3 == puVar7) {
        puVar3 = puVar5;
      }
      goto LAB_004bcf90;
    }
    puVar7 = puVar7 + param_3;
    if (puVar3 < puVar7) {
      do {
        puVar7 = puVar7 + -param_3;
        if (puVar7 <= puVar3) goto LAB_004bd050;
        iVar4 = (*(code *)param_4)(puVar7,puVar3);
      } while (iVar4 == 0);
      if (puVar7 <= puVar3) goto LAB_004bd050;
    }
    else {
LAB_004bd050:
      do {
        puVar7 = puVar7 + -param_3;
        if (puVar7 <= local_100) break;
        iVar4 = (*(code *)param_4)(puVar7,puVar3);
      } while (iVar4 == 0);
    }
    if ((int)puVar7 - (int)local_100 < (int)local_fc - (int)puVar5) goto LAB_004bd0ab;
    if (local_100 < puVar7) {
      auStack_f0[local_f8] = local_100;
      auStack_78[local_f8] = puVar7;
      local_f8 = local_f8 + 1;
    }
    local_100 = puVar5;
    if (local_fc <= puVar5) goto LAB_004bcec7;
  }
  shortsort(local_100,param_3,param_4);
  goto LAB_004bcec7;
LAB_004bd0ab:
  if (puVar5 < local_fc) {
    auStack_f0[local_f8] = puVar5;
    auStack_78[local_f8] = local_fc;
    local_f8 = local_f8 + 1;
  }
  local_fc = puVar7;
  if (puVar7 <= local_100) {
LAB_004bcec7:
    local_f8 = local_f8 + -1;
    if (local_f8 < 0) {
      return;
    }
    local_100 = (undefined1 *)auStack_f0[local_f8];
    local_fc = (undefined1 *)auStack_78[local_f8];
  }
  goto LAB_004bcea5;
}

