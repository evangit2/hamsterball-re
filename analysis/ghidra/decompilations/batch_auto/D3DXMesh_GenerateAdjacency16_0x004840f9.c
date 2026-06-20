
undefined4 D3DXMesh_GenerateAdjacency16(void)

{
  ushort uVar1;
  void *this;
  void *pvVar2;
  void *pvVar3;
  undefined2 *puVar4;
  undefined2 *puVar5;
  undefined4 uVar6;
  int iVar7;
  uint uVar8;
  uint uVar9;
  int unaff_EBP;
  undefined4 *puVar10;
  uint *puVar11;
  
  __security_init_cookie();
  this = *(void **)(unaff_EBP + 8);
  MeshFace_Init((void *)(unaff_EBP + -0x48),this);
  uVar6 = *(undefined4 *)((int)this + 0x44);
  *(undefined4 *)(unaff_EBP + 8) = 0;
  *(undefined4 *)(unaff_EBP + -0x34) = uVar6;
  if (((*(int *)(unaff_EBP + 0xc) == 0) || (*(int *)(unaff_EBP + 0x10) == 0)) ||
     ((*(byte *)((int)this + 9) & 4) != 0)) {
    *(undefined4 *)(unaff_EBP + 0xc) = 0x8876086c;
  }
  else {
    iVar7 = *(int *)((int)this + 0x58);
    pvVar2 = operator_new(iVar7 * 6);
    *(void **)(unaff_EBP + -0x30) = pvVar2;
    *(undefined4 *)(unaff_EBP + -4) = 0;
    pvVar3 = (void *)0x0;
    if (pvVar2 != (void *)0x0) {
      RepeatCall(pvVar2,6,iVar7,&LAB_0047d949);
      pvVar3 = pvVar2;
    }
    *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
    iVar7 = 0;
    *(void **)((int)this + 0x44) = pvVar3;
    if (pvVar3 == (void *)0x0) {
      *(undefined4 *)(unaff_EBP + 0xc) = 0x8007000e;
    }
    else {
      puVar4 = *(undefined2 **)(unaff_EBP + 0xc);
      uVar8 = 0;
      if (*(int *)((int)this + 0x58) != 0) {
        do {
          *(undefined2 *)(iVar7 + *(int *)((int)this + 0x44)) = *puVar4;
          *(undefined2 *)(iVar7 + 2 + *(int *)((int)this + 0x44)) = puVar4[2];
          puVar5 = puVar4 + 4;
          puVar4 = puVar4 + 6;
          *(undefined2 *)(iVar7 + 4 + *(int *)((int)this + 0x44)) = *puVar5;
          uVar8 = uVar8 + 1;
          iVar7 = iVar7 + 6;
        } while (uVar8 < *(uint *)((int)this + 0x58));
      }
      puVar10 = *(undefined4 **)(unaff_EBP + 0x10);
      for (uVar8 = *(uint *)((int)this + 0x30) & 0x3fffffff; uVar8 != 0; uVar8 = uVar8 - 1) {
        *puVar10 = 0xffffffff;
        puVar10 = puVar10 + 1;
      }
      for (iVar7 = 0; iVar7 != 0; iVar7 = iVar7 + -1) {
        *(undefined1 *)puVar10 = 0xff;
        puVar10 = (undefined4 *)((int)puVar10 + 1);
      }
      iVar7 = Graphics_DrawIndexedPrimitive(this,unaff_EBP + 8,0x10);
      *(int *)(unaff_EBP + 0xc) = iVar7;
      if (-1 < iVar7) {
        *(undefined4 *)((int)this + 0x40) = *(undefined4 *)(unaff_EBP + 8);
        if (*(int *)((int)this + 0x54) == 0) {
          *(undefined4 *)(unaff_EBP + -0x54) = *(undefined4 *)((int)this + 0x58);
          uVar6 = *(undefined4 *)((int)this + 0x30);
          *(undefined4 *)(unaff_EBP + -0x20) = 1;
          iVar7 = unaff_EBP + -0x5c;
          *(undefined4 *)(unaff_EBP + -0x58) = 0;
          *(undefined4 *)(unaff_EBP + -0x50) = 0;
          *(undefined4 *)(unaff_EBP + -0x4c) = uVar6;
        }
        else {
          *(int *)(unaff_EBP + -0x20) = *(int *)((int)this + 0x54);
          iVar7 = *(int *)((int)this + 0x50);
        }
        *(undefined4 *)(unaff_EBP + -0x1c) = 0;
        if (*(int *)(unaff_EBP + -0x20) != 0) {
          puVar11 = (uint *)(iVar7 + 4);
          *(uint **)(unaff_EBP + -0x28) = puVar11;
          do {
            uVar8 = *puVar11;
            uVar9 = puVar11[1] + uVar8;
            *(uint *)(unaff_EBP + -0x24) = uVar9;
            *(uint *)(unaff_EBP + -0x14) = uVar8;
            if (uVar8 < uVar9) {
              iVar7 = uVar8 * 6;
              *(int *)(unaff_EBP + -0x18) = iVar7;
              do {
                *(undefined4 *)(unaff_EBP + -0x10) = 0;
                *(int *)(unaff_EBP + -0x2c) = *(int *)(unaff_EBP + 8) + iVar7;
                do {
                  uVar1 = *(ushort *)(*(int *)(unaff_EBP + -0x2c) + *(int *)(unaff_EBP + -0x10) * 2)
                  ;
                  if ((uVar1 != 0xffff) &&
                     (*(int *)(*(int *)(unaff_EBP + 0x10) + (uint)uVar1 * 4) == -1)) {
                    uVar8 = 0;
                    MeshIter_InitShortEdge
                              ((void *)(unaff_EBP + -0x48),
                               (ushort)*(undefined4 *)(unaff_EBP + -0x14),uVar1,'\0');
                    while (*(short *)(unaff_EBP + -0x44) != -1) {
                      uVar6 = Mesh_AdjacencyIteratorNext16((ushort *)(unaff_EBP + -0x48));
                      *(undefined4 *)(unaff_EBP + -0x30) = uVar6;
                      if (*(char *)(unaff_EBP + -0x38) == '\0') {
                        uVar9 = (*(ushort *)(unaff_EBP + -0x42) + 1) % 3;
                      }
                      else {
                        uVar9 = (uint)*(ushort *)(unaff_EBP + -0x42);
                      }
                      uVar8 = uVar8 + 1;
                      *(uint *)(*(int *)(unaff_EBP + 0x10) +
                               (uint)*(ushort *)
                                      (*(int *)(unaff_EBP + 8) +
                                      ((uint)*(ushort *)(unaff_EBP + -0x30) * 3 + uVar9) * 2) * 4) =
                           (uint)uVar1;
                      if (10000 < uVar8) {
                        *(undefined4 *)(unaff_EBP + 0xc) = 0x88760b55;
                        goto LAB_00484337;
                      }
                    }
                    uVar9 = *(uint *)(unaff_EBP + -0x24);
                    iVar7 = *(int *)(unaff_EBP + -0x18);
                  }
                  *(int *)(unaff_EBP + -0x10) = *(int *)(unaff_EBP + -0x10) + 1;
                } while (*(uint *)(unaff_EBP + -0x10) < 3);
                *(int *)(unaff_EBP + -0x14) = *(int *)(unaff_EBP + -0x14) + 1;
                iVar7 = iVar7 + 6;
                *(int *)(unaff_EBP + -0x18) = iVar7;
              } while (*(uint *)(unaff_EBP + -0x14) < uVar9);
              puVar11 = *(uint **)(unaff_EBP + -0x28);
            }
            *(int *)(unaff_EBP + -0x1c) = *(int *)(unaff_EBP + -0x1c) + 1;
            puVar11 = puVar11 + 5;
            *(uint **)(unaff_EBP + -0x28) = puVar11;
          } while (*(uint *)(unaff_EBP + -0x1c) < *(uint *)(unaff_EBP + -0x20));
        }
        uVar8 = 0;
        if (*(int *)((int)this + 0x30) != 0) {
          do {
            puVar11 = (uint *)(*(int *)(unaff_EBP + 0x10) + uVar8 * 4);
            if (*puVar11 == 0xffffffff) {
              *puVar11 = uVar8;
            }
            uVar8 = uVar8 + 1;
          } while (uVar8 < *(uint *)((int)this + 0x30));
        }
      }
    }
LAB_00484337:
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

