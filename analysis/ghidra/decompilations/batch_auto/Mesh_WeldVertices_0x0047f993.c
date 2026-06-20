
int Mesh_WeldVertices(void *param_1,int param_2,undefined4 *param_3)

{
  uint uVar1;
  uint *puVar2;
  uint uVar3;
  int iVar4;
  uint *puVar5;
  uint uVar6;
  int iVar7;
  int iVar8;
  undefined4 *puVar9;
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
  uint *local_20;
  uint *local_1c;
  int local_18;
  uint local_14;
  int local_10;
  uint *local_c;
  uint local_8;
  
  iVar4 = *(int *)((int)param_1 + 0x58);
  iVar8 = iVar4 * 3;
  uVar1 = *(uint *)((int)param_1 + 0x30) / 3;
  local_18 = 0;
  local_38 = 0;
  local_24 = 0;
  local_c = (uint *)0x0;
  local_3c = (void *)0x0;
  local_4c = iVar8;
  local_30 = uVar1;
  if ((param_3 == (undefined4 *)0x0) || ((*(byte *)((int)param_1 + 9) & 4) != 0)) {
    local_10 = -0x7789f794;
  }
  else {
    local_c = operator_new(uVar1 << 2);
    local_3c = operator_new(iVar4 * 0x3c);
    if ((local_c == (uint *)0x0) || (local_3c == (void *)0x0)) {
      local_10 = -0x7ff8fff2;
    }
    else {
      local_10 = Graphics_DrawIndexedPrimitive(param_1,&local_18,0x10);
      if (-1 < local_10) {
        local_10 = Graphics_DrawIndexedPrimitiveUP(param_1,&local_38,0x10);
        if (-1 < local_10) {
          puVar2 = local_c;
          for (uVar1 = uVar1 & 0x3fffffff; uVar1 != 0; uVar1 = uVar1 - 1) {
            *puVar2 = 0;
            puVar2 = puVar2 + 1;
          }
          for (iVar4 = 0; iVar4 != 0; iVar4 = iVar4 + -1) {
            *(undefined1 *)puVar2 = 0;
            puVar2 = (uint *)((int)puVar2 + 1);
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
              local_8 = *local_44;
              local_14 = local_44[1] + local_8;
              if (local_8 < local_14) {
                local_40 = local_8 * 0xc;
                local_20 = (uint *)((int)local_3c + local_24 * 0x14);
                do {
                  puVar2 = (uint *)(local_18 + local_40);
                  uVar1 = *puVar2;
                  if (uVar1 != 0xffffffff) {
                    if (param_2 == 0) {
                      local_5c[0] = uVar1;
                      local_5c[1] = puVar2[1];
                      local_5c[2] = puVar2[2];
                    }
                    else {
                      local_5c[0] = *(uint *)(param_2 + uVar1 * 4);
                      local_5c[1] = *(uint *)(param_2 + puVar2[1] * 4);
                      local_5c[2] = *(uint *)(param_2 + puVar2[2] * 4);
                    }
                    if (((local_5c[0] != local_5c[1]) && (local_5c[0] != local_5c[2])) &&
                       (local_5c[1] != local_5c[2])) {
                      uVar1 = 2;
                      local_1c = local_20;
                      local_24 = local_24 + 3;
                      local_20 = local_20 + 0xf;
                      puVar2 = local_5c;
                      local_34 = 3;
                      do {
                        puVar5 = local_1c;
                        uVar6 = *puVar2;
                        local_5c[3] = uVar6 % local_30;
                        local_1c[3] = local_8;
                        *local_1c = uVar6;
                        local_1c = local_1c + 5;
                        puVar2 = puVar2 + 1;
                        puVar5[1] = local_5c[(uVar1 - 1) % 3];
                        uVar6 = uVar1 % 3;
                        uVar1 = uVar1 + 1;
                        local_34 = local_34 + -1;
                        puVar5[2] = local_5c[uVar6];
                        puVar5[4] = local_c[local_5c[3]];
                        local_c[local_5c[3]] = (uint)puVar5;
                      } while (local_34 != 0);
                    }
                  }
                  local_8 = local_8 + 1;
                  local_40 = local_40 + 0xc;
                } while (local_8 < local_14);
              }
              local_44 = local_44 + 5;
              local_48 = local_48 + -1;
              iVar8 = local_4c;
            } while (local_48 != 0);
          }
          puVar9 = param_3;
          for (uVar1 = *(int *)((int)param_1 + 0x58) * 3 & 0x3fffffff; uVar1 != 0; uVar1 = uVar1 - 1
              ) {
            *puVar9 = 0xffffffff;
            puVar9 = puVar9 + 1;
          }
          for (iVar4 = 0; iVar4 != 0; iVar4 = iVar4 + -1) {
            *(undefined1 *)puVar9 = 0xff;
            puVar9 = (undefined4 *)((int)puVar9 + 1);
          }
          if (local_28 != 0) {
            local_44 = (uint *)(local_2c + 4);
            local_40 = local_28;
            do {
              local_8 = *local_44;
              uVar1 = local_44[1] + local_8;
              local_14 = uVar1;
              if (local_8 < uVar1) {
                iVar4 = local_8 * 0xc;
                iVar7 = (int)local_5c + (local_8 * -0xc - (int)param_3);
                local_28 = iVar4;
                local_20 = param_3 + local_8 * 3;
                local_34 = iVar7;
                do {
                  puVar2 = (uint *)(local_18 + iVar4);
                  uVar6 = *puVar2;
                  if (uVar6 != 0xffffffff) {
                    if (param_2 == 0) {
                      local_5c[0] = uVar6;
                      local_5c[1] = puVar2[1];
                      local_5c[2] = puVar2[2];
                    }
                    else {
                      local_5c[0] = *(uint *)(param_2 + uVar6 * 4);
                      local_5c[1] = *(uint *)(param_2 + puVar2[1] * 4);
                      local_5c[2] = *(uint *)(param_2 + puVar2[2] * 4);
                    }
                    if (((local_5c[0] == local_5c[1]) || (local_5c[0] == local_5c[2])) ||
                       (local_5c[1] == local_5c[2])) {
                      *local_20 = 0xffffffff;
                      local_20[1] = 0xffffffff;
                      local_20[2] = 0xffffffff;
                      uVar1 = local_14;
                    }
                    else {
                      local_2c = (undefined1 *)0x1;
                      local_48 = 3;
                      puVar2 = local_20;
                      local_34 = iVar7;
                      local_28 = iVar4;
                      do {
                        if (*puVar2 == 0xffffffff) {
                          uVar1 = *(uint *)((int)puVar2 + iVar7);
                          uVar6 = local_5c[(uint)local_2c % 3];
                          uVar3 = Mesh_FindBestDuplicateVertex
                                            (uVar6,uVar1,local_5c[(uint)(local_2c + 1) % 3],local_c,
                                             local_30,0xffffffff,local_38,
                                             *(int *)((int)param_1 + 0x2c));
                          *puVar2 = uVar3;
                          iVar4 = local_28;
                          iVar7 = local_34;
                          if (uVar3 != 0xffffffff) {
                            Mesh_RemoveEdgeFromHash(uVar1,uVar6,local_8,(int)local_c,local_30);
                            local_1c = (uint *)0x0;
                            puVar5 = (uint *)(local_18 + *puVar2 * 0xc);
                            do {
                              uVar1 = *puVar5;
                              if (param_2 != 0) {
                                uVar1 = *(uint *)(param_2 + uVar1 * 4);
                              }
                              if (uVar1 == uVar6) break;
                              local_1c = (uint *)((int)local_1c + 1);
                              puVar5 = puVar5 + 1;
                            } while (local_1c < (uint *)0x3);
                            param_3[(int)local_1c + *puVar2 * 3] = local_8;
                            iVar4 = local_28;
                            iVar7 = local_34;
                          }
                        }
                        local_2c = local_2c + 1;
                        puVar2 = puVar2 + 1;
                        local_48 = local_48 + -1;
                        uVar1 = local_14;
                      } while (local_48 != 0);
                    }
                  }
                  local_8 = local_8 + 1;
                  local_20 = local_20 + 3;
                  iVar4 = iVar4 + 0xc;
                  iVar7 = iVar7 + -0xc;
                  local_28 = iVar4;
                  local_34 = iVar7;
                  iVar8 = local_4c;
                } while (local_8 < uVar1);
              }
              local_44 = local_44 + 5;
              local_40 = local_40 + -1;
            } while (local_40 != 0);
          }
          if (iVar8 < local_24) {
            local_10 = -0x7fffbffb;
          }
        }
      }
    }
    if (local_18 != 0) {
      (**(code **)(**(int **)((int)param_1 + 0x3c) + 0x30))(*(int **)((int)param_1 + 0x3c));
    }
    if (local_38 != 0) {
      (**(code **)(**(int **)((int)param_1 + 0x28) + 0x30))(*(int **)((int)param_1 + 0x28));
    }
  }
  _free(local_c);
  _free(local_3c);
  return local_10;
}

