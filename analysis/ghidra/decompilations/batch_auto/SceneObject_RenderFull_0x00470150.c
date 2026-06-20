
void __thiscall SceneObject_RenderFull(void *this,int param_1)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  undefined4 *puVar7;
  
  iVar6 = 0;
  if (param_1 != 0) {
    *(undefined4 *)(param_1 + 0x41c) = 0;
  }
  iVar3 = (**(code **)(*(int *)this + 0x24))();
  if (iVar3 != 0) {
    Ball_InitRenderState(*(void **)((int)this + 4));
    iVar3 = *(int *)((int)this + 8);
    if (*(char *)(iVar3 + 0x459) == '\x01') {
      iVar4 = AthenaList_NextIndex(iVar3 + 0x2c);
      iVar3 = *(int *)((int)this + 8);
      *(undefined4 *)(iVar3 + 0x34 + iVar4 * 4) = 0;
      if (0 < *(int *)(iVar3 + 0x30)) {
        iVar6 = **(int **)(iVar3 + 0x438);
        *(undefined4 *)(iVar3 + 0x34 + iVar4 * 4) = 1;
      }
      if (iVar6 != 0) {
        do {
          if (param_1 == 0) {
            puVar7 = (undefined4 *)
                     (*(int *)(iVar6 + 4) * 0x50 + *(int *)(*(int *)((int)this + 8) + 0x28));
          }
          else {
            iVar3 = *(int *)(param_1 + 0x41c);
            *(int *)(param_1 + 0x41c) = iVar3 + 1;
            if (iVar3 < 0) {
              puVar7 = (undefined4 *)0x0;
            }
            else if (iVar3 < *(int *)(param_1 + 8)) {
              puVar7 = *(undefined4 **)(*(int *)(param_1 + 0x410) + iVar3 * 4);
            }
            else {
              puVar7 = (undefined4 *)0x0;
            }
          }
          Graphics_ApplyMaterialAndDraw(*(void **)((int)this + 4),puVar7);
          if (*(char *)((int)this + 0xe) == '\0') {
            iVar3 = AthenaList_NextIndex(iVar6 + 0x424);
            *(undefined4 *)(iVar6 + 0x42c + iVar3 * 4) = 0;
            if (*(int *)(iVar6 + 0x428) < 1) {
              iVar5 = 0;
            }
            else {
              iVar5 = **(int **)(iVar6 + 0x830);
              *(undefined4 *)(iVar6 + 0x42c + iVar3 * 4) = 1;
            }
            while (iVar5 != 0) {
              piVar1 = *(int **)(*(int *)((int)this + 4) + 0x154);
              (**(code **)(*piVar1 + 0x118))
                        (piVar1,5,*(undefined4 *)(iVar5 + 8),*(undefined4 *)(iVar5 + 4));
              iVar2 = *(int *)(iVar6 + 0x42c + iVar3 * 4);
              if (*(int *)(iVar6 + 0x428) <= iVar2) break;
              iVar5 = *(int *)(*(int *)(iVar6 + 0x830) + iVar2 * 4);
              *(int *)(iVar6 + 0x42c + iVar3 * 4) = iVar2 + 1;
            }
          }
          else {
            piVar1 = *(int **)(*(int *)((int)this + 8) + 0x454);
            (**(code **)(*piVar1 + 0xc))(piVar1,*(undefined4 *)(iVar6 + 0x858));
          }
          iVar3 = *(int *)((int)this + 8);
          iVar5 = *(int *)(iVar3 + 0x34 + iVar4 * 4);
          if (*(int *)(iVar3 + 0x30) <= iVar5) {
            return;
          }
          iVar6 = *(int *)(*(int *)(iVar3 + 0x438) + iVar5 * 4);
          *(int *)(iVar3 + 0x34 + iVar4 * 4) = iVar5 + 1;
          if (iVar6 == 0) {
            return;
          }
        } while( true );
      }
    }
    else {
      *(undefined4 *)(iVar3 + 0x34) = 0;
      if (0 < *(int *)(iVar3 + 0x30)) {
        iVar6 = **(int **)(iVar3 + 0x438);
        *(undefined4 *)(iVar3 + 0x34) = 1;
      }
      while (iVar6 != 0) {
        piVar1 = *(int **)(*(int *)((int)this + 4) + 0x154);
        (**(code **)(*piVar1 + 0xa8))
                  (piVar1,*(int *)(iVar6 + 4) * 0x50 + 4 + *(int *)(*(int *)((int)this + 8) + 0x28))
        ;
        iVar3 = *(int *)(*(int *)(iVar6 + 4) * 0x50 + 0x48 +
                        *(int *)(*(int *)((int)this + 8) + 0x28));
        if (iVar3 == 0) {
          Gfx_ResetLighting((int)*(void **)((int)this + 4));
        }
        else {
          Gfx_ApplyLightingState(*(void **)((int)this + 4),iVar3);
        }
        if (*(char *)(iVar6 + 0x85c) == '\0') {
          *(undefined4 *)(iVar6 + 0x14) = 0;
          if (*(int *)(iVar6 + 0x10) < 1) {
            iVar3 = 0;
          }
          else {
            iVar3 = **(int **)(iVar6 + 0x418);
            *(undefined4 *)(iVar6 + 0x14) = 1;
          }
          while (iVar3 != 0) {
            piVar1 = *(int **)(*(int *)((int)this + 4) + 0x154);
            (**(code **)(*piVar1 + 0x120))(piVar1,5,1,iVar3,0x20);
            iVar4 = *(int *)(iVar6 + 0x14);
            if (*(int *)(iVar6 + 0x10) <= iVar4) break;
            iVar3 = *(int *)(*(int *)(iVar6 + 0x418) + iVar4 * 4);
            *(int *)(iVar6 + 0x14) = iVar4 + 1;
          }
        }
        else {
          *(undefined4 *)(iVar6 + 0x42c) = 0;
          if (*(int *)(iVar6 + 0x428) < 1) {
            iVar3 = 0;
          }
          else {
            iVar3 = **(int **)(iVar6 + 0x830);
            *(undefined4 *)(iVar6 + 0x42c) = 1;
          }
          while (iVar3 != 0) {
            piVar1 = *(int **)(*(int *)((int)this + 4) + 0x154);
            (**(code **)(*piVar1 + 0x120))
                      (piVar1,5,*(undefined4 *)(iVar3 + 4),*(undefined4 *)(iVar3 + 0x10),0x20);
            iVar4 = *(int *)(iVar6 + 0x42c);
            if (*(int *)(iVar6 + 0x428) <= iVar4) break;
            iVar3 = *(int *)(*(int *)(iVar6 + 0x830) + iVar4 * 4);
            *(int *)(iVar6 + 0x42c) = iVar4 + 1;
          }
        }
        iVar3 = *(int *)((int)this + 8);
        iVar4 = *(int *)(iVar3 + 0x34);
        if (*(int *)(iVar3 + 0x30) <= iVar4) {
          return;
        }
        iVar6 = *(int *)(*(int *)(iVar3 + 0x438) + iVar4 * 4);
        *(int *)(iVar3 + 0x34) = iVar4 + 1;
      }
    }
  }
  return;
}

