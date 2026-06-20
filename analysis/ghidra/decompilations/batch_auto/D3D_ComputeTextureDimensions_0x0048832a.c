
undefined4 __fastcall
D3D_ComputeTextureDimensions
          (undefined4 param_1,uint param_2,int *param_3,uint *param_4,uint *param_5,uint *param_6,
          uint *param_7,int *param_8,int param_9,int param_10)

{
  int *piVar1;
  int *piVar2;
  int *piVar3;
  int *piVar4;
  int *piVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  undefined1 local_e0 [60];
  uint local_a4;
  int *local_88;
  int *local_84;
  int *local_80;
  int local_78;
  int local_c;
  int *local_8;
  
  piVar4 = param_3;
  if (param_3 == (int *)0x0) {
    return 0x8876086c;
  }
  if (param_2 == 0xffffffff) {
    param_2 = 0;
  }
  if (param_2 != (param_2 & 0x20b)) {
    return 0x8876086c;
  }
  if ((((param_9 != 0) && (param_9 != 1)) && (param_9 != 2)) && (param_9 != 3)) {
    return 0x8876086c;
  }
  if (param_8 == (int *)0x0) {
    local_c = 0;
  }
  else {
    local_c = *param_8;
  }
  if (param_9 == 3) {
    if (local_c == 0) {
      local_c = 0x15;
    }
  }
  else {
    piVar2 = D3D_FindDisplayMode(local_c);
    local_c = D3D_SelectBestAdapterMode(param_3,param_2,param_10,piVar2);
    if (local_c == 0) {
      return 0x8876086a;
    }
  }
  if (param_4 == (uint *)0x0) {
    param_3 = (int *)0xffffffff;
  }
  else {
    param_3 = (int *)*param_4;
  }
  if (param_5 == (uint *)0x0) {
    piVar2 = (int *)0xffffffff;
  }
  else {
    piVar2 = (int *)*param_5;
  }
  if (param_6 == (uint *)0x0) {
    local_8 = (int *)0xffffffff;
  }
  else {
    local_8 = (int *)*param_6;
  }
  if (param_3 == (int *)0xffffffff) {
    param_3 = piVar2;
    if (piVar2 != (int *)0xffffffff) goto LAB_00488415;
    piVar2 = (int *)&DAT_00000100;
    param_3 = (int *)&DAT_00000100;
  }
  else {
    if (piVar2 == (int *)0xffffffff) {
      piVar2 = param_3;
    }
LAB_00488415:
    if (param_3 == (int *)0x0) {
      param_3 = (int *)0x1;
    }
    if (piVar2 == (int *)0x0) {
      piVar2 = (int *)0x1;
    }
  }
  piVar5 = param_3;
  if ((param_10 == 5) && (piVar5 = piVar2, piVar2 < param_3)) {
    piVar2 = param_3;
    piVar5 = param_3;
  }
  param_3 = piVar5;
  if ((local_8 == (int *)0xffffffff) || (local_8 == (int *)0x0)) {
    local_8 = (int *)0x1;
  }
  piVar5 = piVar2;
  if (param_9 == 3) goto LAB_00488526;
  (**(code **)(*piVar4 + 0x1c))(piVar4,local_e0);
  if (param_10 == 4) {
    if (local_80 < local_8) {
      local_8 = local_80;
    }
    if (local_80 < param_3) {
      param_3 = local_80;
    }
LAB_004884be:
    if (local_80 < piVar2) {
      piVar2 = local_80;
    }
  }
  else {
    if (local_88 < param_3) {
      param_3 = local_88;
    }
    if (local_84 < piVar2) {
      piVar2 = local_84;
    }
    if (param_10 == 3) {
      if ((local_a4 & 0x20) != 0) {
        local_78 = 1;
      }
      if (local_78 != 0) {
        if ((int *)(local_78 * (int)piVar2) < param_3) {
          param_3 = (int *)(local_78 * (int)piVar2);
        }
        local_80 = (int *)(local_78 * (int)param_3);
        goto LAB_004884be;
      }
    }
  }
  piVar1 = local_8;
  piVar4 = param_3;
  if (param_10 == 3) {
    piVar3 = (int *)0x2;
  }
  else if (param_10 == 4) {
    piVar3 = (int *)0x40000;
  }
  else {
    piVar3 = param_3;
    if (param_10 == 5) {
      piVar3 = (int *)0x20000;
    }
  }
  piVar5 = piVar2;
  if ((local_a4 & (uint)piVar3) != 0) {
    piVar5 = (int *)0x1;
    param_3 = (int *)0x1;
    if ((int *)0x1 < piVar4) {
      do {
        param_3 = (int *)((int)param_3 << 1);
      } while (param_3 < piVar4);
    }
    if ((int *)0x1 < piVar2) {
      do {
        piVar5 = (int *)((int)piVar5 << 1);
      } while (piVar5 < piVar2);
    }
    local_8 = (int *)0x1;
    if ((int *)0x1 < piVar1) {
      do {
        local_8 = (int *)((int)local_8 << 1);
      } while (local_8 < piVar1);
    }
  }
LAB_00488526:
  if ((((local_c == 0x31545844) || (local_c == 0x32545844)) || (local_c == 0x33545844)) ||
     ((local_c == 0x34545844 || (local_c == 0x35545844)))) {
    param_3 = (int *)((int)param_3 + 3U & 0xfffffffc);
    piVar5 = (int *)((uint)((int)piVar5 + 3) & 0xfffffffc);
  }
  if (param_10 == 3) {
    piVar4 = (int *)0x4000;
  }
  else if (param_10 == 4) {
    piVar4 = (int *)0x8000;
  }
  else {
    piVar4 = param_3;
    if (param_10 == 5) {
      piVar4 = (int *)0x10000;
    }
  }
  if (param_7 == (uint *)0x0) {
    uVar9 = 0xffffffff;
  }
  else {
    uVar9 = *param_7;
  }
  if ((param_9 == 3) ||
     (((local_a4 & (uint)piVar4) != 0 &&
      (((local_a4 & 0x100) == 0 ||
       (((((uint)param_3 & (int)param_3 - 1U) == 0 &&
         (((uint)piVar5 & (uint)((int)piVar5 + -1)) == 0)) &&
        (((uint)local_8 & (uint)((int)local_8 + -1)) == 0)))))))) {
    uVar8 = 0;
    for (piVar4 = param_3; piVar4 != (int *)0x0; piVar4 = (int *)((uint)piVar4 >> 1)) {
      uVar8 = uVar8 + 1;
    }
    uVar6 = 0;
    for (piVar4 = piVar5; piVar4 != (int *)0x0; piVar4 = (int *)((uint)piVar4 >> 1)) {
      uVar6 = uVar6 + 1;
    }
    uVar7 = 0;
    for (piVar4 = local_8; piVar4 != (int *)0x0; piVar4 = (int *)((uint)piVar4 >> 1)) {
      uVar7 = uVar7 + 1;
    }
    if (uVar8 <= uVar6) {
      uVar8 = uVar6;
    }
    if ((param_10 == 4) && (uVar8 < uVar7)) {
      uVar8 = uVar7;
    }
    if ((uVar8 < uVar9) || (uVar9 == 0)) {
      uVar9 = uVar8;
    }
  }
  else {
    uVar9 = 1;
  }
  if (param_4 != (uint *)0x0) {
    *param_4 = (uint)param_3;
  }
  if (param_5 != (uint *)0x0) {
    *param_5 = (uint)piVar5;
  }
  if (param_6 != (uint *)0x0) {
    *param_6 = (uint)local_8;
  }
  if (param_7 != (uint *)0x0) {
    *param_7 = uVar9;
  }
  if (param_8 != (int *)0x0) {
    *param_8 = local_c;
  }
  return 0;
}

