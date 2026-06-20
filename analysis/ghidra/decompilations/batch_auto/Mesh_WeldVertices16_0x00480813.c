
int Mesh_WeldVertices16(void *param_1,int param_2,undefined4 *param_3)

{
  ushort uVar1;
  uint *puVar2;
  uint uVar3;
  ushort *puVar4;
  uint uVar5;
  int iVar6;
  uint uVar7;
  undefined1 *puVar8;
  int iVar9;
  uint *puVar10;
  undefined4 *puVar11;
  undefined1 local_70 [4];
  undefined4 local_6c;
  undefined4 local_68;
  undefined4 local_64;
  undefined4 local_60;
  uint local_5c [4];
  int local_4c;
  int local_48;
  uint *local_44;
  int local_40;
  void *local_3c;
  int local_38;
  int local_34;
  uint local_30;
  undefined1 *local_2c;
  int local_28;
  int local_24;
  uint local_20;
  uint *local_1c;
  uint *local_18;
  uint local_14;
  int local_10;
  int local_c;
  uint *local_8;
  
  iVar6 = *(int *)((int)param_1 + 0x58);
  iVar9 = iVar6 * 3;
  uVar3 = *(uint *)((int)param_1 + 0x30) / 3;
  local_10 = 0;
  local_38 = 0;
  local_24 = 0;
  local_8 = (uint *)0x0;
  local_3c = (void *)0x0;
  local_4c = iVar9;
  local_30 = uVar3;
  if ((param_3 == (undefined4 *)0x0) || ((*(byte *)((int)param_1 + 9) & 4) != 0)) {
    local_c = -0x7789f794;
  }
  else {
    local_8 = operator_new(uVar3 << 2);
    local_3c = operator_new(iVar6 * 0x3c);
    if ((local_8 == (uint *)0x0) || (local_3c == (void *)0x0)) {
      local_c = -0x7ff8fff2;
    }
    else {
      local_c = Graphics_DrawIndexedPrimitive(param_1,&local_10,0x10);
      if (-1 < local_c) {
        local_c = Graphics_DrawIndexedPrimitiveUP(param_1,&local_38,0x10);
        if (-1 < local_c) {
          puVar10 = local_8;
          for (uVar3 = uVar3 & 0x3fffffff; uVar3 != 0; uVar3 = uVar3 - 1) {
            *puVar10 = 0;
            puVar10 = puVar10 + 1;
          }
          for (iVar6 = 0; iVar6 != 0; iVar6 = iVar6 + -1) {
            *(undefined1 *)puVar10 = 0;
            puVar10 = (uint *)((int)puVar10 + 1);
          }
          if (*(int *)((int)param_1 + 0x54) == 0) {
            local_6c = 0;
            local_64 = 0;
            local_2c = local_70;
            local_68 = *(undefined4 *)((int)param_1 + 0x58);
            local_28 = 1;
            local_60 = *(undefined4 *)((int)param_1 + 0x30);
          }
          else {
            local_28 = *(int *)((int)param_1 + 0x54);
            local_2c = *(undefined1 **)((int)param_1 + 0x50);
          }
          if (local_28 != 0) {
            local_44 = (uint *)(local_2c + 4);
            local_48 = local_28;
            do {
              local_14 = *local_44;
              local_20 = local_44[1] + local_14;
              if (local_14 < local_20) {
                local_40 = local_14 * 6;
                local_1c = (uint *)((int)local_3c + local_24 * 0x14);
                do {
                  puVar4 = (ushort *)(local_10 + local_40);
                  uVar1 = *puVar4;
                  if (uVar1 != 0xffff) {
                    if (param_2 == 0) {
                      local_5c[2] = (uint)puVar4[2];
                      local_5c[0] = (uint)uVar1;
                      local_5c[1] = (uint)puVar4[1];
                    }
                    else {
                      local_5c[0] = *(uint *)(param_2 + (uint)uVar1 * 4);
                      local_5c[1] = *(uint *)(param_2 + (uint)puVar4[1] * 4);
                      local_5c[2] = *(uint *)(param_2 + (uint)puVar4[2] * 4);
                    }
                    if (((local_5c[0] != local_5c[1]) && (local_5c[0] != local_5c[2])) &&
                       (local_5c[1] != local_5c[2])) {
                      uVar3 = 2;
                      local_18 = local_1c;
                      local_24 = local_24 + 3;
                      local_1c = local_1c + 0xf;
                      puVar10 = local_5c;
                      local_34 = 3;
                      do {
                        puVar2 = local_18;
                        uVar7 = *puVar10;
                        local_5c[3] = uVar7 % local_30;
                        local_18[3] = local_14;
                        *local_18 = uVar7;
                        local_18 = local_18 + 5;
                        puVar10 = puVar10 + 1;
                        puVar2[1] = local_5c[(uVar3 - 1) % 3];
                        uVar7 = uVar3 % 3;
                        uVar3 = uVar3 + 1;
                        local_34 = local_34 + -1;
                        puVar2[2] = local_5c[uVar7];
                        puVar2[4] = local_8[local_5c[3]];
                        local_8[local_5c[3]] = (uint)puVar2;
                      } while (local_34 != 0);
                    }
                  }
                  local_14 = local_14 + 1;
                  local_40 = local_40 + 6;
                  iVar9 = local_4c;
                } while (local_14 < local_20);
              }
              local_44 = local_44 + 5;
              local_48 = local_48 + -1;
            } while (local_48 != 0);
          }
          puVar11 = param_3;
          for (uVar3 = *(int *)((int)param_1 + 0x58) * 3 & 0x3fffffff; uVar3 != 0; uVar3 = uVar3 - 1
              ) {
            *puVar11 = 0xffffffff;
            puVar11 = puVar11 + 1;
          }
          for (iVar6 = 0; iVar6 != 0; iVar6 = iVar6 + -1) {
            *(undefined1 *)puVar11 = 0xff;
            puVar11 = (undefined4 *)((int)puVar11 + 1);
          }
          if (local_28 != 0) {
            local_44 = (uint *)(local_2c + 4);
            local_40 = local_28;
            do {
              uVar3 = *local_44;
              local_20 = local_44[1] + uVar3;
              if (uVar3 < local_44[1] + uVar3) {
                local_1c = param_3 + uVar3 * 3;
                local_28 = uVar3 * 6;
                iVar6 = (int)local_5c + (uVar3 * -0xc - (int)param_3);
                local_34 = iVar6;
                do {
                  puVar4 = (ushort *)(local_10 + local_28);
                  uVar1 = *puVar4;
                  if (uVar1 != 0xffff) {
                    if (param_2 == 0) {
                      local_5c[1] = (uint)puVar4[1];
                      local_5c[0] = (uint)uVar1;
                      local_5c[2] = (uint)puVar4[2];
                    }
                    else {
                      local_5c[0] = *(uint *)(param_2 + (uint)uVar1 * 4);
                      local_5c[1] = *(uint *)(param_2 + (uint)puVar4[1] * 4);
                      local_5c[2] = *(uint *)(param_2 + (uint)puVar4[2] * 4);
                    }
                    if (((local_5c[0] == local_5c[1]) || (local_5c[0] == local_5c[2])) ||
                       (local_5c[1] == local_5c[2])) {
                      *local_1c = 0xffffffff;
                      local_1c[1] = 0xffffffff;
                      local_1c[2] = 0xffffffff;
                    }
                    else {
                      local_18 = (uint *)0x1;
                      local_48 = 3;
                      puVar10 = local_1c;
                      local_34 = iVar6;
                      do {
                        if (*puVar10 == 0xffffffff) {
                          uVar7 = *(uint *)(iVar6 + (int)puVar10);
                          local_2c = (undefined1 *)local_5c[(uint)local_18 % 3];
                          uVar5 = Mesh_FindBestDuplicateVertex
                                            (local_5c[(uint)local_18 % 3],uVar7,
                                             local_5c[((int)local_18 + 1U) % 3],local_8,local_30,
                                             0xffffffff,local_38,*(int *)((int)param_1 + 0x2c));
                          *puVar10 = uVar5;
                          iVar6 = local_34;
                          if (uVar5 != 0xffffffff) {
                            Mesh_RemoveEdgeFromHash
                                      (uVar7,(uint)local_2c,uVar3,(int)local_8,local_30);
                            local_14 = 0;
                            puVar4 = (ushort *)(local_10 + *puVar10 * 6);
                            do {
                              puVar8 = (undefined1 *)(uint)*puVar4;
                              if (param_2 != 0) {
                                puVar8 = *(undefined1 **)(param_2 + (int)puVar8 * 4);
                              }
                              if (puVar8 == local_2c) break;
                              local_14 = local_14 + 1;
                              puVar4 = puVar4 + 1;
                            } while (local_14 < 3);
                            param_3[local_14 + *puVar10 * 3] = uVar3;
                            iVar6 = local_34;
                          }
                        }
                        local_18 = (uint *)((int)local_18 + 1);
                        puVar10 = puVar10 + 1;
                        local_48 = local_48 + -1;
                      } while (local_48 != 0);
                    }
                  }
                  local_1c = local_1c + 3;
                  uVar3 = uVar3 + 1;
                  local_28 = local_28 + 6;
                  iVar6 = iVar6 + -0xc;
                  local_34 = iVar6;
                  iVar9 = local_4c;
                } while (uVar3 < local_20);
              }
              local_44 = local_44 + 5;
              local_40 = local_40 + -1;
            } while (local_40 != 0);
          }
          if (iVar9 < local_24) {
            local_c = -0x7fffbffb;
          }
        }
      }
    }
    if (local_10 != 0) {
      (**(code **)(**(int **)((int)param_1 + 0x3c) + 0x30))(*(int **)((int)param_1 + 0x3c));
    }
    if (local_38 != 0) {
      (**(code **)(**(int **)((int)param_1 + 0x28) + 0x30))(*(int **)((int)param_1 + 0x28));
    }
  }
  _free(local_8);
  _free(local_3c);
  return local_c;
}

