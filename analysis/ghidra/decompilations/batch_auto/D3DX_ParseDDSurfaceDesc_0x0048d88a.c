
undefined4 __thiscall D3DX_ParseDDSurfaceDesc(void *this,uint *param_1,uint param_2)

{
  undefined1 *puVar1;
  undefined1 *puVar2;
  uint uVar3;
  byte bVar4;
  ushort uVar5;
  uint uVar6;
  uint *puVar7;
  byte bVar8;
  uint uVar9;
  uint uVar10;
  uint uVar11;
  undefined4 *puVar12;
  uint *puVar13;
  uint uVar14;
  int iVar15;
  uint uVar16;
  uint *puVar17;
  uint *puVar18;
  bool bVar19;
  bool bVar20;
  uint local_20;
  uint local_1c;
  uint local_18;
  int local_10;
  uint *local_8;
  
  puVar7 = param_1;
  if (param_2 < 0x12) {
    return 0x80004005;
  }
  if (((((*(byte *)((int)param_1 + 1) & 0xfe) != 0) ||
       (bVar4 = *(byte *)((int)param_1 + 2), (bVar4 & 0xf4) != 0)) || ((short)param_1[3] == 0)) ||
     (*(short *)((int)param_1 + 0xe) == 0)) {
    return 0x80004005;
  }
  uVar9 = (uint)*(byte *)((int)param_1 + 7);
  local_18 = 0;
  if (*(byte *)((int)param_1 + 1) != 0) {
    if (uVar9 == 0xf) {
      local_18 = 0x18;
    }
    else if (uVar9 == 0x10) {
      local_18 = 0x19;
    }
    else if (uVar9 == 0x18) {
      local_18 = 0x14;
    }
    else {
      if (uVar9 != 0x20) {
        return 0x80004005;
      }
      local_18 = 0x15;
    }
  }
  uVar14 = (uint)(byte)param_1[4];
  uVar10 = uVar14 + 7 >> 3;
  bVar8 = bVar4 & 3;
  if (bVar8 == 1) {
    if (*(char *)((int)param_1 + 1) == '\0') {
      return 0x80004005;
    }
    if ((char)param_1[4] != '\b') {
      return 0x80004005;
    }
    local_10 = 0x29;
  }
  else if (bVar8 == 2) {
    if (uVar14 == 0xf) {
      local_10 = 0x18;
    }
    else if (uVar14 == 0x10) {
      local_10 = 0x19;
    }
    else if (uVar14 == 0x18) {
      local_10 = 0x14;
    }
    else {
      if (uVar14 != 0x20) {
        return 0x80004005;
      }
      local_10 = 0x15;
    }
  }
  else {
    if (bVar8 != 3) {
      return 0x80004005;
    }
    if ((char)param_1[4] != '\b') {
      return 0x80004005;
    }
    local_10 = 0x32;
  }
  bVar19 = (*(byte *)((int)param_1 + 0x11) & 0x20) != 0x20;
  bVar20 = (*(byte *)((int)param_1 + 0x11) & 0x10) == 0x10;
  uVar11 = (uint)(byte)*param_1;
  if (param_2 - 0x12 < uVar11) {
    return 0x80004005;
  }
  uVar3 = (param_2 - 0x12) - uVar11;
  local_8 = (uint *)(uVar11 + 0x12 + (int)param_1);
  uVar9 = (uint)*(ushort *)((int)param_1 + 5) * (uVar9 + 7 >> 3);
  if (uVar3 < uVar9) {
    return 0x80004005;
  }
  if ((*(int *)((int)this + 0x40) != 0) && (local_10 == 0x29)) {
    if (0x100 < (uint)*(ushort *)((int)param_1 + 3) + (uint)*(ushort *)((int)param_1 + 5)) {
      return 0x80004005;
    }
    puVar12 = operator_new(0x400);
    *(undefined4 **)((int)this + 8) = puVar12;
    if (puVar12 == (undefined4 *)0x0) {
      return 0x8007000e;
    }
    *(undefined4 *)((int)this + 0x3c) = 1;
    for (iVar15 = 0x100; iVar15 != 0; iVar15 = iVar15 + -1) {
      *puVar12 = 0xffffffff;
      puVar12 = puVar12 + 1;
    }
    puVar1 = (undefined1 *)(*(int *)((int)this + 8) + (uint)*(ushort *)((int)param_1 + 3) * 4);
    puVar2 = puVar1 + (uint)*(ushort *)((int)param_1 + 5) * 4;
    uVar11 = uVar3;
    uVar16 = uVar3;
    param_1 = local_8;
    for (; puVar1 < puVar2; puVar1 = puVar1 + 4) {
      if (local_18 == 0x14) {
        uVar11 = (uint)*(byte *)((int)param_1 + 2);
        uVar16 = (uint)*(byte *)((int)param_1 + 1);
        local_1c = (uint)(byte)*param_1;
        param_1 = (uint *)((int)param_1 + 3);
LAB_0048db67:
        local_20 = 0xff;
      }
      else if (local_18 == 0x15) {
        uVar6 = *param_1;
        local_20 = uVar6 >> 0x18;
        uVar16 = uVar6 >> 8 & 0xff;
        uVar11 = uVar6 >> 0x10 & 0xff;
        local_1c = uVar6 & 0xff;
        param_1 = param_1 + 1;
      }
      else {
        if (local_18 == 0x18) {
          uVar5 = (ushort)*param_1;
          uVar11 = uVar5 >> 10 & 0x1f;
          uVar16 = uVar5 >> 5 & 0x1f;
          uVar11 = uVar11 >> 2 | uVar11 << 3;
          uVar16 = uVar16 >> 2 | uVar16 << 3;
          local_1c = (uVar5 & 0x1f) >> 2 | (uVar5 & 0x1f) << 3;
          param_1 = (uint *)((int)param_1 + 2);
          goto LAB_0048db67;
        }
        if (local_18 == 0x19) {
          uVar5 = (ushort)*param_1;
          local_20 = (uint)(uVar5 >> 0xf) * 0xff;
          uVar11 = uVar5 >> 10 & 0x1f;
          uVar16 = uVar5 >> 5 & 0x1f;
          uVar11 = uVar11 >> 2 | uVar11 << 3;
          uVar16 = uVar16 >> 2 | uVar16 << 3;
          local_1c = (uVar5 & 0x1f) >> 2 | (uVar5 & 0x1f) << 3;
          param_1 = (uint *)((int)param_1 + 2);
        }
      }
      puVar1[2] = (char)local_1c;
      puVar1[3] = (undefined1)local_20;
      *puVar1 = (char)uVar11;
      puVar1[1] = (char)uVar16;
    }
  }
  param_2 = uVar3 - uVar9;
  local_8 = (uint *)((int)local_8 + uVar9);
  uVar9 = (uint)*(ushort *)((int)puVar7 + 0xe) * (uint)(ushort)puVar7[3] * uVar10;
  *(int *)this = local_10;
  *(uint **)((int)this + 4) = local_8;
  *(uint *)((int)this + 0x30) = (ushort)puVar7[3] * uVar10;
  *(undefined4 *)((int)this + 0x34) = 0;
  *(uint *)((int)this + 0xc) = (uint)(ushort)puVar7[3];
  *(uint *)((int)this + 0x10) = (uint)*(ushort *)((int)puVar7 + 0xe);
  *(undefined4 *)((int)this + 0x14) = 1;
  if (*(int *)((int)this + 0x40) != 0) {
    if ((((bVar4 & 8) != 0) || (bVar19)) || (bVar20)) {
      puVar13 = operator_new(uVar9);
      *(uint **)((int)this + 4) = puVar13;
      if (puVar13 == (uint *)0x0) {
        return 0x8007000e;
      }
      *(undefined4 *)((int)this + 0x38) = 1;
      if (bVar19) {
        puVar13 = (uint *)((*(ushort *)((int)puVar7 + 0xe) - 1) * *(int *)((int)this + 0x30) +
                          (int)puVar13);
      }
      local_18 = 0;
      if (*(short *)((int)puVar7 + 0xe) != 0) {
        do {
          param_1 = puVar13;
          if (bVar20) {
            param_1 = (uint *)((*(int *)((int)this + 0x30) - uVar10) + (int)puVar13);
          }
          uVar9 = (uint)(ushort)puVar7[3];
          local_1c = 0;
          if (uVar9 != 0) {
            do {
              if ((bVar4 & 8) == 0) {
                local_20 = 0;
              }
              else {
                if (param_2 == 0) {
                  return 0x80004005;
                }
                local_20 = (byte)*local_8 & 0x80;
                uVar9 = ((byte)*local_8 & 0x7f) + 1;
                local_8 = (uint *)((int)local_8 + 1);
                param_2 = param_2 - 1;
              }
              local_1c = local_1c + uVar9;
              while (uVar9 != 0) {
                uVar9 = uVar9 - 1;
                if (param_2 < uVar10) {
                  return 0x80004005;
                }
                puVar17 = local_8;
                puVar18 = param_1;
                for (uVar11 = uVar14 + 7 >> 5; uVar11 != 0; uVar11 = uVar11 - 1) {
                  *puVar18 = *puVar17;
                  puVar17 = puVar17 + 1;
                  puVar18 = puVar18 + 1;
                }
                for (uVar11 = uVar10 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
                  *(byte *)puVar18 = (byte)*puVar17;
                  puVar17 = (uint *)((int)puVar17 + 1);
                  puVar18 = (uint *)((int)puVar18 + 1);
                }
                if (local_20 == 0) {
                  local_8 = (uint *)((int)local_8 + uVar10);
                  param_2 = param_2 - uVar10;
                }
                uVar11 = uVar10;
                if (bVar20) {
                  uVar11 = -uVar10;
                }
                param_1 = (uint *)((int)param_1 + uVar11);
              }
              if (local_20 != 0) {
                local_8 = (uint *)((int)local_8 + uVar10);
                param_2 = param_2 - uVar10;
              }
              uVar9 = (uint)(ushort)puVar7[3];
            } while (local_1c < uVar9);
          }
          if (bVar19) {
            iVar15 = -*(int *)((int)this + 0x30);
          }
          else {
            iVar15 = *(int *)((int)this + 0x30);
          }
          puVar13 = (uint *)((int)puVar13 + iVar15);
          local_18 = local_18 + 1;
        } while (local_18 < *(ushort *)((int)puVar7 + 0xe));
      }
    }
    else {
      if (param_2 < uVar9) {
        return 0x80004005;
      }
      *(uint **)((int)this + 4) = local_8;
      *(undefined4 *)((int)this + 0x38) = 0;
    }
  }
  return 0;
}

