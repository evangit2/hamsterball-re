
int __thiscall
D3DXMesh_OptimizeFull
          (void *this,uint *param_1,uint *param_2,uint *param_3,uint *param_4,undefined4 *param_5,
          int *param_6,uint *param_7)

{
  uint uVar1;
  void *pvVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  int *piVar6;
  uint *puVar7;
  uint *puVar8;
  undefined1 local_174 [4];
  int local_170;
  uint local_50;
  uint local_4c;
  uint local_48;
  void *local_44;
  void *local_40;
  uint local_3c;
  uint local_38;
  void *local_34;
  int *local_30;
  uint *local_2c;
  int local_28;
  int local_24;
  int *local_20;
  uint local_1c;
  uint *local_18;
  void *local_14;
  uint *local_10;
  uint *local_c;
  int local_8;
  
  local_50 = *(uint *)((int)this + 0x58);
  local_10 = (uint *)0x0;
  local_c = (uint *)0x0;
  local_2c = (uint *)0x0;
  local_14 = (void *)0x0;
  local_44 = (void *)0x0;
  local_18 = (uint *)0x0;
  local_40 = (void *)0x0;
  local_1c = 0;
  local_38 = 0;
  local_20 = (int *)0x0;
  local_28 = 0;
  local_24 = 0;
  local_30 = (int *)0x0;
  local_34 = (void *)0x0;
  if ((((param_1 == (uint *)0x0) || (((uint)param_1 & 0xffefff) != 0)) ||
      ((param_2 == (uint *)0x0 && (param_3 != (uint *)0x0)))) ||
     ((((uint)param_1 & 0x40000000) != 0 && (*(int *)((int)this + 0x54) == 0)))) {
    local_8 = -0x7789f794;
    goto LAB_004863cd;
  }
  if (((uint)param_1 & 0xc000000) != 0) {
    param_1 = (uint *)((uint)param_1 | 0x3000000);
  }
  local_3c = (uint)param_1 & 0x2000000;
  if ((local_3c != 0) && (((uint)param_1 & 0x40000000) == 0)) {
    local_38 = *(uint *)((int)this + 0x30);
    local_8 = MeshData_SplitVerticesByAttribute(this,(int *)&local_40,&local_1c);
    if (local_8 < 0) goto LAB_004863cd;
    if ((local_1c != 0) && (((uint)param_1 & 0x10000000) != 0)) {
      local_8 = -0x7789f4aa;
      goto LAB_004863cd;
    }
  }
  local_10 = operator_new(*(int *)((int)this + 0x58) << 2);
  local_c = operator_new(*(int *)((int)this + 0x58) << 2);
  local_14 = operator_new(*(int *)((int)this + 0x30) << 2);
  if (((local_10 != (uint *)0x0) && (local_c != (uint *)0x0)) && (local_14 != (void *)0x0)) {
    local_48 = *(uint *)((int)this + 0x30);
    local_4c = (uint)param_1 & 0xc000000;
    if ((local_4c == 0) ||
       (local_2c = operator_new(*(int *)((int)this + 0x58) << 2), local_2c != (uint *)0x0)) {
      if (((uint)param_1 & 0x40000000) == 0) {
        local_8 = MeshData_GenerateAdjacency(this,(int)local_10,(int)local_c,(int)local_14);
        if (local_8 < 0) goto LAB_004863cd;
      }
      else {
        uVar1 = 0;
        if (*(int *)((int)this + 0x58) != 0) {
          puVar7 = local_10;
          do {
            *(uint *)(((int)local_c - (int)local_10) + (int)puVar7) = uVar1;
            *puVar7 = uVar1;
            uVar1 = uVar1 + 1;
            puVar7 = puVar7 + 1;
          } while (uVar1 < *(uint *)((int)this + 0x58));
        }
        uVar1 = 0;
        if (*(int *)((int)this + 0x30) != 0) {
          do {
            *(uint *)((int)local_14 + uVar1 * 4) = uVar1;
            uVar1 = uVar1 + 1;
          } while (uVar1 < *(uint *)((int)this + 0x30));
        }
      }
      if (((local_3c != 0) && (((uint)param_1 & 0x40000000) == 0)) &&
         (local_8 = MeshData_AttributeSort
                              (this,(int)local_10,(int)local_c,(int)local_14,
                               (uint)param_1 & 0x10000000), local_8 < 0)) goto LAB_004863cd;
      local_3c = (uint)param_1 & 0x10000000;
      if ((local_3c != 0) && (uVar1 = 0, *(int *)((int)this + 0x30) != 0)) {
        do {
          *(uint *)((int)local_14 + uVar1 * 4) = uVar1;
          uVar1 = uVar1 + 1;
        } while (uVar1 < *(uint *)((int)this + 0x30));
      }
      local_8 = Mesh_OptimizeVertices(this,(int)local_10,(int)local_14);
      if (local_8 < 0) goto LAB_004863cd;
      if (local_4c != 0) {
        puVar7 = local_c;
        puVar8 = local_2c;
        for (uVar1 = *(uint *)((int)this + 0x58) & 0x3fffffff; uVar1 != 0; uVar1 = uVar1 - 1) {
          *puVar8 = *puVar7;
          puVar7 = puVar7 + 1;
          puVar8 = puVar8 + 1;
        }
        for (iVar3 = 0; iVar3 != 0; iVar3 = iVar3 + -1) {
          *(char *)puVar8 = (char)*puVar7;
          puVar7 = (uint *)((int)puVar7 + 1);
          puVar8 = (uint *)((int)puVar8 + 1);
        }
        local_8 = D3DXMesh_ConvertAdjacencyToStrip();
        if (local_8 < 0) goto LAB_004863cd;
        pvVar2 = operator_new(*(int *)((int)this + 0x30) << 2);
        local_44 = pvVar2;
        if (pvVar2 == (void *)0x0) goto LAB_004863c5;
        if (local_3c == 0) {
          local_8 = MeshData_GenerateVertexRemap(this,(int)local_c,(int)pvVar2);
          if (local_8 < 0) goto LAB_004863cd;
          uVar1 = 0;
          if (*(int *)((int)this + 0x30) != 0) {
            do {
              piVar6 = (int *)((int)local_14 + uVar1 * 4);
              iVar3 = *piVar6;
              if (iVar3 != -1) {
                *piVar6 = *(int *)((int)pvVar2 + iVar3 * 4);
              }
              uVar1 = uVar1 + 1;
            } while (uVar1 < *(uint *)((int)this + 0x30));
          }
        }
        else {
          uVar1 = 0;
          if (*(int *)((int)this + 0x30) != 0) {
            do {
              *(uint *)((int)pvVar2 + uVar1 * 4) = uVar1;
              uVar1 = uVar1 + 1;
            } while (uVar1 < *(uint *)((int)this + 0x30));
          }
        }
        puVar7 = local_10;
        local_8 = Mesh_OptimizeVertices(this,(int)local_10,(int)pvVar2);
        if (local_8 < 0) goto LAB_004863cd;
        uVar1 = 0;
        if (*(int *)((int)this + 0x58) != 0) {
          do {
            puVar8 = local_c + uVar1;
            if (*puVar8 == 0xffffffff) {
              *puVar8 = 0xffffffff;
            }
            else {
              *puVar8 = local_2c[*puVar8];
            }
            uVar1 = uVar1 + 1;
          } while (uVar1 < *(uint *)((int)this + 0x58));
        }
        uVar1 = 0;
        if (*(int *)((int)this + 0x58) != 0) {
          do {
            puVar7[uVar1] = 0xffffffff;
            uVar1 = uVar1 + 1;
          } while (uVar1 < *(uint *)((int)this + 0x58));
        }
        uVar1 = 0;
        if (*(int *)((int)this + 0x58) != 0) {
          do {
            if (local_c[uVar1] != 0xffffffff) {
              puVar7[local_c[uVar1]] = uVar1;
            }
            uVar1 = uVar1 + 1;
          } while (uVar1 < *(uint *)((int)this + 0x58));
        }
      }
      if (((param_5 != (undefined4 *)0x0) || (local_1c != 0)) || (*(int *)((int)this + 0x34) == 0))
      {
        local_8 = D3DX_CreateErrorHandler(*(int *)((int)this + 0x30) << 2,&local_20);
        if (local_8 < 0) goto LAB_004863cd;
        local_28 = (**(code **)(*local_20 + 0xc))(local_20);
        uVar1 = 0;
        if (*(int *)((int)this + 0x30) != 0) {
          do {
            *(undefined4 *)(local_28 + uVar1 * 4) = 0xffffffff;
            uVar1 = uVar1 + 1;
          } while (uVar1 < *(uint *)((int)this + 0x30));
        }
        uVar1 = 0;
        if (local_48 != 0) {
          do {
            iVar3 = *(int *)((int)local_14 + uVar1 * 4);
            if (iVar3 != -1) {
              *(uint *)(local_28 + iVar3 * 4) = uVar1;
            }
            uVar1 = uVar1 + 1;
          } while (uVar1 < local_48);
        }
        if ((local_1c != 0) && (uVar1 = 0, *(int *)((int)this + 0x30) != 0)) {
          do {
            uVar5 = *(uint *)(local_28 + uVar1 * 4);
            if ((uVar5 != 0xffffffff) && (local_38 <= uVar5)) {
              *(undefined4 *)(local_28 + uVar1 * 4) =
                   *(undefined4 *)((int)local_40 + (uVar5 - local_38) * 4);
            }
            uVar1 = uVar1 + 1;
          } while (uVar1 < *(uint *)((int)this + 0x30));
        }
      }
      if ((local_1c == 0) && (*(int *)((int)this + 0x34) != 0)) {
        local_8 = Graphics_DrawIndexedPrimitiveUP(this,&local_18,0);
        if ((local_8 < 0) ||
           (local_8 = Mesh_OptimizeFaces(this,(int)local_14,(int)local_18), local_8 < 0))
        goto LAB_004863cd;
      }
      else if ((*(byte *)((int)this + 0xc) & 0x20) == 0) {
        local_34 = *(void **)((int)this + 0x38);
        local_30 = *(int **)((int)this + 0x28);
        uVar1 = *(uint *)((int)this + 0x30);
        piVar6 = local_30;
        if (*(int *)((int)this + 0x34) == 0) {
          local_30 = param_6;
          (**(code **)(*param_6 + 4))(param_6);
          piVar6 = param_6;
        }
        *(undefined4 *)((int)this + 0x28) = 0;
        *(undefined4 *)((int)this + 0x30) = 0;
        *(undefined4 *)((int)this + 0x34) = 0;
        *(undefined4 *)((int)this + 0x38) = 0;
        local_8 = D3DTexture_ResizeAndValidate(this,*(uint *)((int)this + 0x58),uVar1);
        if (((local_8 < 0) ||
            (local_8 = Graphics_DrawIndexedPrimitiveUP(this,&local_18,0), local_8 < 0)) ||
           (local_8 = (**(code **)(*piVar6 + 0x2c))(piVar6,0,0,&local_24,0x810), local_8 < 0))
        goto LAB_004863cd;
        if (*(uint **)((int)this + 4) == param_7) {
          param_7 = local_18;
          uVar1 = 0;
          if (*(int *)((int)this + 0x30) != 0) {
            uVar5 = *(uint *)((int)this + 0x2c);
            do {
              puVar7 = (uint *)(*(int *)(local_28 + uVar1 * 4) * uVar5 + local_24);
              puVar8 = param_7;
              for (uVar4 = uVar5 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
                *puVar8 = *puVar7;
                puVar7 = puVar7 + 1;
                puVar8 = puVar8 + 1;
              }
              for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
                *(char *)puVar8 = (char)*puVar7;
                puVar7 = (uint *)((int)puVar7 + 1);
                puVar8 = (uint *)((int)puVar8 + 1);
              }
              uVar5 = *(uint *)((int)this + 0x2c);
              param_7 = (uint *)((int)param_7 + uVar5);
              uVar1 = uVar1 + 1;
            } while (uVar1 < *(uint *)((int)this + 0x30));
          }
        }
        else {
          D3D_InitDisplayModes(local_174,(uint)param_7,(uint)*(uint **)((int)this + 4));
          uVar1 = 0;
          puVar7 = local_18;
          if (*(int *)((int)this + 0x30) != 0) {
            do {
              VertexDecl_CopyVertexData
                        (local_174,
                         (undefined4 *)(*(int *)(local_28 + uVar1 * 4) * local_170 + local_24),
                         puVar7);
              puVar7 = (uint *)((int)puVar7 + *(int *)((int)this + 0x2c));
              uVar1 = uVar1 + 1;
            } while (uVar1 < *(uint *)((int)this + 0x30));
          }
        }
        uVar1 = 0;
        if ((local_34 != (void *)0x0) && (*(int *)((int)this + 0x30) != 0)) {
          do {
            *(undefined4 *)(*(int *)((int)this + 0x38) + uVar1 * 4) =
                 *(undefined4 *)
                  ((int)local_14 + *(int *)((int)local_34 + *(int *)(local_28 + uVar1 * 4) * 4) * 4)
            ;
            uVar1 = uVar1 + 1;
          } while (uVar1 < *(uint *)((int)this + 0x30));
        }
        uVar1 = 0;
        iVar3 = 0;
        if (*(int *)((int)this + 0x30) != 0) {
          do {
            piVar6 = (int *)((int)local_14 + uVar1 * 4);
            if (*piVar6 == -1) {
              *piVar6 = -1;
            }
            else {
              iVar3 = iVar3 + 1;
            }
            uVar1 = uVar1 + 1;
          } while (uVar1 < *(uint *)((int)this + 0x30));
        }
        *(int *)((int)this + 0x30) = iVar3;
      }
      if (param_4 != (uint *)0x0) {
        uVar1 = *(uint *)((int)this + 0x58) & 0x3fffffff;
        puVar7 = local_10;
        if (((uint)param_1 & 0x80000000) == 0) {
          puVar7 = local_c;
        }
        for (; uVar1 != 0; uVar1 = uVar1 - 1) {
          *param_4 = *puVar7;
          puVar7 = puVar7 + 1;
          param_4 = param_4 + 1;
        }
        for (iVar3 = 0; iVar3 != 0; iVar3 = iVar3 + -1) {
          *(char *)param_4 = (char)*puVar7;
          puVar7 = (uint *)((int)puVar7 + 1);
          param_4 = (uint *)((int)param_4 + 1);
        }
      }
      uVar1 = 0;
      if (param_3 != (uint *)0x0) {
        if (param_3 == param_2) {
          local_8 = Mesh_ReorderFacesByAdjacency((int)local_10,(int)param_3,local_50);
          if (local_8 < 0) goto LAB_004863cd;
        }
        else if (*(int *)((int)this + 0x58) != 0) {
          param_1 = param_3;
          do {
            uVar5 = local_c[uVar1];
            param_3 = (uint *)0x3;
            puVar7 = param_1;
            do {
              iVar3 = *(int *)((int)param_2 + (uVar5 * 0xc - (int)param_1) + (int)puVar7);
              if (iVar3 == -1) {
                *puVar7 = 0xffffffff;
              }
              else {
                *puVar7 = local_10[iVar3];
              }
              puVar7 = puVar7 + 1;
              param_3 = (uint *)((int)param_3 + -1);
            } while (param_3 != (uint *)0x0);
            param_1 = param_1 + 3;
            uVar1 = uVar1 + 1;
          } while (uVar1 < *(uint *)((int)this + 0x58));
        }
      }
      piVar6 = local_20;
      if (param_5 != (undefined4 *)0x0) {
        local_20 = (int *)0x0;
        *param_5 = piVar6;
      }
      goto LAB_004863cd;
    }
  }
LAB_004863c5:
  local_8 = -0x7ff8fff2;
LAB_004863cd:
  _free(local_14);
  _free(local_10);
  _free(local_c);
  _free(local_2c);
  _free(local_40);
  _free(local_44);
  if (*(void **)((int)this + 0x38) != local_34) {
    _free(local_34);
  }
  if (local_18 != (uint *)0x0) {
    (**(code **)(**(int **)((int)this + 0x28) + 0x30))(*(int **)((int)this + 0x28));
  }
  piVar6 = local_30;
  if (local_24 != 0) {
    (**(code **)(*local_30 + 0x30))(local_30);
  }
  if (local_20 != (int *)0x0) {
    (**(code **)(*local_20 + 8))(local_20);
  }
  if (piVar6 != (int *)0x0) {
    (**(code **)(*piVar6 + 8))(piVar6);
  }
  return local_8;
}

