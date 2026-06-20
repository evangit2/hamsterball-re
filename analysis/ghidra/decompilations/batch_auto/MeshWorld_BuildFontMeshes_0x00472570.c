
void __fastcall MeshWorld_BuildFontMeshes(int param_1)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  bool bVar9;
  int local_54;
  int local_48 [11];
  undefined4 local_1c;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd9a8;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *(undefined1 *)(param_1 + 0xe) = 1;
  iVar1 = AthenaList_NextIndex(*(int *)(param_1 + 8) + 0x2c);
  iVar5 = *(int *)(param_1 + 8);
  *(undefined4 *)(iVar5 + 0x34 + iVar1 * 4) = 0;
  if (*(int *)(iVar5 + 0x30) < 1) {
    iVar7 = 0;
  }
  else {
    iVar7 = **(int **)(iVar5 + 0x438);
    *(undefined4 *)(iVar5 + 0x34 + iVar1 * 4) = 1;
  }
  do {
    if (iVar7 == 0) {
      ExceptionList = local_c;
      return;
    }
    iVar5 = 0;
    MeshWorld_ctor(local_48 + 3,*(int *)(param_1 + 4),*(int *)(*(int *)(param_1 + 8) + 0x444) << 2,
                   (int *)0x0);
    local_4 = 0;
    *(undefined4 *)(iVar7 + 0x844) = local_1c;
    *(undefined4 *)(iVar7 + 0x848) = local_1c;
    *(undefined4 *)(iVar7 + 0x840) = 0;
    iVar2 = AthenaList_NextIndex(iVar7 + 0x424);
    *(undefined4 *)(iVar7 + 0x42c + iVar2 * 4) = 0;
    if (0 < *(int *)(iVar7 + 0x428)) {
      iVar5 = **(int **)(iVar7 + 0x830);
      *(undefined4 *)(iVar7 + 0x42c + iVar2 * 4) = 1;
    }
    while (iVar5 != 0) {
      iVar6 = *(int *)(iVar5 + 8) * 0x20 + *(int *)(*(int *)(param_1 + 8) + 0x448);
      bVar9 = false;
      local_54 = 0;
      if (0 < *(int *)(iVar5 + 4)) {
        iVar4 = iVar6 + 0x40;
        do {
          local_48[0] = iVar6;
          if (bVar9) {
            local_48[2] = iVar4 + -0x20;
            local_48[1] = iVar4;
          }
          else {
            local_48[1] = iVar4 + -0x20;
            local_48[2] = iVar4;
          }
          bVar9 = bVar9 == false;
          *(int *)(iVar7 + 0x840) = *(int *)(iVar7 + 0x840) + 1;
          iVar8 = 0;
          do {
            Mesh_AddVertex(local_48 + 3,(float *)local_48[iVar8]);
            iVar8 = iVar8 + 1;
          } while (iVar8 < 3);
          iVar6 = iVar6 + 0x20;
          iVar4 = iVar4 + 0x20;
          local_54 = local_54 + 1;
        } while (local_54 < *(int *)(iVar5 + 4));
      }
      piVar3 = Font_RenderToTextureComplex((int)(local_48 + 3));
      *(int **)(iVar7 + 0x854) = piVar3;
      iVar6 = *(int *)(iVar7 + 0x42c + iVar2 * 4);
      if (*(int *)(iVar7 + 0x428) <= iVar6) break;
      iVar5 = *(int *)(*(int *)(iVar7 + 0x830) + iVar6 * 4);
      *(int *)(iVar7 + 0x42c + iVar2 * 4) = iVar6 + 1;
    }
    local_4 = 0xffffffff;
    Mesh_SaveAndFree(local_48 + 3);
    iVar5 = *(int *)(param_1 + 8);
    iVar2 = *(int *)(iVar5 + 0x34 + iVar1 * 4);
    if (*(int *)(iVar5 + 0x30) <= iVar2) {
      ExceptionList = local_c;
      return;
    }
    iVar7 = *(int *)(*(int *)(iVar5 + 0x438) + iVar2 * 4);
    *(int *)(iVar5 + 0x34 + iVar1 * 4) = iVar2 + 1;
  } while( true );
}

