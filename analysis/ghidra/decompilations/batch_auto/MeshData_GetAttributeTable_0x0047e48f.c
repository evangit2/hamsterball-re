
undefined4 __thiscall MeshData_GetAttributeTable(void *this,undefined4 *param_1,uint *param_2)

{
  int iVar1;
  undefined4 *puVar2;
  uint uVar3;
  int iVar4;
  undefined4 *puVar5;
  undefined4 *puVar6;
  uint local_24;
  int local_20;
  undefined4 local_1c;
  int local_18;
  uint local_14;
  undefined4 *local_10;
  uint local_c;
  uint local_8;
  
  local_1c = 0;
  local_c = 0;
  local_10 = (undefined4 *)0x0;
  if (*(int *)((int)this + 0x58) == 0) {
    *param_2 = 0;
    *param_1 = 0;
  }
  else {
    local_24 = 10;
    puVar2 = operator_new(0x28);
    local_10 = puVar2;
    if (puVar2 == (undefined4 *)0x0) {
LAB_0047e4d4:
      local_1c = 0x8007000e;
    }
    else {
      local_20 = 0;
      local_14 = 0;
      if (*(int *)((int)this + 0x58) != 0) {
        local_8 = 0;
        local_18 = 0;
        puVar5 = puVar2;
        do {
          puVar2 = puVar5;
          if ((*(int *)(local_18 + *(int *)((int)this + 0x40)) != -1) &&
             ((iVar1 = *(int *)(*(int *)((int)this + 0x48) + local_14 * 4), iVar1 != local_20 ||
              (local_8 == 0)))) {
            uVar3 = 0;
            if (local_8 != 0) {
              do {
                if (puVar5[uVar3] == iVar1) break;
                uVar3 = uVar3 + 1;
              } while (uVar3 < local_c);
            }
            local_20 = iVar1;
            if (uVar3 == local_c) {
              if (local_c == local_24) {
                puVar2 = operator_new(local_24 << 3);
                if (puVar2 == (undefined4 *)0x0) goto LAB_0047e4d4;
                puVar6 = puVar2;
                for (uVar3 = local_8 >> 2; uVar3 != 0; uVar3 = uVar3 - 1) {
                  *puVar6 = *puVar5;
                  puVar5 = puVar5 + 1;
                  puVar6 = puVar6 + 1;
                }
                for (iVar4 = 0; iVar4 != 0; iVar4 = iVar4 + -1) {
                  *(undefined1 *)puVar6 = *(undefined1 *)puVar5;
                  puVar5 = (undefined4 *)((int)puVar5 + 1);
                  puVar6 = (undefined4 *)((int)puVar6 + 1);
                }
                _free(local_10);
                local_24 = local_24 * 2;
                local_10 = puVar2;
              }
              local_c = local_c + 1;
              *(int *)(local_8 + (int)puVar2) = iVar1;
              local_8 = local_8 + 4;
            }
          }
          local_14 = local_14 + 1;
          local_18 = local_18 + 0xc;
          puVar5 = puVar2;
        } while (local_14 < *(uint *)((int)this + 0x58));
      }
      *param_1 = puVar2;
      *param_2 = local_c;
      local_10 = (undefined4 *)0x0;
    }
  }
  _free(local_10);
  return local_1c;
}

