
undefined4 D3DX_ParseDeclarationType(uint *param_1,uint *param_2)

{
  uint uVar1;
  uint *puVar2;
  undefined4 uVar3;
  uint uVar4;
  int iVar5;
  uint uVar6;
  int iVar7;
  int iStack_18;
  uint local_8;
  
  iVar7 = 0;
  local_8 = 0;
  if (*param_1 != 0x20000000) goto LAB_0047dae6;
  puVar2 = param_1 + 1;
  if (*puVar2 == 0x40020000) {
    puVar2 = param_1 + 2;
    uVar4 = *puVar2;
    iVar5 = 0;
    if ((((uVar4 == 0x40000001) || (uVar4 == 0x40010001)) || (uVar4 == 0x40020001)) ||
       (uVar4 == 0x40030001)) {
      iVar5 = (uVar4 >> 0x10 & 0xf) + 1;
      puVar2 = param_1 + 3;
    }
    if (*puVar2 == 0x40050002) {
      iVar5 = iVar5 + 1;
      local_8 = 0x1000;
      puVar2 = puVar2 + 1;
    }
    if (iVar5 == 0) {
      local_8 = local_8 | 2;
    }
    else if (iVar5 == 1) {
      local_8 = local_8 | 6;
    }
    else if (iVar5 == 2) {
      local_8 = local_8 | 8;
    }
    else if (iVar5 == 3) {
      local_8 = local_8 | 10;
    }
    else if (iVar5 == 4) {
      local_8 = local_8 | 0xc;
    }
    else if (iVar5 == 5) {
      local_8 = local_8 | 0xe;
    }
  }
  else if (*puVar2 == 0x40030000) {
    local_8 = 4;
    puVar2 = param_1 + 2;
  }
  uVar4 = 0x10;
  param_1 = puVar2;
  if (*puVar2 == 0x40020003) {
    local_8 = local_8 | 0x10;
    param_1 = puVar2 + 1;
  }
  if (*param_1 == 0x40000004) {
    local_8 = local_8 | 0x20;
    param_1 = param_1 + 1;
  }
  if (*param_1 == 0x40040005) {
    local_8 = local_8 | 0x40;
    param_1 = param_1 + 1;
  }
  if (*param_1 == 0x40040006) {
    local_8 = local_8 | 0x80;
    param_1 = param_1 + 1;
  }
  uVar6 = 7;
LAB_0047da8f:
  uVar1 = *param_1;
  if ((uVar6 | 0x40000000) == uVar1) {
    iStack_18 = 3;
  }
  else {
    if ((uVar6 | 0x40010000) == uVar1) goto LAB_0047dad2;
    if ((uVar6 | 0x40020000) != uVar1) {
      if ((uVar6 | 0x40030000) == uVar1) {
        iStack_18 = 2;
        goto LAB_0047dacd;
      }
      goto LAB_0047dade;
    }
    iStack_18 = 1;
  }
LAB_0047dacd:
  local_8 = local_8 | iStack_18 << ((byte)uVar4 & 0x1f);
LAB_0047dad2:
  param_1 = param_1 + 1;
  iVar7 = iVar7 + 1;
  uVar4 = uVar4 + 2;
  uVar6 = uVar6 + 1;
  if (0x1f < uVar4) goto LAB_0047dade;
  goto LAB_0047da8f;
LAB_0047dade:
  local_8 = local_8 | iVar7 << 8;
LAB_0047dae6:
  if (*param_1 == 0xffffffff) {
    *param_2 = local_8;
    uVar3 = 0;
  }
  else {
    uVar3 = 0x8876086c;
  }
  return uVar3;
}

