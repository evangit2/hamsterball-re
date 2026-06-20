
void __thiscall Scene_RenderFrame(void *this,undefined4 *param_1)

{
  void *pvVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int *piVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  bool bVar10;
  int local_28;
  int local_18 [3];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004ccf0b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *(undefined1 *)(*(int *)((int)this + 0x480) + 0x10c4) = 1;
  if (*(void **)((int)this + 0x47c) == this) {
    g_renderIndex = 0;
    pvVar1 = operator_new(0x30);
    local_4 = 0;
    if (pvVar1 == (void *)0x0) {
      param_1 = (undefined4 *)0x0;
    }
    else {
      iVar2 = *(int *)((int)this + 0x480);
      if (*(uint *)(iVar2 + 0x18) < 0x10) {
        piVar5 = (int *)(iVar2 + 4);
      }
      else {
        piVar5 = *(int **)(iVar2 + 4);
      }
      param_1 = MeshWorld_ctor(pvVar1,*(int *)((int)this + 4),*(int *)(iVar2 + 0x438) << 2,piVar5);
    }
    local_4 = 0xffffffff;
    pvVar1 = operator_new(*(int *)(*(int *)((int)this + 0x480) + 0x43c) * 0x60);
    *(void **)(*(int *)((int)this + 0x480) + 0x10c8) = pvVar1;
    *(undefined4 *)(*(int *)((int)this + 0x480) + 0x10cc) = 0;
  }
  if (*(char *)((int)this + 0x430) != '\0') {
    iVar2 = AthenaList_NextIndex((int)this + 0x18);
    *(undefined4 *)((int)this + iVar2 * 4 + 0x20) = 0;
    if (*(int *)((int)this + 0x1c) < 1) {
      piVar5 = (int *)0x0;
    }
    else {
      piVar5 = (int *)**(undefined4 **)((int)this + 0x424);
      *(undefined4 *)((int)this + iVar2 * 4 + 0x20) = 1;
    }
    while (piVar5 != (int *)0x0) {
      (**(code **)(*piVar5 + 0x3c))(param_1);
      iVar3 = *(int *)((int)this + iVar2 * 4 + 0x20);
      if (*(int *)((int)this + 0x1c) <= iVar3) break;
      piVar5 = *(int **)(*(int *)((int)this + 0x424) + iVar3 * 4);
      *(int *)((int)this + iVar2 * 4 + 0x20) = iVar3 + 1;
    }
  }
  iVar3 = AthenaList_NextIndex(*(int *)((int)this + 8) + 0x2c);
  iVar2 = *(int *)((int)this + 8);
  *(undefined4 *)(iVar2 + 0x34 + iVar3 * 4) = 0;
  if (*(int *)(iVar2 + 0x30) < 1) {
    iVar7 = 0;
  }
  else {
    iVar7 = **(int **)(iVar2 + 0x438);
    *(undefined4 *)(iVar2 + 0x34 + iVar3 * 4) = 1;
  }
  while (iVar7 != 0) {
    SpriteAnim_SetRange(param_1,g_renderIndex);
    g_renderIndex = g_renderIndex + 1;
    *(undefined4 *)(iVar7 + 0x858) = param_1[10];
    iVar4 = AthenaList_NextIndex(iVar7 + 0x424);
    iVar2 = 0;
    *(undefined4 *)(iVar7 + 0x42c + iVar4 * 4) = 0;
    if (0 < *(int *)(iVar7 + 0x428)) {
      iVar2 = **(int **)(iVar7 + 0x830);
      *(undefined4 *)(iVar7 + 0x42c + iVar4 * 4) = 1;
    }
    while (iVar2 != 0) {
      if (*(char *)(*(int *)((int)this + 0x480) + 0x434) == '\0') {
        iVar6 = *(int *)(iVar2 + 8);
        iVar9 = *(int *)(*(int *)((int)this + 8) + 0x448);
      }
      else {
        iVar6 = *(int *)(iVar2 + 0xc);
        iVar9 = *(int *)(*(int *)(*(int *)((int)this + 0x47c) + 0x480) + 0x440);
      }
      iVar9 = iVar6 * 0x20 + iVar9;
      bVar10 = false;
      local_28 = 0;
      if (0 < *(int *)(iVar2 + 4)) {
        iVar6 = iVar9 + 0x40;
        do {
          local_18[0] = iVar9;
          if (bVar10) {
            local_18[2] = iVar6 + -0x20;
            local_18[1] = iVar6;
          }
          else {
            local_18[1] = iVar6 + -0x20;
            local_18[2] = iVar6;
          }
          bVar10 = bVar10 == false;
          *(int *)(iVar7 + 0x840) = *(int *)(iVar7 + 0x840) + 1;
          iVar8 = 0;
          do {
            Mesh_AddVertex(param_1,(float *)local_18[iVar8]);
            iVar8 = iVar8 + 1;
          } while (iVar8 < 3);
          iVar9 = iVar9 + 0x20;
          iVar6 = iVar6 + 0x20;
          local_28 = local_28 + 1;
        } while (local_28 < *(int *)(iVar2 + 4));
      }
      iVar6 = *(int *)(iVar7 + 0x42c + iVar4 * 4);
      if (*(int *)(iVar7 + 0x428) <= iVar6) break;
      iVar2 = *(int *)(*(int *)(iVar7 + 0x830) + iVar6 * 4);
      *(int *)(iVar7 + 0x42c + iVar4 * 4) = iVar6 + 1;
    }
    iVar2 = *(int *)((int)this + 8);
    iVar4 = *(int *)(iVar2 + 0x34 + iVar3 * 4);
    if (*(int *)(iVar2 + 0x30) <= iVar4) break;
    iVar7 = *(int *)(*(int *)(iVar2 + 0x438) + iVar4 * 4);
    *(int *)(iVar2 + 0x34 + iVar3 * 4) = iVar4 + 1;
  }
  if (*(void **)((int)this + 0x47c) == this) {
    piVar5 = Font_RenderToTextureComplex((int)param_1);
    *(int **)(*(int *)((int)this + 0x480) + 0x44c) = piVar5;
    if (param_1 != (undefined4 *)0x0) {
      Mesh_SaveAndFree(param_1);
      _free(param_1);
    }
  }
  ExceptionList = local_c;
  return;
}

