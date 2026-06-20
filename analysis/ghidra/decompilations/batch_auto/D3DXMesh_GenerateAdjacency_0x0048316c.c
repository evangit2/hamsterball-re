
undefined4 D3DXMesh_GenerateAdjacency(void)

{
  void *this;
  undefined4 uVar1;
  int iVar2;
  void *pvVar3;
  void *pvVar4;
  undefined4 *puVar5;
  undefined4 *puVar6;
  int iVar7;
  int iVar8;
  uint uVar9;
  uint uVar10;
  uint *puVar11;
  int unaff_EBP;
  
  __security_init_cookie();
  *(undefined4 *)(unaff_EBP + -100) = 0xffffffff;
  *(undefined4 *)(unaff_EBP + -0x60) = 0xffffffff;
  *(undefined4 *)(unaff_EBP + -0x5c) = 0xffffffff;
  *(undefined1 *)(unaff_EBP + -0x4c) = 0;
  *(undefined1 *)(unaff_EBP + -0x4b) = 0;
  this = *(void **)(unaff_EBP + 8);
  uVar1 = *(undefined4 *)((int)this + 0x44);
  *(void **)(unaff_EBP + -0x50) = this;
  *(undefined4 *)(unaff_EBP + 8) = 0;
  *(undefined4 *)(unaff_EBP + -0x34) = uVar1;
  if (((*(int *)(unaff_EBP + 0xc) == 0) || (*(int *)(unaff_EBP + 0x10) == 0)) ||
     ((*(byte *)((int)this + 9) & 4) != 0)) {
    *(undefined4 *)(unaff_EBP + 0xc) = 0x8876086c;
  }
  else {
    iVar8 = *(int *)((int)this + 0x58);
    pvVar3 = operator_new(iVar8 * 0xc);
    *(void **)(unaff_EBP + -0x30) = pvVar3;
    *(undefined4 *)(unaff_EBP + -4) = 0;
    pvVar4 = (void *)0x0;
    if (pvVar3 != (void *)0x0) {
      RepeatCall(pvVar3,0xc,iVar8,&LAB_0047d949);
      pvVar4 = pvVar3;
    }
    *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
    iVar8 = 0;
    *(void **)((int)this + 0x44) = pvVar4;
    if (pvVar4 == (void *)0x0) {
      *(undefined4 *)(unaff_EBP + 0xc) = 0x8007000e;
    }
    else {
      puVar5 = *(undefined4 **)(unaff_EBP + 0xc);
      uVar10 = 0;
      if (*(int *)((int)this + 0x58) != 0) {
        do {
          *(undefined4 *)(iVar8 + *(int *)((int)this + 0x44)) = *puVar5;
          *(undefined4 *)(iVar8 + 4 + *(int *)((int)this + 0x44)) = puVar5[1];
          puVar6 = puVar5 + 2;
          puVar5 = puVar5 + 3;
          *(undefined4 *)(iVar8 + 8 + *(int *)((int)this + 0x44)) = *puVar6;
          uVar10 = uVar10 + 1;
          iVar8 = iVar8 + 0xc;
        } while (uVar10 < *(uint *)((int)this + 0x58));
      }
      puVar5 = *(undefined4 **)(unaff_EBP + 0x10);
      for (uVar10 = *(uint *)((int)this + 0x30) & 0x3fffffff; uVar10 != 0; uVar10 = uVar10 - 1) {
        *puVar5 = 0xffffffff;
        puVar5 = puVar5 + 1;
      }
      for (iVar8 = 0; iVar8 != 0; iVar8 = iVar8 + -1) {
        *(undefined1 *)puVar5 = 0xff;
        puVar5 = (undefined4 *)((int)puVar5 + 1);
      }
      iVar8 = Graphics_DrawIndexedPrimitive(this,unaff_EBP + 8,0x10);
      *(int *)(unaff_EBP + 0xc) = iVar8;
      if (-1 < iVar8) {
        *(undefined4 *)((int)this + 0x40) = *(undefined4 *)(unaff_EBP + 8);
        if (*(int *)((int)this + 0x54) == 0) {
          *(undefined4 *)(unaff_EBP + -0x40) = *(undefined4 *)((int)this + 0x58);
          uVar1 = *(undefined4 *)((int)this + 0x30);
          *(undefined4 *)(unaff_EBP + -0x20) = 1;
          iVar8 = unaff_EBP + -0x48;
          *(undefined4 *)(unaff_EBP + -0x44) = 0;
          *(undefined4 *)(unaff_EBP + -0x3c) = 0;
          *(undefined4 *)(unaff_EBP + -0x38) = uVar1;
        }
        else {
          *(int *)(unaff_EBP + -0x20) = *(int *)((int)this + 0x54);
          iVar8 = *(int *)((int)this + 0x50);
        }
        *(undefined4 *)(unaff_EBP + -0x1c) = 0;
        if (*(int *)(unaff_EBP + -0x20) != 0) {
          puVar11 = (uint *)(iVar8 + 4);
          *(uint **)(unaff_EBP + -0x2c) = puVar11;
          do {
            uVar10 = *puVar11;
            uVar9 = puVar11[1] + uVar10;
            *(uint *)(unaff_EBP + -0x24) = uVar9;
            *(uint *)(unaff_EBP + -0x14) = uVar10;
            if (uVar10 < uVar9) {
              iVar8 = uVar10 * 0xc;
              *(int *)(unaff_EBP + -0x28) = iVar8;
              do {
                *(undefined4 *)(unaff_EBP + -0x10) = 0;
                *(int *)(unaff_EBP + -0x30) = *(int *)(unaff_EBP + 8) + iVar8;
                do {
                  iVar2 = *(int *)(*(int *)(unaff_EBP + -0x30) + *(int *)(unaff_EBP + -0x10) * 4);
                  if ((iVar2 != -1) && (*(int *)(*(int *)(unaff_EBP + 0x10) + iVar2 * 4) == -1)) {
                    MeshIter_InitEdge((void *)(unaff_EBP + -100),*(int *)(unaff_EBP + -0x14),iVar2,
                                      '\0');
                    *(undefined4 *)(unaff_EBP + -0x18) = 0;
                    while (*(int *)(unaff_EBP + -0x5c) != -1) {
                      iVar7 = Mesh_AdjacencyIteratorNext((int *)(unaff_EBP + -100));
                      if (*(char *)(unaff_EBP + -0x4c) == '\0') {
                        uVar10 = (*(int *)(unaff_EBP + -0x58) + 1U) % 3;
                        iVar8 = *(int *)(unaff_EBP + -0x28);
                      }
                      else {
                        uVar10 = *(uint *)(unaff_EBP + -0x58);
                      }
                      iVar7 = *(int *)(*(int *)(unaff_EBP + 8) + (uVar10 + iVar7 * 3) * 4);
                      *(int *)(unaff_EBP + -0x18) = *(int *)(unaff_EBP + -0x18) + 1;
                      uVar10 = *(uint *)(unaff_EBP + -0x18);
                      *(int *)(*(int *)(unaff_EBP + 0x10) + iVar7 * 4) = iVar2;
                      if (10000 < uVar10) {
                        *(undefined4 *)(unaff_EBP + 0xc) = 0x88760b55;
                        goto LAB_004833a9;
                      }
                    }
                    uVar9 = *(uint *)(unaff_EBP + -0x24);
                  }
                  *(int *)(unaff_EBP + -0x10) = *(int *)(unaff_EBP + -0x10) + 1;
                } while (*(uint *)(unaff_EBP + -0x10) < 3);
                *(int *)(unaff_EBP + -0x14) = *(int *)(unaff_EBP + -0x14) + 1;
                iVar8 = iVar8 + 0xc;
                *(int *)(unaff_EBP + -0x28) = iVar8;
              } while (*(uint *)(unaff_EBP + -0x14) < uVar9);
              puVar11 = *(uint **)(unaff_EBP + -0x2c);
            }
            *(int *)(unaff_EBP + -0x1c) = *(int *)(unaff_EBP + -0x1c) + 1;
            puVar11 = puVar11 + 5;
            *(uint **)(unaff_EBP + -0x2c) = puVar11;
          } while (*(uint *)(unaff_EBP + -0x1c) < *(uint *)(unaff_EBP + -0x20));
        }
        uVar10 = 0;
        if (*(int *)((int)this + 0x30) != 0) {
          do {
            puVar11 = (uint *)(*(int *)(unaff_EBP + 0x10) + uVar10 * 4);
            if (*puVar11 == 0xffffffff) {
              *puVar11 = uVar10;
            }
            uVar10 = uVar10 + 1;
          } while (uVar10 < *(uint *)((int)this + 0x30));
        }
      }
    }
LAB_004833a9:
    if (*(int *)(unaff_EBP + 8) != 0) {
      (**(code **)(**(int **)((int)this + 0x3c) + 0x30))(*(int **)((int)this + 0x3c));
    }
  }
  *(undefined4 *)((int)this + 0x40) = 0;
  _free(*(void **)((int)this + 0x44));
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  *(undefined4 *)((int)this + 0x44) = *(undefined4 *)(unaff_EBP + -0x34);
  return *(undefined4 *)(unaff_EBP + 0xc);
}

