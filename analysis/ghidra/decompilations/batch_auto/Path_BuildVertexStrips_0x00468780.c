
void Path_BuildVertexStrips(int param_1,void *param_2)

{
  float *pfVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  int local_3c;
  undefined1 local_34 [4];
  void *local_30;
  undefined1 local_24 [4];
  void *local_20;
  undefined4 local_1c;
  undefined4 local_18;
  int local_14;
  int local_10;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  iVar2 = param_1;
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd3e8;
  local_c = ExceptionList;
  if ((*(int *)(param_1 + 4) != 0) &&
     (uVar5 = *(int *)(param_1 + 8) - *(int *)(param_1 + 4) >> 2, 1 < uVar5)) {
    iVar6 = uVar5 - 1;
    ExceptionList = &local_c;
    D3DXSkinMesh_Init(local_34,iVar6 * 4);
    *(undefined4 *)((int)local_30 + local_14 * local_10 * 4 + 8) = 0x3f800000;
    local_10 = local_10 + 1;
    iVar3 = 0;
    local_4 = 0;
    local_3c = 0;
    iVar7 = 0;
    if (3 < iVar6) {
      iVar11 = uVar5 - 2;
      param_1 = 2;
      iVar4 = 8;
      do {
        *(undefined4 *)((int)local_30 + (local_14 * local_10 + iVar3) * 4 + 0xc) = 0x3f800000;
        *(undefined4 *)((int)local_30 + (local_10 + 1) * local_14 * 4 + -4) =
             *(undefined4 *)(iVar4 + -8 + *(int *)(iVar2 + 4));
        iVar7 = local_10 + 1;
        *(undefined4 *)((int)local_30 + (local_14 * iVar7 + iVar3) * 4) = 0x3f800000;
        *(undefined4 *)((int)local_30 + (local_14 * iVar7 + iVar3) * 4 + 4) = 0x3f800000;
        *(undefined4 *)((int)local_30 + (local_14 * iVar7 + iVar3) * 4 + 8) = 0x3f800000;
        pfVar1 = (float *)(iVar4 + -8 + *(int *)(iVar2 + 4));
        *(float *)((int)local_30 + (local_10 + 2) * local_14 * 4 + -4) = pfVar1[1] - *pfVar1;
        iVar8 = local_10 + 2;
        *(undefined4 *)((int)local_30 + (local_14 * iVar8 + iVar3) * 4) = 0x40400000;
        *(undefined4 *)((int)local_30 + (local_14 * iVar8 + iVar3) * 4 + 4) = 0x40000000;
        *(undefined4 *)((int)local_30 + (local_14 * iVar8 + iVar3) * 4 + 8) = 0x3f800000;
        iVar9 = local_10 + 3;
        iVar7 = local_10 + 3;
        if (local_3c < iVar11) {
          *(undefined4 *)((int)local_30 + (local_14 * iVar8 + iVar3) * 4 + 0x18) = 0xbf800000;
          *(undefined4 *)((int)local_30 + (local_14 * iVar9 + iVar3) * 4) = 0x40c00000;
          *(undefined4 *)((int)local_30 + (local_14 * iVar9 + iVar3) * 4 + 4) = 0x40000000;
          *(undefined4 *)((int)local_30 + (local_14 * iVar9 + iVar3) * 4 + 0x14) = 0xc0000000;
          iVar7 = local_10 + 4;
        }
        local_10 = iVar7;
        iVar7 = iVar3 + 4;
        *(undefined4 *)((int)local_30 + (local_14 * local_10 + iVar7) * 4 + 0xc) = 0x3f800000;
        *(undefined4 *)((int)local_30 + (local_10 + 1) * local_14 * 4 + -4) =
             *(undefined4 *)(iVar4 + -4 + *(int *)(iVar2 + 4));
        iVar8 = local_10 + 1;
        *(undefined4 *)((int)local_30 + (local_14 * iVar8 + iVar7) * 4) = 0x3f800000;
        *(undefined4 *)((int)local_30 + (local_14 * iVar8 + iVar7) * 4 + 4) = 0x3f800000;
        *(undefined4 *)((int)local_30 + (local_14 * iVar8 + iVar7) * 4 + 8) = 0x3f800000;
        *(float *)((int)local_30 + (local_10 + 2) * local_14 * 4 + -4) =
             *(float *)(iVar4 + *(int *)(iVar2 + 4)) - *(float *)(iVar4 + -4 + *(int *)(iVar2 + 4));
        iVar8 = local_10 + 2;
        *(undefined4 *)((int)local_30 + (local_14 * iVar8 + iVar7) * 4) = 0x40400000;
        *(undefined4 *)((int)local_30 + (local_14 * iVar8 + iVar7) * 4 + 4) = 0x40000000;
        *(undefined4 *)((int)local_30 + (local_14 * iVar8 + iVar7) * 4 + 8) = 0x3f800000;
        if (param_1 + -1 < iVar11) {
          *(undefined4 *)((int)local_30 + (local_14 * iVar8 + iVar7) * 4 + 0x18) = 0xbf800000;
        }
        iVar8 = local_10 + 3;
        if (param_1 + -1 < iVar11) {
          *(undefined4 *)((int)local_30 + (local_14 * iVar8 + iVar7) * 4) = 0x40c00000;
          *(undefined4 *)((int)local_30 + (local_14 * iVar8 + iVar7) * 4 + 4) = 0x40000000;
          *(undefined4 *)((int)local_30 + (local_14 * iVar8 + iVar7) * 4 + 0x14) = 0xc0000000;
          iVar8 = local_10 + 4;
        }
        local_10 = iVar8;
        iVar8 = iVar3 + 8;
        *(undefined4 *)((int)local_30 + (local_14 * local_10 + iVar8) * 4 + 0xc) = 0x3f800000;
        *(undefined4 *)((int)local_30 + (local_10 + 1) * local_14 * 4 + -4) =
             *(undefined4 *)(iVar4 + *(int *)(iVar2 + 4));
        iVar7 = local_10 + 1;
        *(undefined4 *)((int)local_30 + (local_14 * iVar7 + iVar8) * 4) = 0x3f800000;
        *(undefined4 *)((int)local_30 + (local_14 * iVar7 + iVar8) * 4 + 4) = 0x3f800000;
        *(undefined4 *)((int)local_30 + (local_14 * iVar7 + iVar8) * 4 + 8) = 0x3f800000;
        *(float *)((int)local_30 + (local_10 + 2) * local_14 * 4 + -4) =
             *(float *)(iVar4 + 4 + *(int *)(iVar2 + 4)) - *(float *)(iVar4 + *(int *)(iVar2 + 4));
        iVar9 = local_10 + 2;
        *(undefined4 *)((int)local_30 + (local_14 * iVar9 + iVar8) * 4) = 0x40400000;
        *(undefined4 *)((int)local_30 + (local_14 * iVar9 + iVar8) * 4 + 4) = 0x40000000;
        *(undefined4 *)((int)local_30 + (local_14 * iVar9 + iVar8) * 4 + 8) = 0x3f800000;
        iVar10 = local_10 + 3;
        iVar7 = local_10 + 3;
        if (param_1 < iVar11) {
          *(undefined4 *)((int)local_30 + (local_14 * iVar9 + iVar8) * 4 + 0x18) = 0xbf800000;
          *(undefined4 *)((int)local_30 + (local_14 * iVar10 + iVar8) * 4) = 0x40c00000;
          *(undefined4 *)((int)local_30 + (local_14 * iVar10 + iVar8) * 4 + 4) = 0x40000000;
          *(undefined4 *)((int)local_30 + (local_14 * iVar10 + iVar8) * 4 + 0x14) = 0xc0000000;
          iVar7 = local_10 + 4;
        }
        local_10 = iVar7;
        iVar7 = iVar3 + 0xc;
        *(undefined4 *)((int)local_30 + (local_14 * local_10 + iVar7) * 4 + 0xc) = 0x3f800000;
        *(undefined4 *)((int)local_30 + (local_10 + 1) * local_14 * 4 + -4) =
             *(undefined4 *)(iVar4 + 4 + *(int *)(iVar2 + 4));
        iVar8 = local_10 + 1;
        *(undefined4 *)((int)local_30 + (local_14 * iVar8 + iVar7) * 4) = 0x3f800000;
        *(undefined4 *)((int)local_30 + (local_14 * iVar8 + iVar7) * 4 + 4) = 0x3f800000;
        *(undefined4 *)((int)local_30 + (local_14 * iVar8 + iVar7) * 4 + 8) = 0x3f800000;
        *(float *)((int)local_30 + (local_10 + 2) * local_14 * 4 + -4) =
             *(float *)(iVar4 + 8 + *(int *)(iVar2 + 4)) -
             *(float *)(iVar4 + 4 + *(int *)(iVar2 + 4));
        iVar8 = local_10 + 2;
        *(undefined4 *)((int)local_30 + (local_14 * iVar8 + iVar7) * 4) = 0x40400000;
        *(undefined4 *)((int)local_30 + (local_14 * iVar8 + iVar7) * 4 + 4) = 0x40000000;
        *(undefined4 *)((int)local_30 + (local_14 * iVar8 + iVar7) * 4 + 8) = 0x3f800000;
        if (param_1 + 1 < iVar11) {
          *(undefined4 *)((int)local_30 + (local_14 * iVar8 + iVar7) * 4 + 0x18) = 0xbf800000;
        }
        iVar8 = local_10 + 3;
        if (param_1 + 1 < iVar11) {
          *(undefined4 *)((int)local_30 + (local_14 * iVar8 + iVar7) * 4) = 0x40c00000;
          *(undefined4 *)((int)local_30 + (local_14 * iVar8 + iVar7) * 4 + 4) = 0x40000000;
          *(undefined4 *)((int)local_30 + (local_14 * iVar8 + iVar7) * 4 + 0x14) = 0xc0000000;
          iVar8 = local_10 + 4;
        }
        local_10 = iVar8;
        local_3c = local_3c + 4;
        iVar8 = param_1 + 5;
        iVar3 = iVar3 + 0x10;
        iVar4 = iVar4 + 0x10;
        param_1 = param_1 + 4;
        iVar7 = local_3c;
      } while (iVar8 < iVar6);
    }
    for (; iVar7 < iVar6; iVar7 = iVar7 + 1) {
      *(undefined4 *)((int)local_30 + (local_14 * local_10 + iVar3) * 4 + 0xc) = 0x3f800000;
      *(undefined4 *)((int)local_30 + (local_10 + 1) * local_14 * 4 + -4) =
           *(undefined4 *)(*(int *)(iVar2 + 4) + iVar7 * 4);
      iVar11 = local_10 + 1;
      *(undefined4 *)((int)local_30 + (local_14 * iVar11 + iVar3) * 4) = 0x3f800000;
      *(undefined4 *)((int)local_30 + (local_14 * iVar11 + iVar3) * 4 + 4) = 0x3f800000;
      *(undefined4 *)((int)local_30 + (local_14 * iVar11 + iVar3) * 4 + 8) = 0x3f800000;
      pfVar1 = (float *)(*(int *)(iVar2 + 4) + iVar7 * 4);
      *(float *)((int)local_30 + (local_10 + 2) * local_14 * 4 + -4) = pfVar1[1] - *pfVar1;
      iVar11 = local_10 + 2;
      *(undefined4 *)((int)local_30 + (local_14 * iVar11 + iVar3) * 4) = 0x40400000;
      *(undefined4 *)((int)local_30 + (local_14 * iVar11 + iVar3) * 4 + 4) = 0x40000000;
      *(undefined4 *)((int)local_30 + (local_14 * iVar11 + iVar3) * 4 + 8) = 0x3f800000;
      if (iVar7 < (int)(uVar5 - 2)) {
        *(undefined4 *)((int)local_30 + (local_14 * iVar11 + iVar3) * 4 + 0x18) = 0xbf800000;
      }
      iVar11 = local_10 + 3;
      if (iVar7 < (int)(uVar5 - 2)) {
        *(undefined4 *)((int)local_30 + (local_14 * iVar11 + iVar3) * 4) = 0x40c00000;
        *(undefined4 *)((int)local_30 + (local_14 * iVar11 + iVar3) * 4 + 4) = 0x40000000;
        *(undefined4 *)((int)local_30 + (local_14 * iVar11 + iVar3) * 4 + 0x14) = 0xc0000000;
        iVar11 = local_10 + 4;
      }
      local_10 = iVar11;
      iVar3 = iVar3 + 4;
    }
    Matrix_SolveGaussElim((int)local_34);
    Vector_Assign(param_2,local_24);
    if (local_20 != (void *)0x0) {
      _free(local_20);
    }
    local_20 = (void *)0x0;
    local_1c = 0;
    local_18 = 0;
    if (local_30 != (void *)0x0) {
      _free(local_30);
    }
  }
  ExceptionList = local_c;
  return;
}

