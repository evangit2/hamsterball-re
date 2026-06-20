
/* WARNING: Function: __chkstk replaced with injection: alloca_probe */

int __thiscall Mesh_SortByAttribute(void *this,int param_1,int param_2,int param_3,int param_4)

{
  uint *puVar1;
  int *piVar2;
  ushort uVar3;
  uint uVar4;
  void *pvVar5;
  int *piVar6;
  undefined4 *puVar7;
  uint uVar8;
  int iVar9;
  int iVar10;
  char acStack_130 [236];
  undefined4 uStack_44;
  uint local_24;
  void *local_20;
  uint local_1c;
  uint local_18;
  int local_14;
  uint local_10;
  undefined4 *local_c;
  undefined1 *local_8;
  
  local_14 = 0;
  local_c = (undefined4 *)0x0;
  local_8 = (undefined1 *)0x0;
  local_20 = (void *)0x0;
  if ((param_4 == 0) || (*(int *)((int)this + 0x50) == 0)) {
    local_14 = Mesh_CollectUniqueAttributes(this,&local_8,&local_24);
    uVar8 = local_24;
    if (local_14 < 0) goto LAB_0048256d;
    uStack_44 = 0x4822ad;
    CRT_qsort(local_8,local_24,4,&LAB_0047db01);
LAB_004822b0:
    local_c = operator_new(uVar8 << 4);
    puVar7 = local_c;
    uVar4 = uVar8;
    if (local_c != (undefined4 *)0x0) {
      for (; uVar4 != 0; uVar4 = uVar4 - 1) {
        *puVar7 = 0;
        puVar7[2] = 0;
        puVar7 = puVar7 + 4;
      }
      uVar4 = 0;
      if (*(int *)((int)this + 0x30) != 0) {
        do {
          *(undefined4 *)(param_3 + uVar4 * 4) = 0xffff;
          uVar4 = uVar4 + 1;
        } while (uVar4 < *(uint *)((int)this + 0x30));
      }
      local_18 = 0;
      local_1c = 0;
      if (uVar8 != 0) {
        do {
          if (*(int *)(local_8 + local_18 * 4) == 0) break;
          local_18 = local_18 + 1;
        } while (local_18 < uVar8);
      }
      local_10 = 0;
      if (*(int *)((int)this + 0x58) != 0) {
        uVar4 = 0;
        do {
          uVar3 = *(ushort *)(param_2 + uVar4 * 4);
          if (uVar3 != 0xffff) {
            iVar9 = *(int *)(*(int *)((int)this + 0x48) + (uint)uVar3 * 4);
            if ((iVar9 != local_1c) && (local_18 = 0, local_1c = iVar9, uVar8 != 0)) {
              do {
                if (*(int *)(local_8 + local_18 * 4) == iVar9) break;
                local_18 = local_18 + 1;
              } while (local_18 < uVar8);
            }
            iVar9 = (uint)uVar3 * 6;
            local_20 = (void *)0x3;
            do {
              puVar1 = (uint *)(param_3 + (uint)*(ushort *)(iVar9 + *(int *)((int)this + 0x40)) * 4)
              ;
              if (*puVar1 == 0xffff) {
                *puVar1 = local_18;
                local_c[local_18 * 4 + 2] = local_c[local_18 * 4 + 2] + 1;
              }
              iVar9 = iVar9 + 2;
              local_20 = (void *)((int)local_20 + -1);
            } while (local_20 != (void *)0x0);
            local_c[local_18 * 4] = local_c[local_18 * 4] + 1;
          }
          local_10 = local_10 + 1;
          uVar4 = local_10 & 0xffff;
        } while (uVar4 < *(uint *)((int)this + 0x58));
      }
      local_20 = *(void **)((int)this + 0x50);
      *(uint *)((int)this + 0x54) = uVar8;
      pvVar5 = operator_new(uVar8 * 0x14);
      *(void **)((int)this + 0x50) = pvVar5;
      if (pvVar5 != (void *)0x0) {
        local_18 = 0;
        local_10 = 0;
        local_1c = 0;
        uVar4 = 0;
        if (uVar8 != 0) {
          piVar6 = local_c + 2;
          iVar9 = 0;
          do {
            piVar6[-1] = local_18;
            *(undefined4 *)(iVar9 + *(int *)((int)this + 0x50)) =
                 *(undefined4 *)(local_8 + local_1c * 4);
            *(uint *)(iVar9 + 4 + *(int *)((int)this + 0x50)) = local_18;
            *(int *)(iVar9 + 8 + *(int *)((int)this + 0x50)) = piVar6[-2];
            local_18 = local_18 + piVar6[-2];
            piVar6[1] = local_10;
            *(uint *)(iVar9 + 0xc + *(int *)((int)this + 0x50)) = local_10;
            *(int *)(iVar9 + 0x10 + *(int *)((int)this + 0x50)) = *piVar6;
            local_10 = local_10 + *piVar6;
            local_1c = local_1c + 1;
            piVar6 = piVar6 + 4;
            iVar9 = iVar9 + 0x14;
            uVar4 = local_24;
          } while (local_1c < local_24);
        }
        if (((param_4 != 0) && (local_20 != (void *)0x0)) && (uVar4 != 0)) {
          iVar9 = 0;
          puVar7 = (undefined4 *)((int)local_20 + 0x10);
          local_24 = uVar4;
          do {
            *(undefined4 *)(*(int *)((int)this + 0x50) + 0xc + iVar9) = puVar7[-1];
            *(undefined4 *)(*(int *)((int)this + 0x50) + 0x10 + iVar9) = *puVar7;
            iVar9 = iVar9 + 0x14;
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
        local_10 = 0;
        if (*(int *)((int)this + 0x58) != 0) {
          uVar8 = 0;
          do {
            uVar3 = *(ushort *)(param_2 + uVar8 * 4);
            if (uVar3 != 0xffff) {
              iVar10 = (uint)uVar3 * 4;
              iVar9 = *(int *)(iVar10 + *(int *)((int)this + 0x48));
              if ((iVar9 != local_1c) && (local_18 = 0, local_1c = iVar9, uVar4 != 0)) {
                do {
                  if (*(int *)(local_8 + local_18 * 4) == iVar9) break;
                  local_18 = local_18 + 1;
                } while (local_18 < uVar4);
              }
              piVar6 = local_c + local_18 * 4 + 1;
              *(int *)(iVar10 + param_1) = *piVar6;
              *piVar6 = *piVar6 + 1;
            }
            local_10 = local_10 + 1;
            uVar8 = local_10 & 0xffff;
          } while (uVar8 < *(uint *)((int)this + 0x58));
        }
        if (param_4 == 0) {
          uVar8 = 0;
          if (*(int *)((int)this + 0x30) != 0) {
            do {
              piVar6 = (int *)(param_3 + uVar8 * 4);
              iVar9 = *piVar6;
              if (iVar9 != 0xffff) {
                piVar2 = local_c + iVar9 * 4 + 3;
                *piVar6 = *piVar2;
                *piVar2 = *piVar2 + 1;
              }
              uVar8 = uVar8 + 1;
            } while (uVar8 < *(uint *)((int)this + 0x30));
          }
        }
        else {
          uVar8 = 0;
          if (*(int *)((int)this + 0x30) != 0) {
            do {
              puVar1 = (uint *)(param_3 + uVar8 * 4);
              uVar4 = *puVar1;
              if (uVar4 != 0xffff) {
                iVar9 = *(int *)((int)this + 0x50) + uVar4 * 0x14;
                uVar4 = *(uint *)(iVar9 + 0xc);
                if ((uVar8 < uVar4) || (*(int *)(iVar9 + 0x10) + uVar4 <= uVar8)) {
                  AthenaString_SprintfToBuffer
                            (acStack_130,
                             (byte *)
                             "ID3DXMeshOptimize: Cannot do AttributeSort because %d vertex needs to be moved and either IgnoreVerts or ShareVB is set\n"
                            );
                  local_14 = -0x7789f4aa;
                  goto LAB_0048256d;
                }
                *puVar1 = uVar8;
              }
              uVar8 = uVar8 + 1;
            } while (uVar8 < *(uint *)((int)this + 0x30));
          }
        }
        uVar8 = 0;
        if (*(int *)((int)this + 0x58) != 0) {
          uVar4 = 0;
          do {
            uVar8 = uVar8 + 1;
            *(undefined4 *)(param_2 + uVar4 * 4) = 0xffff;
            uVar4 = uVar8 & 0xffff;
          } while (uVar4 < *(uint *)((int)this + 0x58));
        }
        uVar8 = 0;
        if (*(int *)((int)this + 0x58) != 0) {
          uVar4 = 0;
          do {
            uVar3 = *(ushort *)(param_1 + uVar4 * 4);
            if (uVar3 != 0xffff) {
              *(uint *)(param_2 + (uint)uVar3 * 4) = uVar4;
            }
            uVar8 = uVar8 + 1;
            uVar4 = uVar8 & 0xffff;
          } while (uVar4 < *(uint *)((int)this + 0x58));
        }
        *(uint *)((int)this + 0xc) = *(uint *)((int)this + 0xc) | 0x10;
        goto LAB_0048256d;
      }
    }
  }
  else {
    uVar8 = *(uint *)((int)this + 0x54);
    local_24 = uVar8;
    local_8 = operator_new(uVar8 << 2);
    if (local_8 != (undefined1 *)0x0) {
      uVar4 = 0;
      if (uVar8 != 0) {
        iVar9 = 0;
        do {
          *(undefined4 *)(local_8 + uVar4 * 4) = *(undefined4 *)(iVar9 + *(int *)((int)this + 0x50))
          ;
          uVar4 = uVar4 + 1;
          iVar9 = iVar9 + 0x14;
        } while (uVar4 < uVar8);
      }
      goto LAB_004822b0;
    }
  }
  local_14 = -0x7ff8fff2;
LAB_0048256d:
  _free(local_c);
  _free(local_8);
  _free(local_20);
  if (local_14 < 0) {
    *(uint *)((int)this + 0xc) = *(uint *)((int)this + 0xc) & 0xffffffef;
    _free(*(void **)((int)this + 0x50));
    *(undefined4 *)((int)this + 0x50) = 0;
  }
  return local_14;
}

