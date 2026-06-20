
undefined4 D3DXMesh_StripifyOptimized(void)

{
  int iVar1;
  void *this;
  bool bVar2;
  bool bVar3;
  int iVar4;
  uint uVar5;
  undefined4 uVar6;
  int *piVar7;
  int extraout_ECX;
  int unaff_EBP;
  uint uVar8;
  void *pvVar9;
  
  __security_init_cookie();
  *(int *)(unaff_EBP + -0x78) = *(int *)(unaff_EBP + 0x14);
  *(undefined4 *)(unaff_EBP + -0x74) = 0;
  iVar4 = *(int *)(extraout_ECX + 0x6c);
  iVar1 = *(int *)(extraout_ECX + 0x54);
  *(undefined4 *)(unaff_EBP + -4) = 0;
  *(undefined4 *)(unaff_EBP + -0x10) = 0;
  *(undefined4 *)(unaff_EBP + -0x30) = 0xffffffff;
  *(undefined4 *)(unaff_EBP + -0x2c) = 0xffffffff;
  *(undefined4 *)(unaff_EBP + -0x34) = 0xffffffff;
  *(int *)(unaff_EBP + -0x28) = *(int *)(unaff_EBP + 0x14) - iVar4;
  *(undefined4 *)(unaff_EBP + -0x20) = 0;
  *(undefined4 *)(unaff_EBP + -0x38) = 0xffffffff;
  *(undefined4 *)(unaff_EBP + -0x1c) = 0;
  if (iVar1 != 0) {
    this = *(void **)(unaff_EBP + 8);
    *(undefined4 *)(unaff_EBP + -0x18) = 0;
    do {
      iVar4 = *(int *)(extraout_ECX + 0x50) + *(int *)(unaff_EBP + -0x18);
      uVar6 = *(undefined4 *)(iVar4 + 4);
      *(undefined4 *)(unaff_EBP + 8) = *(undefined4 *)(iVar4 + 8);
      *(undefined4 *)(unaff_EBP + -0x24) = uVar6;
      *(undefined4 *)(unaff_EBP + -0x14) = 0;
      iVar4 = SmallIntArray_Init((undefined4 *)(unaff_EBP + -0x84));
      *(int *)(unaff_EBP + -0x20) = iVar4;
      if (iVar4 < 0) goto LAB_00484ebb;
      iVar4 = MeshData_InitVertexAdj(this,*(uint *)(unaff_EBP + -0x24),*(int *)(unaff_EBP + 8),'\0')
      ;
      *(int *)(unaff_EBP + -0x20) = iVar4;
      if (iVar4 < 0) goto LAB_00484ebb;
LAB_00484e6c:
      iVar4 = MeshData_FindAttributeOffset((int)this);
      *(int *)(unaff_EBP + -0x30) = iVar4;
      if (iVar4 != -1) {
        piVar7 = (int *)(*(int *)((int)this + 4) + iVar4 * 0xc);
        iVar1 = *piVar7;
        if ((iVar1 == -1) ||
           (*(char *)(*(int *)((int)this + 0x18) + (iVar1 - *(int *)((int)this + 0x1c)) * 0xc) !=
            '\0')) {
          iVar1 = piVar7[1];
          if ((iVar1 == -1) ||
             (*(char *)(*(int *)((int)this + 0x18) + (iVar1 - *(int *)((int)this + 0x1c)) * 0xc) !=
              '\0')) {
            *(undefined4 *)(unaff_EBP + -0x2c) = 0;
          }
          else {
            *(undefined4 *)(unaff_EBP + -0x2c) = 2;
          }
        }
        else {
          *(undefined4 *)(unaff_EBP + -0x2c) = 1;
        }
        do {
          if (*(int *)(unaff_EBP + -0x38) != -1) {
            uVar6 = *(undefined4 *)(unaff_EBP + -0x2c);
            *(undefined4 *)(unaff_EBP + 8) = 1;
            *(int *)(unaff_EBP + -0x40) = iVar4;
            while( true ) {
              *(undefined4 *)(unaff_EBP + -0x3c) = uVar6;
              piVar7 = (int *)Mesh_GetNextEdge((int *)(unaff_EBP + -0x50),(int *)(unaff_EBP + -0x40)
                                               ,(int)this);
              iVar1 = *piVar7;
              *(int *)(unaff_EBP + -0x44) = piVar7[1];
              if ((iVar1 == -1) ||
                 (*(char *)(*(int *)((int)this + 0x18) + (iVar1 - *(int *)((int)this + 0x1c)) * 0xc)
                  != '\0')) break;
              *(int *)(unaff_EBP + 8) = *(int *)(unaff_EBP + 8) + 1;
              uVar6 = *(undefined4 *)(unaff_EBP + -0x44);
              *(int *)(unaff_EBP + -0x40) = iVar1;
            }
            if (*(int *)(unaff_EBP + -0x28) <
                *(int *)(unaff_EBP + 8) + -1 + *(int *)(unaff_EBP + -0x10)) {
              if (*(char *)(*(int *)((int)this + 0x18) +
                           (*(int *)(unaff_EBP + -0x38) - *(int *)((int)this + 0x1c)) * 0xc) == '\0'
                 ) {
                iVar4 = *(int *)(unaff_EBP + -0x38);
                *(int *)(unaff_EBP + -0x30) = iVar4;
                *(undefined4 *)(unaff_EBP + -0x2c) = *(undefined4 *)(unaff_EBP + -0x34);
              }
              *(undefined4 *)(unaff_EBP + -0x38) = 0xffffffff;
            }
          }
          while( true ) {
            while( true ) {
              MeshData_RemoveFace(this,iVar4);
              iVar1 = *(int *)(unaff_EBP + -0x14);
              *(int *)(unaff_EBP + -0x14) = *(int *)(unaff_EBP + -0x14) + 1;
              *(undefined4 *)(unaff_EBP + 8) = 0;
              *(int *)(*(int *)(unaff_EBP + 0xc) + iVar4 * 4) = iVar1 + *(int *)(unaff_EBP + -0x24);
              iVar1 = *(int *)(extraout_ECX + 0x40);
              do {
                MeshData_FindOrAddAttr
                          ((void *)(unaff_EBP + -0x84),
                           *(int *)(iVar1 + iVar4 * 0xc + *(int *)(unaff_EBP + 8) * 4),
                           (char *)(unaff_EBP + 0x17));
                *(int *)(unaff_EBP + -0x10) =
                     *(int *)(unaff_EBP + -0x10) + (1 - (uint)*(byte *)(unaff_EBP + 0x17));
                *(int *)(unaff_EBP + 8) = *(int *)(unaff_EBP + 8) + 1;
              } while (*(uint *)(unaff_EBP + 8) < 3);
              piVar7 = (int *)Mesh_GetNextEdge((int *)(unaff_EBP + -0x58),(int *)(unaff_EBP + -0x30)
                                               ,(int)this);
              iVar4 = *piVar7;
              *(int **)(unaff_EBP + 8) = piVar7;
              *(int *)(unaff_EBP + -0x5c) = piVar7[1];
              pvVar9 = this;
              piVar7 = (int *)Mesh_GetPrevVertex((undefined4 *)(unaff_EBP + -0x68),
                                                 (undefined4 *)(unaff_EBP + -0x30));
              piVar7 = (int *)Mesh_GetNextEdge((int *)(unaff_EBP + -0x70),piVar7,(int)pvVar9);
              iVar1 = *piVar7;
              *(int *)(unaff_EBP + -0x5c) = piVar7[1];
              if ((iVar4 == -1) ||
                 (*(char *)(*(int *)((int)this + 0x18) + (iVar4 - *(int *)((int)this + 0x1c)) * 0xc)
                  != '\0')) {
                bVar2 = false;
              }
              else {
                bVar2 = true;
              }
              if ((iVar1 == -1) ||
                 (*(char *)(*(int *)((int)this + 0x18) + (iVar1 - *(int *)((int)this + 0x1c)) * 0xc)
                  != '\0')) {
                bVar3 = false;
              }
              else {
                bVar3 = true;
              }
              if (!bVar2) break;
              if ((bVar3) && (*(int *)(unaff_EBP + -0x38) == -1)) {
                iVar4 = *piVar7;
                iVar1 = piVar7[1];
                *(undefined4 *)(unaff_EBP + -0x10) = 0;
                *(int *)(unaff_EBP + -0x38) = iVar4;
                *(int *)(unaff_EBP + -0x34) = iVar1;
              }
              iVar4 = **(int **)(unaff_EBP + 8);
              iVar1 = (*(int **)(unaff_EBP + 8))[1];
              *(int *)(unaff_EBP + -0x30) = iVar4;
              *(int *)(unaff_EBP + -0x2c) = iVar1;
            }
            if (bVar3) break;
            iVar4 = *(int *)(unaff_EBP + -0x38);
            *(undefined4 *)(unaff_EBP + -0x38) = 0xffffffff;
            *(int *)(unaff_EBP + -0x30) = iVar4;
            *(undefined4 *)(unaff_EBP + -0x2c) = *(undefined4 *)(unaff_EBP + -0x34);
            if ((iVar4 == -1) ||
               (*(char *)(*(int *)((int)this + 0x18) + (iVar4 - *(int *)((int)this + 0x1c)) * 0xc)
                != '\0')) goto LAB_00484e6c;
          }
          iVar4 = *piVar7;
          iVar1 = piVar7[1];
          *(int *)(unaff_EBP + -0x30) = iVar4;
          *(int *)(unaff_EBP + -0x2c) = iVar1;
        } while( true );
      }
      *(int *)(unaff_EBP + -0x1c) = *(int *)(unaff_EBP + -0x1c) + 1;
      *(int *)(unaff_EBP + -0x18) = *(int *)(unaff_EBP + -0x18) + 0x14;
    } while (*(uint *)(unaff_EBP + -0x1c) < *(uint *)(extraout_ECX + 0x54));
  }
  uVar8 = 0;
  iVar4 = *(int *)(unaff_EBP + 0x10);
  uVar5 = 0;
  if (*(int *)(extraout_ECX + 0x58) != 0) {
    do {
      *(undefined4 *)(iVar4 + uVar5 * 4) = 0xffffffff;
      uVar5 = uVar5 + 1;
    } while (uVar5 < *(uint *)(extraout_ECX + 0x58));
    if (*(int *)(extraout_ECX + 0x58) != 0) {
      do {
        iVar1 = *(int *)(*(int *)(unaff_EBP + 0xc) + uVar8 * 4);
        if (iVar1 != -1) {
          *(uint *)(iVar4 + iVar1 * 4) = uVar8;
        }
        uVar8 = uVar8 + 1;
      } while (uVar8 < *(uint *)(extraout_ECX + 0x58));
    }
  }
LAB_00484ebb:
  *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
  _free(*(void **)(unaff_EBP + -0x74));
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return *(undefined4 *)(unaff_EBP + -0x20);
}

