
void __fastcall SceneObject_BuildStrips(int param_1)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  bool bVar10;
  int local_5c;
  int local_48 [13];
  int local_14;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd9a8;
  local_c = ExceptionList;
  iVar5 = *(int *)(param_1 + 8);
  if (*(uint *)(iVar5 + 0x20) < 0x10) {
    piVar3 = (int *)(iVar5 + 0xc);
  }
  else {
    piVar3 = *(int **)(iVar5 + 0xc);
  }
  ExceptionList = &local_c;
  MeshWorld_ctor(local_48 + 3,*(int *)(param_1 + 4),*(int *)(iVar5 + 0x444) << 2,piVar3);
  local_4 = 0;
  iVar1 = AthenaList_NextIndex(*(int *)(param_1 + 8) + 0x2c);
  iVar5 = *(int *)(param_1 + 8);
  *(undefined4 *)(iVar5 + 0x34 + iVar1 * 4) = 0;
  iVar2 = local_14;
  iVar6 = 0;
  if (*(int *)(iVar5 + 0x30) < 1) {
    iVar8 = 0;
  }
  else {
    iVar8 = **(int **)(iVar5 + 0x438);
    *(undefined4 *)(iVar5 + 0x34 + iVar1 * 4) = 1;
  }
  while (local_14 = iVar6, iVar8 != 0) {
    iVar5 = 0;
    *(int *)(iVar8 + 0x858) = local_14;
    iVar6 = local_14 + 1;
    iVar2 = AthenaList_NextIndex(iVar8 + 0x424);
    *(undefined4 *)(iVar8 + 0x42c + iVar2 * 4) = 0;
    if (0 < *(int *)(iVar8 + 0x428)) {
      iVar5 = **(int **)(iVar8 + 0x830);
      *(undefined4 *)(iVar8 + 0x42c + iVar2 * 4) = 1;
    }
    while (iVar5 != 0) {
      iVar7 = *(int *)(iVar5 + 8) * 0x20 + *(int *)(*(int *)(param_1 + 8) + 0x448);
      bVar10 = false;
      local_5c = 0;
      if (0 < *(int *)(iVar5 + 4)) {
        iVar4 = iVar7 + 0x40;
        do {
          local_48[0] = iVar7;
          if (bVar10) {
            local_48[2] = iVar4 + -0x20;
            local_48[1] = iVar4;
          }
          else {
            local_48[1] = iVar4 + -0x20;
            local_48[2] = iVar4;
          }
          bVar10 = bVar10 == false;
          *(int *)(iVar8 + 0x840) = *(int *)(iVar8 + 0x840) + 1;
          iVar9 = 0;
          do {
            Mesh_AddVertex(local_48 + 3,(float *)local_48[iVar9]);
            iVar9 = iVar9 + 1;
          } while (iVar9 < 3);
          iVar7 = iVar7 + 0x20;
          iVar4 = iVar4 + 0x20;
          local_5c = local_5c + 1;
        } while (local_5c < *(int *)(iVar5 + 4));
      }
      iVar7 = *(int *)(iVar8 + 0x42c + iVar2 * 4);
      if (*(int *)(iVar8 + 0x428) <= iVar7) break;
      iVar5 = *(int *)(*(int *)(iVar8 + 0x830) + iVar7 * 4);
      *(int *)(iVar8 + 0x42c + iVar2 * 4) = iVar7 + 1;
    }
    iVar5 = *(int *)(param_1 + 8);
    iVar7 = *(int *)(iVar5 + 0x34 + iVar1 * 4);
    iVar2 = local_14;
    if (*(int *)(iVar5 + 0x30) <= iVar7) break;
    iVar8 = *(int *)(*(int *)(iVar5 + 0x438) + iVar7 * 4);
    *(int *)(iVar5 + 0x34 + iVar1 * 4) = iVar7 + 1;
  }
  local_14 = iVar2;
  piVar3 = Font_RenderToTextureComplex((int)(local_48 + 3));
  *(int **)(*(int *)(param_1 + 8) + 0x454) = piVar3;
  *(undefined1 *)(param_1 + 0xe) = 1;
  local_4 = 0xffffffff;
  Mesh_SaveAndFree(local_48 + 3);
  ExceptionList = local_c;
  return;
}

