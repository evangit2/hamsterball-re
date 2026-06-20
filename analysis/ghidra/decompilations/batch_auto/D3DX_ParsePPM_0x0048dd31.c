
uint __thiscall D3DX_ParsePPM(void *this,uint *param_1,uint param_2)

{
  byte bVar1;
  bool bVar2;
  byte *pbVar3;
  uint uVar4;
  uint uVar5;
  int iVar6;
  uint uVar7;
  byte *pbVar8;
  uint local_18;
  uint *local_14;
  uint local_c;
  int local_8;
  
  if (param_2 < 2) {
    return 0x80004005;
  }
  if ((char)*param_1 == 'P') {
    if (*(char *)((int)param_1 + 1) == '3') {
      bVar2 = true;
    }
    else {
      if (*(char *)((int)param_1 + 1) != '6') {
        return 0x80004005;
      }
      bVar2 = false;
    }
    pbVar8 = (byte *)((int)param_1 + 2);
    param_2 = param_2 - 2;
    local_8 = 0;
    local_18 = 0;
    local_c = 0xff;
    pbVar3 = pbVar8;
    while (param_2 != 0) {
      uVar7 = 0;
      if ((!bVar2) && (local_8 == 3)) {
        if (0xff < local_c) {
          return 0x80004005;
        }
        if ((1 < param_2) && (*pbVar8 == 0xd)) {
          pbVar8 = pbVar8 + 1;
          param_2 = param_2 - 1;
        }
        iVar6 = param_2 - 1;
        pbVar8 = pbVar8 + 1;
        for (; (iVar6 != 0 && (param_1 < local_14)); param_1 = param_1 + 1) {
          *param_1 = ((((uint)*pbVar8 * 0xff) / local_c | 0xffffff00) << 8 |
                     ((uint)pbVar8[1] * 0xff) / local_c) << 8 | ((uint)pbVar8[2] * 0xff) / local_c;
          iVar6 = iVar6 + -3;
          pbVar8 = pbVar8 + 3;
        }
        return -(uint)(param_1 != local_14) & 0x80004005;
      }
      iVar6 = _isspace((uint)*pbVar8);
      if (iVar6 == 0) {
        if (*pbVar8 == 0x23) {
          for (; (param_2 != 0 && (*pbVar8 != 10)); pbVar8 = pbVar8 + 1) {
            param_2 = param_2 - 1;
          }
          goto LAB_0048ddbb;
        }
        while ((param_2 != 0 && (iVar6 = _isspace((uint)*pbVar8), iVar6 == 0))) {
          iVar6 = _isdigit((uint)*pbVar8);
          if (iVar6 == 0) {
            return 0x80004005;
          }
          bVar1 = *pbVar8;
          pbVar8 = pbVar8 + 1;
          param_2 = param_2 - 1;
          uVar7 = (bVar1 - 0x30) + uVar7 * 10;
          pbVar3 = pbVar8;
        }
        uVar4 = uVar7;
        uVar5 = local_c;
        if (local_8 == 0) {
LAB_0048df00:
          local_c = uVar5;
          local_18 = uVar4;
          if (uVar7 == 0) {
            return 0x80004005;
          }
        }
        else if (local_8 == 1) {
          if (uVar7 == 0) {
            return 0x80004005;
          }
          param_1 = operator_new(uVar7 * local_18 * 4);
          *(uint **)((int)this + 4) = param_1;
          if (param_1 == (uint *)0x0) {
            return 0x8007000e;
          }
          local_14 = param_1 + uVar7 * local_18;
          *(undefined4 *)((int)this + 0x34) = 0;
          *(undefined4 *)((int)this + 0x38) = 1;
          *(undefined4 *)this = 0x16;
          *(uint *)((int)this + 0x30) = local_18 << 2;
          *(uint *)((int)this + 0xc) = local_18;
          *(uint *)((int)this + 0x10) = uVar7;
          *(undefined4 *)((int)this + 0x14) = 1;
          pbVar8 = pbVar3;
        }
        else {
          uVar4 = local_18;
          uVar5 = uVar7;
          if (local_8 == 2) goto LAB_0048df00;
          if (local_8 == 3) {
            if (local_14 <= param_1) {
              return 0x80004005;
            }
            *param_1 = ((uVar7 * 0xff) / local_c | 0xffffff00) << 0x10;
          }
          else if (local_8 == 4) {
            *param_1 = *param_1 | (uVar7 * 0xff) / local_c << 8;
          }
          else if (local_8 == 5) {
            *param_1 = *param_1 | (uVar7 * 0xff) / local_c;
            param_1 = param_1 + 1;
            if (param_1 == local_14) {
              return 0;
            }
            local_8 = 2;
          }
        }
        local_8 = local_8 + 1;
      }
      else {
LAB_0048ddbb:
        pbVar8 = pbVar8 + 1;
        param_2 = param_2 - 1;
        pbVar3 = pbVar8;
      }
    }
  }
  return 0x80004005;
}

