
/* WARNING: Function: __chkstk replaced with injection: alloca_probe */

int __thiscall MeshData_AttributeSort(void *this,int param_1,int param_2,int param_3,int param_4)

{
  uint *puVar1;
  int *piVar2;
  int iVar3;
  uint uVar4;
  void *pvVar5;
  int *piVar6;
  undefined4 *puVar7;
  int iVar8;
  uint uVar9;
  char acStack_130 [236];
  undefined4 uStack_44;
  uint local_24;
  void *local_20;
  uint local_1c;
  uint local_18;
  int local_14;
  undefined4 *local_10;
  uint local_c;
  undefined1 *local_8;
  
  local_14 = 0;
  local_10 = (undefined4 *)0x0;
  local_8 = (undefined1 *)0x0;
  local_20 = (void *)0x0;
  if ((param_4 == 0) || (*(int *)((int)this + 0x50) == 0)) {
    local_14 = MeshData_GetAttributeTable(this,&local_8,&local_24);
    uVar9 = local_24;
    if (local_14 < 0) goto LAB_00481d1f;
    uStack_44 = 0x481a81;
    CRT_qsort(local_8,local_24,4,&LAB_0047db01);
LAB_00481a84:
    local_10 = operator_new(uVar9 << 4);
    puVar7 = local_10;
    uVar4 = uVar9;
    if (local_10 != (undefined4 *)0x0) {
      for (; uVar4 != 0; uVar4 = uVar4 - 1) {
        *puVar7 = 0;
        puVar7[2] = 0;
        puVar7 = puVar7 + 4;
      }
      uVar4 = 0;
      if (*(int *)((int)this + 0x30) != 0) {
        do {
          *(undefined4 *)(param_3 + uVar4 * 4) = 0xffffffff;
          uVar4 = uVar4 + 1;
        } while (uVar4 < *(uint *)((int)this + 0x30));
      }
      local_18 = 0;
      local_1c = 0;
      if (uVar9 != 0) {
        do {
          if (*(int *)(local_8 + local_18 * 4) == 0) break;
          local_18 = local_18 + 1;
        } while (local_18 < uVar9);
      }
      local_c = 0;
      if (*(int *)((int)this + 0x58) != 0) {
        do {
          iVar8 = *(int *)(param_2 + local_c * 4);
          if (iVar8 != -1) {
            iVar3 = *(int *)(*(int *)((int)this + 0x48) + iVar8 * 4);
            if ((iVar3 != local_1c) && (local_18 = 0, local_1c = iVar3, uVar9 != 0)) {
              do {
                if (*(int *)(local_8 + local_18 * 4) == iVar3) break;
                local_18 = local_18 + 1;
              } while (local_18 < uVar9);
            }
            iVar8 = iVar8 * 0xc;
            local_20 = (void *)0x3;
            do {
              puVar1 = (uint *)(param_3 + *(int *)(iVar8 + *(int *)((int)this + 0x40)) * 4);
              if (*puVar1 == 0xffffffff) {
                *puVar1 = local_18;
                local_10[local_18 * 4 + 2] = local_10[local_18 * 4 + 2] + 1;
              }
              iVar8 = iVar8 + 4;
              local_20 = (void *)((int)local_20 + -1);
            } while (local_20 != (void *)0x0);
            local_10[local_18 * 4] = local_10[local_18 * 4] + 1;
          }
          local_c = local_c + 1;
        } while (local_c < *(uint *)((int)this + 0x58));
      }
      local_20 = *(void **)((int)this + 0x50);
      *(uint *)((int)this + 0x54) = uVar9;
      pvVar5 = operator_new(uVar9 * 0x14);
      *(void **)((int)this + 0x50) = pvVar5;
      if (pvVar5 != (void *)0x0) {
        local_18 = 0;
        local_c = 0;
        local_1c = 0;
        uVar4 = 0;
        if (uVar9 != 0) {
          piVar6 = local_10 + 2;
          iVar8 = 0;
          do {
            piVar6[-1] = local_18;
            *(undefined4 *)(iVar8 + *(int *)((int)this + 0x50)) =
                 *(undefined4 *)(local_8 + local_1c * 4);
            *(uint *)(iVar8 + 4 + *(int *)((int)this + 0x50)) = local_18;
            *(int *)(iVar8 + 8 + *(int *)((int)this + 0x50)) = piVar6[-2];
            local_18 = local_18 + piVar6[-2];
            piVar6[1] = local_c;
            *(uint *)(iVar8 + 0xc + *(int *)((int)this + 0x50)) = local_c;
            *(int *)(iVar8 + 0x10 + *(int *)((int)this + 0x50)) = *piVar6;
            local_c = local_c + *piVar6;
            local_1c = local_1c + 1;
            piVar6 = piVar6 + 4;
            iVar8 = iVar8 + 0x14;
            uVar4 = local_24;
          } while (local_1c < local_24);
        }
        if (((param_4 != 0) && (local_20 != (void *)0x0)) && (uVar4 != 0)) {
          iVar8 = 0;
          puVar7 = (undefined4 *)((int)local_20 + 0x10);
          local_24 = uVar4;
          do {
            *(undefined4 *)(*(int *)((int)this + 0x50) + 0xc + iVar8) = puVar7[-1];
            *(undefined4 *)(*(int *)((int)this + 0x50) + 0x10 + iVar8) = *puVar7;
            iVar8 = iVar8 + 0x14;
            puVar7 = puVar7 + 5;
            local_24 = local_24 - 1;
          } while (local_24 != 0);
          local_24 = 0;
        }
        local_18 = 0;
        local_1c = 0;
        if (uVar4 != 0) {
          do {
            if (*(int *)(local_8 + local_18 * 4) == 0) break;
            local_18 = local_18 + 1;
          } while (local_18 < uVar4);
        }
        local_c = 0;
        if (*(int *)((int)this + 0x58) != 0) {
          do {
            iVar8 = *(int *)(param_2 + local_c * 4);
            if (iVar8 != -1) {
              iVar8 = iVar8 * 4;
              iVar3 = *(int *)(iVar8 + *(int *)((int)this + 0x48));
              if ((iVar3 != local_1c) && (local_18 = 0, local_1c = iVar3, uVar4 != 0)) {
                do {
                  if (*(int *)(local_8 + local_18 * 4) == iVar3) break;
                  local_18 = local_18 + 1;
                } while (local_18 < uVar4);
              }
              piVar6 = local_10 + local_18 * 4 + 1;
              *(int *)(iVar8 + param_1) = *piVar6;
              *piVar6 = *piVar6 + 1;
            }
            local_c = local_c + 1;
          } while (local_c < *(uint *)((int)this + 0x58));
        }
        if (param_4 == 0) {
          uVar9 = 0;
          if (*(int *)((int)this + 0x30) != 0) {
            do {
              piVar6 = (int *)(param_3 + uVar9 * 4);
              iVar8 = *piVar6;
              if (iVar8 != -1) {
                piVar2 = local_10 + iVar8 * 4 + 3;
                *piVar6 = *piVar2;
                *piVar2 = *piVar2 + 1;
              }
              uVar9 = uVar9 + 1;
            } while (uVar9 < *(uint *)((int)this + 0x30));
          }
        }
        else {
          uVar9 = 0;
          if (*(int *)((int)this + 0x30) != 0) {
            do {
              puVar1 = (uint *)(param_3 + uVar9 * 4);
              uVar4 = *puVar1;
              if (uVar4 != 0xffffffff) {
                iVar8 = *(int *)((int)this + 0x50) + uVar4 * 0x14;
                uVar4 = *(uint *)(iVar8 + 0xc);
                if ((uVar9 < uVar4) || (*(int *)(iVar8 + 0x10) + uVar4 <= uVar9)) {
                  AthenaString_SprintfToBuffer
                            (acStack_130,
                             (byte *)
                             "ID3DXMeshOptimize: Cannot do AttributeSort because %d vertex needs to be moved and either IgnoreVerts or ShareVB is set\n"
                            );
                  local_14 = -0x7789f4aa;
                  goto LAB_00481d1f;
                }
                *puVar1 = uVar9;
              }
              uVar9 = uVar9 + 1;
            } while (uVar9 < *(uint *)((int)this + 0x30));
          }
        }
        uVar9 = 0;
        if (*(int *)((int)this + 0x58) != 0) {
          do {
            *(undefined4 *)(param_2 + uVar9 * 4) = 0xffffffff;
            uVar9 = uVar9 + 1;
          } while (uVar9 < *(uint *)((int)this + 0x58));
        }
        uVar9 = 0;
        if (*(int *)((int)this + 0x58) != 0) {
          do {
            iVar8 = *(int *)(param_1 + uVar9 * 4);
            if (iVar8 != -1) {
              *(uint *)(param_2 + iVar8 * 4) = uVar9;
            }
            uVar9 = uVar9 + 1;
          } while (uVar9 < *(uint *)((int)this + 0x58));
        }
        *(uint *)((int)this + 0xc) = *(uint *)((int)this + 0xc) | 0x10;
        goto LAB_00481d1f;
      }
    }
  }
  else {
    uVar9 = *(uint *)((int)this + 0x54);
    local_24 = uVar9;
    local_8 = operator_new(uVar9 << 2);
    if (local_8 != (undefined1 *)0x0) {
      uVar4 = 0;
      if (uVar9 != 0) {
        iVar8 = 0;
        do {
          *(undefined4 *)(local_8 + uVar4 * 4) = *(undefined4 *)(iVar8 + *(int *)((int)this + 0x50))
          ;
          uVar4 = uVar4 + 1;
          iVar8 = iVar8 + 0x14;
        } while (uVar4 < uVar9);
      }
      goto LAB_00481a84;
    }
  }
  local_14 = -0x7ff8fff2;
LAB_00481d1f:
  _free(local_10);
  _free(local_8);
  _free(local_20);
  if (local_14 < 0) {
    *(uint *)((int)this + 0xc) = *(uint *)((int)this + 0xc) & 0xffffffef;
    _free(*(void **)((int)this + 0x50));
    *(undefined4 *)((int)this + 0x50) = 0;
  }
  return local_14;
}

