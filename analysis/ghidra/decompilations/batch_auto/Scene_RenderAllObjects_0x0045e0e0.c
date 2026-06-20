
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall Scene_RenderAllObjects(void *this,char param_1,int param_2)

{
  undefined4 uVar1;
  void *pvVar2;
  int iVar3;
  float fVar4;
  float fVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int *piVar9;
  int iVar10;
  
  Graphics_BeginFrame(*(void **)((int)this + 4),*(int *)((int)this + 0x434));
  if (param_1 != '\0') {
    AthenaList_Free((int)this + 0x488);
    AthenaList_Free((int)this + 0x8a0);
    AthenaList_Free((int)this + 0xcb8);
    if (*(char *)(*(int *)((int)this + 0x480) + 0x434) != '\0') {
      Ball_InitRenderState(*(void **)((int)this + 4));
    }
  }
  if (*(char *)((int)this + 0x430) == '\0') {
    if (*(char *)(*(int *)((int)this + 0x480) + 0x434) == '\0') {
      SceneObject_RenderFull(this,0);
      return;
    }
    iVar6 = AthenaList_NextIndex(*(int *)((int)this + 8) + 0x2c);
    iVar7 = *(int *)((int)this + 8);
    *(undefined4 *)(iVar7 + 0x34 + iVar6 * 4) = 0;
    if (*(int *)(iVar7 + 0x30) < 1) {
      iVar10 = 0;
    }
    else {
      iVar10 = **(int **)(iVar7 + 0x438);
      *(undefined4 *)(iVar7 + 0x34 + iVar6 * 4) = 1;
    }
    while (iVar10 != 0) {
      if (*(char *)(iVar10 + 0x863) == '\0') {
        if (*(char *)(iVar10 + 0x862) == '\0') {
          if (*(char *)(iVar10 + 0x85f) == '\0') {
            if (*(char *)(iVar10 + 0x860) == '\0') {
              *(undefined4 *)(iVar10 + 0x83c) = *(undefined4 *)(*(int *)((int)this + 4) + 0x7c4);
              Graphics_ApplyMaterialAndDraw
                        (*(void **)((int)this + 4),
                         (undefined4 *)
                         (*(int *)(iVar10 + 4) * 0x50 + *(int *)(*(int *)((int)this + 8) + 0x28)));
              iVar7 = *(int *)(*(int *)((int)this + 0x47c) + 0x480);
              if (*(char *)(iVar7 + 0x10c4) == '\0') {
                iVar8 = AthenaList_NextIndex(iVar10 + 0x424);
                iVar7 = AthenaList_SetIndex((void *)(iVar10 + 0x424),iVar8);
                while (iVar7 != 0) {
                  piVar9 = *(int **)(*(int *)((int)this + 4) + 0x154);
                  (**(code **)(*piVar9 + 0x118))
                            (piVar9,5,*(undefined4 *)(iVar7 + 0xc),*(undefined4 *)(iVar7 + 4));
                  piVar9 = (int *)(*(int *)((int)this + 4) + 0x7cc);
                  *piVar9 = *piVar9 + 1;
                  iVar3 = *(int *)(iVar10 + 0x42c + iVar8 * 4);
                  if (*(int *)(iVar10 + 0x428) <= iVar3) break;
                  iVar7 = *(int *)(*(int *)(iVar10 + 0x830) + iVar3 * 4);
                  *(int *)(iVar10 + 0x42c + iVar8 * 4) = iVar3 + 1;
                }
              }
              else if (*(float *)(*(int *)((int)this + 4) + 0x1c) != _DAT_004cf368) {
                piVar9 = *(int **)(iVar7 + 0x44c);
                (**(code **)(*piVar9 + 0xc))(piVar9,*(undefined4 *)(iVar10 + 0x858));
                piVar9 = (int *)(*(int *)((int)this + 4) + 0x7cc);
                *piVar9 = *piVar9 + 1;
              }
            }
            else {
              AthenaList_Append((void *)(*(int *)((int)this + 0x47c) + 0x8a0),iVar10);
              *(int *)(iVar10 + 8) =
                   *(int *)(iVar10 + 4) * 0x50 + *(int *)(*(int *)((int)this + 8) + 0x28);
            }
          }
          else {
            AthenaList_Append((void *)(*(int *)((int)this + 0x47c) + 0x488),iVar10);
            *(int *)(iVar10 + 8) =
                 *(int *)(iVar10 + 4) * 0x50 + *(int *)(*(int *)((int)this + 8) + 0x28);
          }
        }
        else {
          AthenaList_Append((void *)(*(int *)((int)this + 0x47c) + 0xcb8),iVar10);
          *(int *)(iVar10 + 8) =
               *(int *)(iVar10 + 4) * 0x50 + *(int *)(*(int *)((int)this + 8) + 0x28);
        }
      }
      iVar7 = *(int *)((int)this + 8);
      iVar8 = *(int *)(iVar7 + 0x34 + iVar6 * 4);
      if (*(int *)(iVar7 + 0x30) <= iVar8) break;
      iVar10 = *(int *)(*(int *)(iVar7 + 0x438) + iVar8 * 4);
      *(int *)(iVar7 + 0x34 + iVar6 * 4) = iVar8 + 1;
    }
  }
  else {
    *(undefined4 *)((int)this + 0x20) = 0;
    if (*(int *)((int)this + 0x1c) < 1) {
      piVar9 = (int *)0x0;
    }
    else {
      piVar9 = (int *)**(undefined4 **)((int)this + 0x424);
      *(undefined4 *)((int)this + 0x20) = 1;
    }
    while (piVar9 != (int *)0x0) {
      if (param_2 == 2) {
        iVar7 = 2;
LAB_0045e386:
        (**(code **)(*piVar9 + 0x48))(0,iVar7);
      }
      else {
        iVar7 = (**(code **)(*piVar9 + 0x24))();
        if (iVar7 != 0) goto LAB_0045e386;
      }
      iVar7 = *(int *)((int)this + 0x20);
      if (*(int *)((int)this + 0x1c) <= iVar7) break;
      piVar9 = *(int **)(*(int *)((int)this + 0x424) + iVar7 * 4);
      *(int *)((int)this + 0x20) = iVar7 + 1;
    }
  }
  if (param_1 != '\0') {
    iVar7 = AthenaList_GetSize((int)this + 0x8a0);
    if (0 < iVar7) {
      iVar7 = *(int *)((int)this + 4);
      if (*(char *)(iVar7 + 0x70c) != '\0') {
        (**(code **)(**(int **)(iVar7 + 0x154) + 200))(*(int **)(iVar7 + 0x154),0xe,0);
        *(undefined1 *)(iVar7 + 0x70c) = 0;
        *(int *)(iVar7 + 0x7c8) = *(int *)(iVar7 + 0x7c8) + 1;
      }
      iVar7 = AthenaList_NextIndex((int)this + 0x8a0);
      *(undefined4 *)((int)this + iVar7 * 4 + 0x8a8) = 0;
      if (*(int *)((int)this + 0x8a4) < 1) {
        iVar6 = 0;
      }
      else {
        iVar6 = **(int **)((int)this + 0xcac);
        *(undefined4 *)((int)this + iVar7 * 4 + 0x8a8) = 1;
      }
      while (iVar6 != 0) {
        *(undefined4 *)(iVar6 + 0x83c) = *(undefined4 *)(*(int *)((int)this + 4) + 0x7c4);
        Graphics_ApplyMaterialAndDraw(*(void **)((int)this + 4),*(undefined4 **)(iVar6 + 8));
        if (*(char *)(*(int *)(*(int *)((int)this + 0x47c) + 0x480) + 0x10c4) == '\0') {
          iVar10 = AthenaList_NextIndex(iVar6 + 0x424);
          *(undefined4 *)(iVar6 + 0x42c + iVar10 * 4) = 0;
          if (*(int *)(iVar6 + 0x428) < 1) {
            iVar8 = 0;
          }
          else {
            iVar8 = **(int **)(iVar6 + 0x830);
            *(undefined4 *)(iVar6 + 0x42c + iVar10 * 4) = 1;
          }
          while (iVar8 != 0) {
            piVar9 = *(int **)(*(int *)((int)this + 4) + 0x154);
            (**(code **)(*piVar9 + 0x118))
                      (piVar9,5,*(undefined4 *)(iVar8 + 0xc),*(undefined4 *)(iVar8 + 4));
            piVar9 = (int *)(*(int *)((int)this + 4) + 0x7cc);
            *piVar9 = *piVar9 + 1;
            iVar3 = *(int *)(iVar6 + 0x42c + iVar10 * 4);
            if (*(int *)(iVar6 + 0x428) <= iVar3) break;
            iVar8 = *(int *)(*(int *)(iVar6 + 0x830) + iVar3 * 4);
            *(int *)(iVar6 + 0x42c + iVar10 * 4) = iVar3 + 1;
          }
        }
        else {
          uVar1 = *(undefined4 *)(iVar6 + 0x850);
          iVar10 = *(int *)((int)this + 4);
          piVar9 = *(int **)(iVar10 + 0x154);
          (**(code **)(*piVar9 + 0x14c))(piVar9,0,uVar1,0x20);
          *(undefined4 *)(iVar10 + 8) = uVar1;
          piVar9 = *(int **)(*(int *)(*(int *)((int)this + 0x47c) + 0x480) + 0x44c);
          (**(code **)(*piVar9 + 0xc))(piVar9,*(undefined4 *)(iVar6 + 0x858));
          piVar9 = (int *)(*(int *)((int)this + 4) + 0x7cc);
          *piVar9 = *piVar9 + 1;
        }
        iVar10 = *(int *)((int)this + iVar7 * 4 + 0x8a8);
        if (*(int *)((int)this + 0x8a4) <= iVar10) break;
        iVar6 = *(int *)(*(int *)((int)this + 0xcac) + iVar10 * 4);
        *(int *)((int)this + iVar7 * 4 + 0x8a8) = iVar10 + 1;
      }
      iVar7 = *(int *)((int)this + 4);
      if (*(char *)(iVar7 + 0x70c) != '\x01') {
        (**(code **)(**(int **)(iVar7 + 0x154) + 200))(*(int **)(iVar7 + 0x154),0xe,1);
        *(undefined1 *)(iVar7 + 0x70c) = 1;
        *(int *)(iVar7 + 0x7c8) = *(int *)(iVar7 + 0x7c8) + 1;
      }
    }
    iVar7 = AthenaList_GetSize((int)this + 0x488);
    if (0 < iVar7) {
      pvVar2 = *(void **)((int)this + 4);
      fVar4 = *(float *)((int)pvVar2 + 0x790) + (float)_DAT_004cf308;
      *(float *)((int)pvVar2 + 0x790) = fVar4;
      fVar5 = *(float *)((int)pvVar2 + 0x794) + (float)_DAT_004cf308;
      *(float *)((int)pvVar2 + 0x794) = fVar5;
      Graphics_SetProjection(pvVar2,fVar4,fVar5);
      iVar7 = *(int *)((int)this + 4);
      if (*(char *)(iVar7 + 0x70c) != '\0') {
        (**(code **)(**(int **)(iVar7 + 0x154) + 200))(*(int **)(iVar7 + 0x154),0xe,0);
        *(undefined1 *)(iVar7 + 0x70c) = 0;
        *(int *)(iVar7 + 0x7c8) = *(int *)(iVar7 + 0x7c8) + 1;
      }
      piVar9 = *(int **)(*(int *)((int)this + 4) + 0x154);
      (**(code **)(*piVar9 + 200))(piVar9,0x37,1);
      iVar6 = AthenaList_NextIndex((int)this + 0x488);
      iVar7 = 0;
      *(undefined4 *)((int)this + iVar6 * 4 + 0x490) = 0;
      if (0 < *(int *)((int)this + 0x48c)) {
        iVar7 = **(int **)((int)this + 0x894);
        *(undefined4 *)((int)this + iVar6 * 4 + 0x490) = 1;
      }
      while (iVar7 != 0) {
        if ((*(char *)(iVar7 + 0x861) != '\0') &&
           (iVar10 = *(int *)((int)this + 4), *(char *)(iVar10 + 0x70c) != '\x01')) {
          (**(code **)(**(int **)(iVar10 + 0x154) + 200))(*(int **)(iVar10 + 0x154),0xe,1);
          *(undefined1 *)(iVar10 + 0x70c) = 1;
          *(int *)(iVar10 + 0x7c8) = *(int *)(iVar10 + 0x7c8) + 1;
        }
        *(undefined4 *)(iVar7 + 0x83c) = *(undefined4 *)(*(int *)((int)this + 4) + 0x7c4);
        Graphics_ApplyMaterialAndDraw(*(void **)((int)this + 4),*(undefined4 **)(iVar7 + 8));
        if (*(char *)(*(int *)(*(int *)((int)this + 0x47c) + 0x480) + 0x10c4) == '\0') {
          iVar10 = AthenaList_NextIndex(iVar7 + 0x424);
          *(undefined4 *)(iVar7 + 0x42c + iVar10 * 4) = 0;
          if (*(int *)(iVar7 + 0x428) < 1) {
            iVar8 = 0;
          }
          else {
            iVar8 = **(int **)(iVar7 + 0x830);
            *(undefined4 *)(iVar7 + 0x42c + iVar10 * 4) = 1;
          }
          while (iVar8 != 0) {
            piVar9 = *(int **)(*(int *)((int)this + 4) + 0x154);
            (**(code **)(*piVar9 + 0x118))
                      (piVar9,5,*(undefined4 *)(iVar8 + 0xc),*(undefined4 *)(iVar8 + 4));
            piVar9 = (int *)(*(int *)((int)this + 4) + 0x7cc);
            *piVar9 = *piVar9 + 1;
            iVar3 = *(int *)(iVar7 + 0x42c + iVar10 * 4);
            if (*(int *)(iVar7 + 0x428) <= iVar3) break;
            iVar8 = *(int *)(*(int *)(iVar7 + 0x830) + iVar3 * 4);
            *(int *)(iVar7 + 0x42c + iVar10 * 4) = iVar3 + 1;
          }
        }
        else {
          uVar1 = *(undefined4 *)(iVar7 + 0x850);
          iVar10 = *(int *)((int)this + 4);
          piVar9 = *(int **)(iVar10 + 0x154);
          (**(code **)(*piVar9 + 0x14c))(piVar9,0,uVar1,0x20);
          *(undefined4 *)(iVar10 + 8) = uVar1;
          piVar9 = *(int **)(*(int *)(*(int *)((int)this + 0x47c) + 0x480) + 0x44c);
          (**(code **)(*piVar9 + 0xc))(piVar9,*(undefined4 *)(iVar7 + 0x858));
          piVar9 = (int *)(*(int *)((int)this + 4) + 0x7cc);
          *piVar9 = *piVar9 + 1;
        }
        if ((*(char *)(iVar7 + 0x861) != '\0') &&
           (iVar7 = *(int *)((int)this + 4), *(char *)(iVar7 + 0x70c) != '\0')) {
          (**(code **)(**(int **)(iVar7 + 0x154) + 200))(*(int **)(iVar7 + 0x154),0xe,0);
          *(undefined1 *)(iVar7 + 0x70c) = 0;
          *(int *)(iVar7 + 0x7c8) = *(int *)(iVar7 + 0x7c8) + 1;
        }
        iVar10 = *(int *)((int)this + iVar6 * 4 + 0x490);
        if (*(int *)((int)this + 0x48c) <= iVar10) break;
        iVar7 = *(int *)(*(int *)((int)this + 0x894) + iVar10 * 4);
        *(int *)((int)this + iVar6 * 4 + 0x490) = iVar10 + 1;
      }
      piVar9 = *(int **)(*(int *)((int)this + 4) + 0x154);
      (**(code **)(*piVar9 + 200))(piVar9,0x37,3);
      iVar7 = *(int *)((int)this + 4);
      if (*(char *)(iVar7 + 0x70c) != '\x01') {
        (**(code **)(**(int **)(iVar7 + 0x154) + 200))(*(int **)(iVar7 + 0x154),0xe,1);
        *(undefined1 *)(iVar7 + 0x70c) = 1;
        *(int *)(iVar7 + 0x7c8) = *(int *)(iVar7 + 0x7c8) + 1;
      }
      pvVar2 = *(void **)((int)this + 4);
      fVar4 = *(float *)((int)pvVar2 + 0x790) - (float)_DAT_004cf308;
      *(float *)((int)pvVar2 + 0x790) = fVar4;
      fVar5 = *(float *)((int)pvVar2 + 0x794) - (float)_DAT_004cf308;
      *(float *)((int)pvVar2 + 0x794) = fVar5;
      Graphics_SetProjection(pvVar2,fVar4,fVar5);
    }
  }
  iVar6 = AthenaList_NextIndex(*(int *)((int)this + 0x480) + 0x1c);
  iVar7 = *(int *)((int)this + 0x480);
  *(undefined4 *)(iVar7 + 0x24 + iVar6 * 4) = 0;
  if (*(int *)(iVar7 + 0x20) < 1) {
    piVar9 = (int *)0x0;
  }
  else {
    piVar9 = (int *)**(undefined4 **)(iVar7 + 0x428);
    *(undefined4 *)(iVar7 + 0x24 + iVar6 * 4) = 1;
  }
  while( true ) {
    if (piVar9 == (int *)0x0) {
      return;
    }
    (**(code **)(*piVar9 + 0x48))(1,1);
    iVar7 = *(int *)((int)this + 0x480);
    iVar10 = *(int *)(iVar7 + 0x24 + iVar6 * 4);
    if (*(int *)(iVar7 + 0x20) <= iVar10) break;
    piVar9 = *(int **)(*(int *)(iVar7 + 0x428) + iVar10 * 4);
    *(int *)(iVar7 + 0x24 + iVar6 * 4) = iVar10 + 1;
  }
  return;
}

