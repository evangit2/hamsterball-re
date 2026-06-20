
undefined4 __cdecl Image_ParseAdobeMarker(int *param_1)

{
  byte *pbVar1;
  int iVar2;
  byte bVar3;
  undefined4 *puVar4;
  int *piVar5;
  int *piVar6;
  uint uVar7;
  byte *pbVar8;
  byte local_18 [4];
  char local_14;
  undefined1 local_13;
  undefined1 local_12;
  undefined1 local_11;
  undefined1 local_10;
  undefined1 local_f;
  undefined1 local_e;
  byte local_d;
  int local_c;
  int local_8;
  
  piVar5 = param_1;
  puVar4 = (undefined4 *)param_1[5];
  pbVar8 = (byte *)*puVar4;
  piVar6 = (int *)puVar4[1];
  if ((int *)puVar4[1] == (int *)0x0) {
    uVar7 = (*(code *)puVar4[3])(param_1);
    if ((char)uVar7 == '\0') goto LAB_004a365f;
    pbVar8 = (byte *)*puVar4;
    piVar6 = (int *)puVar4[1];
  }
  param_1 = piVar6;
  param_1 = (int *)((int)param_1 + -1);
  local_8 = (uint)*pbVar8 << 8;
  pbVar8 = pbVar8 + 1;
  if (param_1 == (int *)0x0) {
    uVar7 = (*(code *)puVar4[3])(piVar5);
    if ((char)uVar7 == '\0') {
LAB_004a365f:
      return uVar7 & 0xffffff00;
    }
    param_1 = (int *)puVar4[1];
    pbVar8 = (byte *)*puVar4;
  }
  param_1 = (int *)((int)param_1 + -1);
  local_8 = local_8 + (uint)*pbVar8 + -2;
  pbVar8 = pbVar8 + 1;
  if (local_8 < 0xc) {
    *(undefined4 *)(*piVar5 + 0x14) = 0x4d;
    *(int *)(*piVar5 + 0x18) = local_8;
  }
  else {
    local_c = 0;
    do {
      if (param_1 == (int *)0x0) {
        uVar7 = (*(code *)puVar4[3])(piVar5);
        if ((char)uVar7 == '\0') goto LAB_004a365f;
        param_1 = (int *)puVar4[1];
        pbVar8 = (byte *)*puVar4;
      }
      param_1 = (int *)((int)param_1 + -1);
      bVar3 = *pbVar8;
      pbVar8 = pbVar8 + 1;
      iVar2 = local_c + 1;
      pbVar1 = local_18 + local_c;
      local_c = iVar2;
      *pbVar1 = bVar3;
    } while (iVar2 < 0xc);
    if ((((local_18[0] == 0x41) && (local_18[1] == 'd')) && (local_18[2] == 'o')) &&
       ((local_18[3] == 'b' && (local_14 == 'e')))) {
      iVar2 = *piVar5;
      *(uint *)(iVar2 + 0x18) = (uint)CONCAT11(local_13,local_12);
      *(uint *)(iVar2 + 0x1c) = (uint)CONCAT11(local_11,local_10);
      *(uint *)(iVar2 + 0x20) = (uint)CONCAT11(local_f,local_e);
      *(uint *)(iVar2 + 0x24) = (uint)local_d;
      *(undefined4 *)(*piVar5 + 0x14) = 0x4b;
      local_8 = local_8 + -0xc;
      (**(code **)(*piVar5 + 4))(piVar5,1);
      *(undefined1 *)((int)piVar5 + 0x106) = 1;
      *(byte *)((int)piVar5 + 0x107) = local_d;
      goto LAB_004a3692;
    }
    *(undefined4 *)(*piVar5 + 0x14) = 0x4d;
    *(int *)(*piVar5 + 0x18) = local_8;
    local_8 = local_8 + -0xc;
  }
  (**(code **)(*piVar5 + 4))(piVar5,1);
LAB_004a3692:
  *puVar4 = pbVar8;
  puVar4[1] = param_1;
  if (0 < local_8) {
    param_1 = (int *)(**(code **)(piVar5[5] + 0x10))(piVar5,local_8);
  }
  return CONCAT31((int3)((uint)param_1 >> 8),1);
}

