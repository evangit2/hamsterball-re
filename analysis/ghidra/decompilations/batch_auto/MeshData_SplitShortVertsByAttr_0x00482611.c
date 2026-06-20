
undefined4 __thiscall MeshData_SplitShortVertsByAttr(void *this,int *param_1,uint *param_2)

{
  ushort *puVar1;
  ushort uVar2;
  void *pvVar3;
  uint uVar4;
  int iVar5;
  uint uVar6;
  short *psVar7;
  undefined4 *puVar8;
  uint local_50 [4];
  undefined4 *local_40;
  short *local_3c;
  ushort local_38 [2];
  ushort local_34;
  uint local_30 [7];
  void *local_14;
  undefined4 *local_10;
  undefined4 local_c;
  void *local_8;
  
  local_50[3] = *(undefined4 *)((int)this + 0x30);
  *param_1 = 0;
  *param_2 = 0;
  local_c = 0;
  local_8 = (void *)0x0;
  local_14 = (void *)0x0;
  local_50[2] = 0;
  local_30[3] = 0;
  local_50[1] = 0;
  local_30[1] = 0;
  local_30[0] = 0;
  local_10 = operator_new(*(int *)((int)this + 0x30) << 1);
  local_40 = operator_new(*(int *)((int)this + 0x30) << 2);
  if ((local_10 == (undefined4 *)0x0) || (local_40 == (undefined4 *)0x0)) {
LAB_0048284d:
    local_c = 0x8007000e;
  }
  else {
    puVar8 = local_40;
    for (uVar4 = *(uint *)((int)this + 0x30) & 0x3fffffff; uVar4 != 0; uVar4 = uVar4 - 1) {
      *puVar8 = 0xffffffff;
      puVar8 = puVar8 + 1;
    }
    for (iVar5 = 0; iVar5 != 0; iVar5 = iVar5 + -1) {
      *(undefined1 *)puVar8 = 0xff;
      puVar8 = (undefined4 *)((int)puVar8 + 1);
    }
    uVar6 = *(int *)((int)this + 0x30) << 1;
    puVar8 = local_10;
    for (uVar4 = uVar6 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
      *puVar8 = 0xffffffff;
      puVar8 = puVar8 + 1;
    }
    for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
      *(undefined1 *)puVar8 = 0xff;
      puVar8 = (undefined4 *)((int)puVar8 + 1);
    }
    local_30[4] = 0;
    if (*(int *)((int)this + 0x58) != 0) {
      local_30[2] = 0;
      do {
        psVar7 = (short *)(*(int *)((int)this + 0x40) + local_30[2]);
        local_3c = psVar7;
        if (*psVar7 != -1) {
          local_30[6] = *(int *)(*(int *)((int)this + 0x48) + local_30[4] * 4);
          local_30[5] = 0;
          do {
            puVar1 = (ushort *)(psVar7 + local_30[5]);
            iVar5 = local_40[*puVar1];
            if (iVar5 != local_30[6]) {
              if (iVar5 == -1) {
                local_40[*puVar1] = local_30[6];
              }
              else {
                for (uVar2 = *(ushort *)((int)local_10 + (uint)*puVar1 * 2); uVar2 != 0xffff;
                    uVar2 = *(ushort *)((int)local_8 + (uint)uVar2 * 2)) {
                  if (*(int *)((int)local_14 + (uint)uVar2 * 4) == local_30[6]) {
                    *puVar1 = (short)local_50[3] + uVar2;
                    psVar7 = local_3c;
                    goto LAB_0048281f;
                  }
                }
                if (local_30[3] == 0) {
                  local_30[1] = 0x100;
                  local_30[3] = 0x100;
                  local_30[0] = 0x100;
                  local_8 = operator_new(0x200);
                  local_14 = operator_new(0x400);
                  pvVar3 = operator_new(0x400);
                  *param_1 = (int)pvVar3;
                  if (((local_8 == (void *)0x0) || (local_14 == (void *)0x0)) ||
                     (pvVar3 == (void *)0x0)) goto LAB_0048284d;
                }
                puVar8 = local_10;
                local_38[0] = *(ushort *)((int)this + 0x30);
                *(int *)((int)this + 0x30) = *(int *)((int)this + 0x30) + 1;
                local_34 = (ushort)*param_2;
                iVar5 = DynArray_Grow((int *)&local_14,local_30 + 6,local_50 + 2,local_30 + 3);
                if ((iVar5 == 0) ||
                   (iVar5 = UshortArray_PushBack(&local_8,local_38,local_50 + 1,local_30 + 1),
                   iVar5 == 0)) goto LAB_0048284d;
                local_50[0] = (uint)*puVar1;
                iVar5 = DynArray_Grow(param_1,local_50,param_2,local_30);
                if (iVar5 == 0) goto LAB_0048284d;
                *(undefined2 *)((int)local_8 + (uint)local_34 * 2) =
                     *(undefined2 *)((int)puVar8 + (uint)*puVar1 * 2);
                *(ushort *)((int)puVar8 + (uint)*puVar1 * 2) = local_34;
                *puVar1 = local_38[0];
                psVar7 = local_3c;
              }
            }
LAB_0048281f:
            local_30[5] = local_30[5] + 1;
          } while (local_30[5] < 3);
        }
        local_30[4] = local_30[4] + 1;
        local_30[2] = local_30[2] + 6;
      } while (local_30[4] < *(uint *)((int)this + 0x58));
    }
  }
  _free(local_10);
  _free(local_8);
  _free(local_40);
  _free(local_14);
  return local_c;
}

