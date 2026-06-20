
undefined4 __cdecl
Inflate_HuffmanBuild
          (uint *param_1,uint param_2,uint param_3,int param_4,int param_5,int *param_6,int param_7,
          uint *param_8,uint *param_9)

{
  undefined4 uVar1;
  uint *in_EAX;
  int iVar2;
  uint uVar3;
  uint uVar4;
  uint *puVar5;
  int iVar6;
  uint uVar7;
  undefined4 *puVar8;
  uint uVar9;
  uint uVar10;
  int iVar11;
  byte bVar12;
  int iVar13;
  int local_fc [15];
  uint local_c0 [16];
  uint local_80 [17];
  int local_3c;
  int local_38;
  int local_34;
  uint local_30;
  int local_2c;
  uint *local_28;
  uint local_24;
  uint local_20;
  undefined4 local_1c;
  uint local_18;
  uint local_14;
  int local_10;
  uint local_c;
  uint local_8;
  
  local_80[0] = 0;
  local_80[1] = 0;
  local_80[2] = 0;
  local_80[3] = 0;
  local_80[4] = 0;
  local_80[5] = 0;
  local_80[6] = 0;
  local_80[7] = 0;
  local_80[8] = 0;
  local_80[9] = 0;
  local_80[10] = 0;
  local_80[0xb] = 0;
  local_80[0xc] = 0;
  local_80[0xd] = 0;
  local_80[0xe] = 0;
  local_80[0xf] = 0;
  puVar5 = param_1;
  uVar9 = param_2;
  do {
    local_80[*puVar5] = local_80[*puVar5] + 1;
    puVar5 = puVar5 + 1;
    uVar9 = uVar9 - 1;
  } while (uVar9 != 0);
  if (local_80[0] == param_2) {
    *param_6 = 0;
    *in_EAX = 0;
  }
  else {
    uVar10 = 1;
    local_8 = *in_EAX;
    uVar9 = 0xf;
    do {
      if (local_80[uVar10] != 0) break;
      uVar10 = uVar10 + 1;
    } while (uVar10 < 0x10);
    local_c = uVar10;
    if (*in_EAX < uVar10) {
      local_8 = uVar10;
    }
    do {
      if (local_80[uVar9] != 0) break;
      uVar9 = uVar9 - 1;
    } while (uVar9 != 0);
    local_30 = uVar9;
    if (uVar9 < local_8) {
      local_8 = uVar9;
    }
    *in_EAX = local_8;
    iVar11 = 1 << ((byte)uVar10 & 0x1f);
    for (; uVar10 < uVar9; uVar10 = uVar10 + 1) {
      if ((int)(iVar11 - local_80[uVar10]) < 0) {
        return 0xfffffffd;
      }
      iVar11 = (iVar11 - local_80[uVar10]) * 2;
    }
    uVar10 = local_80[uVar9];
    iVar11 = iVar11 - uVar10;
    local_38 = iVar11;
    if (iVar11 < 0) {
      return 0xfffffffd;
    }
    local_80[uVar9] = uVar10 + iVar11;
    iVar6 = 0;
    iVar2 = uVar9 - 1;
    local_c0[1] = 0;
    if (iVar2 != 0) {
      iVar13 = 0;
      do {
        iVar6 = iVar6 + *(int *)((int)local_80 + iVar13 + 4);
        iVar2 = iVar2 + -1;
        *(int *)((int)local_c0 + iVar13 + 8) = iVar6;
        iVar13 = iVar13 + 4;
      } while (iVar2 != 0);
    }
    uVar10 = 0;
    do {
      iVar2 = local_38;
      uVar7 = *param_1;
      param_1 = param_1 + 1;
      if (uVar7 != 0) {
        uVar3 = local_c0[uVar7];
        param_9[uVar3] = uVar10;
        local_c0[uVar7] = uVar3 + 1;
        iVar11 = iVar2;
      }
      uVar10 = uVar10 + 1;
    } while (uVar10 < param_2);
    uVar9 = local_c0[uVar9];
    local_10 = -1;
    param_1 = param_9;
    iVar2 = -local_8;
    local_24 = 0;
    local_c0[0] = 0;
    local_fc[0] = 0;
    local_2c = 0;
    local_14 = 0;
    if ((int)local_c <= (int)local_30) {
      local_34 = local_c - 1;
      local_28 = local_80 + local_c;
      do {
        local_20 = *local_28;
        uVar1 = local_1c;
        while (local_1c = uVar1, local_20 != 0) {
          local_1c._2_2_ = (undefined2)((uint)uVar1 >> 0x10);
          local_20 = local_20 - 1;
          local_3c = local_8 + iVar2;
          if (local_3c < (int)local_c) {
            local_80[0x10] = iVar2 - local_8;
            do {
              iVar11 = local_10;
              local_10 = local_10 + 1;
              iVar2 = iVar2 + local_8;
              local_80[0x10] = local_80[0x10] + local_8;
              local_3c = local_3c + local_8;
              uVar10 = local_30 - iVar2;
              if (local_8 < local_30 - iVar2) {
                uVar10 = local_8;
              }
              uVar7 = local_c - iVar2;
              uVar3 = 1 << ((byte)uVar7 & 0x1f);
              if ((local_20 + 1 < uVar3) &&
                 (iVar6 = uVar3 + (-1 - local_20), puVar5 = local_28, uVar7 < uVar10)) {
                while (uVar7 = uVar7 + 1, uVar7 < uVar10) {
                  uVar3 = puVar5[1];
                  uVar4 = iVar6 * 2;
                  if (uVar4 < uVar3 || uVar4 - uVar3 == 0) break;
                  iVar6 = uVar4 - uVar3;
                  puVar5 = puVar5 + 1;
                }
              }
              local_14 = 1 << ((byte)uVar7 & 0x1f);
              uVar10 = *param_8 + local_14;
              if (0x5a0 < uVar10) {
                return 0xfffffffc;
              }
              local_2c = param_7 + *param_8 * 8;
              local_fc[local_10] = local_2c;
              uVar3 = local_24;
              *param_8 = uVar10;
              if (local_10 == 0) {
                *param_6 = local_2c;
              }
              else {
                local_c0[local_10] = local_24;
                local_1c = CONCAT31(CONCAT21(local_1c._2_2_,(undefined1)local_8),(byte)uVar7);
                uVar3 = uVar3 >> ((byte)local_80[0x10] & 0x1f);
                iVar11 = local_fc[iVar11];
                local_18 = (local_2c - iVar11 >> 3) - uVar3;
                *(undefined4 *)(iVar11 + uVar3 * 8) = local_1c;
                *(uint *)(iVar11 + 4 + uVar3 * 8) = local_18;
              }
              iVar11 = local_38;
            } while (local_3c < (int)local_c);
          }
          bVar12 = (byte)iVar2;
          if (param_1 < param_9 + uVar9) {
            local_18 = *param_1;
            if (local_18 < param_3) {
              local_1c._0_1_ = (local_18 < 0x100) - 1U & 0x60;
            }
            else {
              iVar6 = (local_18 - param_3) * 4;
              local_1c._0_1_ = *(char *)(iVar6 + param_5) + 0x50;
              local_18 = *(uint *)(iVar6 + param_4);
            }
            param_1 = param_1 + 1;
          }
          else {
            local_1c._0_1_ = 0xc0;
          }
          local_1c = CONCAT31(CONCAT21(local_1c._2_2_,(char)local_c - bVar12),(byte)local_1c);
          iVar6 = 1 << ((char)local_c - bVar12 & 0x1f);
          uVar10 = local_24 >> (bVar12 & 0x1f);
          if (uVar10 < local_14) {
            puVar8 = (undefined4 *)(local_2c + uVar10 * 8);
            do {
              *puVar8 = local_1c;
              puVar8[1] = local_18;
              uVar10 = uVar10 + iVar6;
              puVar8 = puVar8 + iVar6 * 2;
              iVar11 = local_38;
            } while (uVar10 < local_14);
          }
          uVar10 = 1 << ((byte)local_34 & 0x1f);
          while ((local_24 & uVar10) != 0) {
            local_24 = local_24 ^ uVar10;
            uVar10 = uVar10 >> 1;
          }
          local_24 = local_24 ^ uVar10;
          uVar1 = local_1c;
          if (((1 << (bVar12 & 0x1f)) - 1U & local_24) != local_c0[local_10]) {
            do {
              iVar2 = iVar2 - local_8;
              local_10 = local_10 + -1;
            } while (((1 << ((byte)iVar2 & 0x1f)) - 1U & local_24) != local_c0[local_10]);
          }
        }
        local_c = local_c + 1;
        local_28 = local_28 + 1;
        local_34 = local_34 + 1;
      } while ((int)local_c <= (int)local_30);
    }
    if ((iVar11 != 0) && (local_30 != 1)) {
      return 0xfffffffb;
    }
  }
  return 0;
}

