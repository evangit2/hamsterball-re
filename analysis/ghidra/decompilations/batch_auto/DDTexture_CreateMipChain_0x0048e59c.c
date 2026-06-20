
undefined4 __thiscall DDTexture_CreateMipChain(void *this,int *param_1,int *param_2)

{
  int iVar1;
  undefined4 *puVar2;
  int *piVar3;
  int *piVar4;
  int iVar5;
  uint uVar6;
  int iVar7;
  int *piVar8;
  uint uVar9;
  int *piVar10;
  uint uVar11;
  uint uVar12;
  int *local_2c;
  uint local_24;
  int *local_1c;
  uint local_18;
  uint local_14;
  uint local_10;
  int *local_8;
  
  if (((param_2 < (int *)0x4) || (*param_1 != 0x20534444)) || (param_2 + -1 < (int *)0x7c)) {
    return 0x80004005;
  }
  local_8 = param_1 + 0x20;
  *(int *)((int)this + 0xc) = param_1[4];
  *(int *)((int)this + 0x10) = param_1[3];
  if ((*(byte *)((int)param_1 + 10) & 0x80) == 0) {
    iVar1 = 1;
  }
  else {
    iVar1 = param_1[6];
  }
  *(int *)((int)this + 0x14) = iVar1;
  if (iVar1 == 0) {
    *(undefined4 *)((int)this + 0x14) = 1;
  }
  if ((param_1[0x1c] & 0xfc00U) == 0xfc00) {
    local_18 = 6;
  }
  else {
    if ((param_1[0x1c] & 0xfc00U) != 0) {
      return 0x80004005;
    }
    local_18 = 1;
  }
  local_24 = param_1[7];
  if (local_24 == 0) {
    local_24 = 1;
  }
  piVar10 = &DAT_004fa170;
  iVar1 = DAT_004fa170;
  while (iVar1 != 0) {
    if ((param_1[0x13] == 0x20) && (uVar11 = piVar10[2], param_1[0x14] == uVar11)) {
      if ((((((uVar11 & 4) == 0) || (param_1[0x15] == piVar10[3])) &&
           (((uVar11 & 0xc4462) == 0 || (param_1[0x16] == piVar10[4])))) &&
          (((uVar11 & 0xe4040) == 0 || (param_1[0x17] == piVar10[5])))) &&
         (((((uVar11 & 0xc4440) == 0 || (param_1[0x18] == piVar10[6])) &&
           (((uVar11 & 0xc4040) == 0 || (param_1[0x19] == piVar10[7])))) &&
          (((uVar11 & 0x80003) == 0 || (param_1[0x1a] == piVar10[8])))))) break;
    }
    piVar10 = piVar10 + 9;
    iVar1 = *piVar10;
  }
  iVar1 = *piVar10;
  if (iVar1 == 0) {
    return 0x80004005;
  }
  piVar10 = D3D_FindDisplayMode(iVar1);
  if ((*(byte *)((int)param_1 + 10) & 0x80) == 0) {
    *(uint *)((int)this + 0x44) = (uint)(local_18 == 6) * 2 + 3;
  }
  else {
    *(undefined4 *)((int)this + 0x44) = 4;
  }
  if (piVar10[1] == 1) {
    if (param_2 + -0x20 < (int *)0x400) {
      return 0x80004005;
    }
    param_2 = param_2 + -0x120;
    local_2c = local_8;
    local_8 = param_1 + 0x120;
  }
  else {
    local_2c = (int *)0x0;
    param_2 = param_2 + -0x20;
  }
  local_14 = 0;
  piVar4 = param_2;
  if (local_18 != 0) {
    do {
      uVar11 = *(uint *)((int)this + 0xc);
      uVar9 = *(uint *)((int)this + 0x10);
      param_1 = *(int **)((int)this + 0x14);
      piVar3 = this;
      if (local_14 != 0) {
        puVar2 = operator_new(0x54);
        if (puVar2 == (undefined4 *)0x0) {
          piVar3 = (int *)0x0;
        }
        else {
          piVar3 = (int *)WebClient_InitResponse(puVar2);
        }
        if (piVar3 == (int *)0x0) {
          return 0x8007000e;
        }
        local_1c[0x14] = (int)piVar3;
      }
      local_10 = 0;
      piVar8 = piVar4;
      if (local_24 != 0) {
        do {
          piVar4 = piVar3;
          if (local_10 != 0) {
            puVar2 = operator_new(0x54);
            if (puVar2 == (undefined4 *)0x0) {
              piVar4 = (int *)0x0;
            }
            else {
              piVar4 = (int *)WebClient_InitResponse(puVar2);
            }
            if (piVar4 == (int *)0x0) {
              return 0x8007000e;
            }
            piVar8[0x13] = (int)piVar4;
          }
          if (iVar1 == 0x31545844) {
            iVar5 = (uVar11 + 3 >> 2) << 3;
LAB_0048e839:
            iVar7 = (uVar9 + 3 >> 2) * iVar5;
          }
          else {
            if (iVar1 == 0x32545844) {
LAB_0048e825:
              iVar5 = (uVar11 + 3 >> 2) << 4;
              goto LAB_0048e839;
            }
            if (iVar1 == 0x32595559) {
LAB_0048e816:
              iVar5 = (uVar11 + 1 >> 1) << 2;
            }
            else {
              if (((iVar1 == 0x33545844) || (iVar1 == 0x34545844)) || (iVar1 == 0x35545844))
              goto LAB_0048e825;
              if (iVar1 == 0x59565955) goto LAB_0048e816;
              iVar5 = ((uint)piVar10[2] >> 3) * uVar11;
            }
            iVar7 = iVar5 * uVar9;
          }
          piVar4[0xe] = 0;
          piVar4[0xf] = 0;
          piVar8 = (int *)(iVar7 * (int)param_1);
          *piVar4 = iVar1;
          piVar4[0xc] = iVar5;
          piVar4[1] = (int)local_8;
          piVar4[5] = (int)param_1;
          piVar4[0xd] = iVar7;
          piVar4[3] = uVar11;
          piVar4[4] = uVar9;
          piVar4[2] = (int)local_2c;
          if (param_2 < piVar8) {
            return 0x80004005;
          }
          local_8 = (int *)((int)local_8 + (int)piVar8);
          param_2 = (int *)((int)param_2 - (int)piVar8);
          uVar6 = 1;
          uVar12 = uVar6;
          if (uVar11 != 1) {
            uVar12 = uVar11 >> 1;
          }
          if (uVar9 != 1) {
            uVar6 = uVar9 >> 1;
          }
          if (param_1 == (int *)0x1) {
            param_1 = (int *)0x1;
          }
          else {
            param_1 = (int *)((uint)param_1 >> 1);
          }
          local_10 = local_10 + 1;
          uVar9 = uVar6;
          piVar8 = piVar4;
          uVar11 = uVar12;
        } while (local_10 < local_24);
      }
      local_14 = local_14 + 1;
      local_1c = piVar3;
    } while (local_14 < local_18);
  }
  return 0;
}

