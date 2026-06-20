
undefined4 __thiscall MeshData_SplitVerticesByAttribute(void *this,int *param_1,uint *param_2)

{
  int *piVar1;
  void *pvVar2;
  uint uVar3;
  int iVar4;
  int *piVar5;
  undefined4 *puVar6;
  int local_4c [4];
  undefined4 *local_3c;
  int *local_38;
  int local_34 [8];
  void *local_14;
  undefined4 *local_10;
  undefined4 local_c;
  void *local_8;
  
  local_4c[3] = *(int *)((int)this + 0x30);
  *param_1 = 0;
  *param_2 = 0;
  local_c = 0;
  local_8 = (void *)0x0;
  local_14 = (void *)0x0;
  local_4c[2] = 0;
  local_34[4] = 0;
  local_4c[1] = 0;
  local_34[2] = 0;
  local_34[1] = 0;
  local_10 = operator_new(*(int *)((int)this + 0x30) << 2);
  local_3c = operator_new(*(int *)((int)this + 0x30) << 2);
  if ((local_10 == (undefined4 *)0x0) || (local_3c == (undefined4 *)0x0)) {
LAB_00481fdc:
    local_c = 0x8007000e;
  }
  else {
    puVar6 = local_3c;
    for (uVar3 = *(uint *)((int)this + 0x30) & 0x3fffffff; uVar3 != 0; uVar3 = uVar3 - 1) {
      *puVar6 = 0xffffffff;
      puVar6 = puVar6 + 1;
    }
    for (iVar4 = 0; iVar4 != 0; iVar4 = iVar4 + -1) {
      *(undefined1 *)puVar6 = 0xff;
      puVar6 = (undefined4 *)((int)puVar6 + 1);
    }
    puVar6 = local_10;
    for (uVar3 = *(uint *)((int)this + 0x30) & 0x3fffffff; uVar3 != 0; uVar3 = uVar3 - 1) {
      *puVar6 = 0xffffffff;
      puVar6 = puVar6 + 1;
    }
    for (iVar4 = 0; iVar4 != 0; iVar4 = iVar4 + -1) {
      *(undefined1 *)puVar6 = 0xff;
      puVar6 = (undefined4 *)((int)puVar6 + 1);
    }
    local_34[5] = 0;
    if (*(int *)((int)this + 0x58) != 0) {
      local_34[3] = 0;
      do {
        piVar5 = (int *)(*(int *)((int)this + 0x40) + local_34[3]);
        local_38 = piVar5;
        if (*piVar5 != -1) {
          local_34[7] = *(int *)(*(int *)((int)this + 0x48) + local_34[5] * 4);
          local_34[6] = 0;
          do {
            piVar1 = piVar5 + local_34[6];
            iVar4 = local_3c[*piVar1];
            if (iVar4 != local_34[7]) {
              if (iVar4 == -1) {
                local_3c[*piVar1] = local_34[7];
              }
              else {
                for (iVar4 = local_10[*piVar1]; iVar4 != -1;
                    iVar4 = *(int *)(iVar4 * 4 + (int)local_8)) {
                  if (*(int *)(iVar4 * 4 + (int)local_14) == local_34[7]) {
                    iVar4 = iVar4 + local_4c[3];
                    goto LAB_00481fac;
                  }
                }
                if (local_34[4] == 0) {
                  local_34[2] = 0x100;
                  local_34[4] = 0x100;
                  local_34[1] = 0x100;
                  local_8 = operator_new(0x400);
                  local_14 = operator_new(0x400);
                  pvVar2 = operator_new(0x400);
                  *param_1 = (int)pvVar2;
                  if (((local_8 == (void *)0x0) || (local_14 == (void *)0x0)) ||
                     (pvVar2 == (void *)0x0)) goto LAB_00481fdc;
                }
                local_34[0] = *(int *)((int)this + 0x30);
                *(int *)((int)this + 0x30) = local_34[0] + 1;
                uVar3 = *param_2;
                iVar4 = DynArray_Grow((int *)&local_14,local_34 + 7,(uint *)(local_4c + 2),
                                      (uint *)(local_34 + 4));
                if ((iVar4 == 0) ||
                   (iVar4 = DynArray_Grow((int *)&local_8,local_34,(uint *)(local_4c + 1),
                                          (uint *)(local_34 + 2)), iVar4 == 0)) goto LAB_00481fdc;
                local_4c[0] = *piVar1;
                iVar4 = DynArray_Grow(param_1,local_4c,param_2,(uint *)(local_34 + 1));
                if (iVar4 == 0) goto LAB_00481fdc;
                *(undefined4 *)((int)local_8 + uVar3 * 4) = local_10[*piVar1];
                local_10[*piVar1] = uVar3;
                iVar4 = local_34[0];
LAB_00481fac:
                *piVar1 = iVar4;
                piVar5 = local_38;
              }
            }
            local_34[6] = local_34[6] + 1;
          } while ((uint)local_34[6] < 3);
        }
        local_34[5] = local_34[5] + 1;
        local_34[3] = local_34[3] + 0xc;
      } while ((uint)local_34[5] < *(uint *)((int)this + 0x58));
    }
  }
  _free(local_10);
  _free(local_8);
  _free(local_3c);
  _free(local_14);
  return local_c;
}

