
undefined4 D3DXMesh_StripifyOptimized16(void)

{
  ushort uVar1;
  int iVar2;
  void *this;
  bool bVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  undefined4 uVar7;
  uint *puVar8;
  ushort *puVar9;
  uint *puVar10;
  int extraout_ECX;
  uint uVar11;
  int unaff_EBP;
  void *pvVar12;
  
  __security_init_cookie();
  *(int *)(unaff_EBP + -0x10) = extraout_ECX;
  *(int *)(unaff_EBP + -0x80) = *(int *)(unaff_EBP + 0x14);
  *(undefined4 *)(unaff_EBP + -0x7c) = 0;
  iVar4 = *(int *)(extraout_ECX + 0x6c);
  *(undefined4 *)(unaff_EBP + -0x38) = 0xffffffff;
  *(undefined4 *)(unaff_EBP + -0x34) = 0xffffffff;
  *(undefined4 *)(unaff_EBP + -0x3c) = 0xffffffff;
  iVar2 = *(int *)(extraout_ECX + 0x54);
  *(undefined4 *)(unaff_EBP + -4) = 0;
  *(undefined4 *)(unaff_EBP + -0x14) = 0;
  *(int *)(unaff_EBP + -0x2c) = *(int *)(unaff_EBP + 0x14) - iVar4;
  *(undefined4 *)(unaff_EBP + -0x24) = 0;
  *(undefined4 *)(unaff_EBP + -0x40) = 0xffff;
  *(undefined4 *)(unaff_EBP + -0x20) = 0;
  iVar4 = extraout_ECX;
  if (iVar2 != 0) {
    this = *(void **)(unaff_EBP + 8);
    *(undefined4 *)(unaff_EBP + -0x1c) = 0;
    do {
      iVar4 = *(int *)(iVar4 + 0x50) + *(int *)(unaff_EBP + -0x1c);
      uVar7 = *(undefined4 *)(iVar4 + 4);
      *(undefined4 *)(unaff_EBP + 0x14) = *(undefined4 *)(iVar4 + 8);
      *(undefined4 *)(unaff_EBP + -0x28) = uVar7;
      *(undefined4 *)(unaff_EBP + -0x18) = 0;
      iVar4 = ShortArray_Init((undefined4 *)(unaff_EBP + -0x8c));
      *(int *)(unaff_EBP + -0x24) = iVar4;
      if (iVar4 < 0) goto LAB_004852c9;
      iVar4 = MeshData_InitShortVertexAdj
                        (this,(ushort)*(undefined4 *)(unaff_EBP + -0x28),
                         (ushort)*(undefined4 *)(unaff_EBP + 0x14),'\0');
      *(int *)(unaff_EBP + -0x24) = iVar4;
      if (iVar4 < 0) goto LAB_004852c9;
LAB_00485268:
      uVar5 = MeshEdge_GetFirstVertex((int)this);
      uVar5 = uVar5 & 0xffff;
      *(uint *)(unaff_EBP + -0x38) = uVar5;
      if (uVar5 != 0xffff) {
        puVar9 = (ushort *)(*(int *)((int)this + 4) + uVar5 * 6);
        uVar1 = *puVar9;
        if ((uVar1 == 0xffff) ||
           (*(char *)(*(int *)((int)this + 0x10) +
                     ((uint)uVar1 - (uint)*(ushort *)((int)this + 0x14)) * 6) != '\0')) {
          uVar1 = puVar9[1];
          if ((uVar1 == 0xffff) ||
             (*(char *)(*(int *)((int)this + 0x10) +
                       ((uint)uVar1 - (uint)*(ushort *)((int)this + 0x14)) * 6) != '\0')) {
            *(undefined4 *)(unaff_EBP + -0x34) = 0;
          }
          else {
            *(undefined4 *)(unaff_EBP + -0x34) = 2;
          }
        }
        else {
          *(undefined4 *)(unaff_EBP + -0x34) = 1;
        }
        do {
          if (*(int *)(unaff_EBP + -0x40) != 0xffff) {
            uVar7 = *(undefined4 *)(unaff_EBP + -0x34);
            *(undefined4 *)(unaff_EBP + 0x14) = 1;
            *(uint *)(unaff_EBP + -0x48) = uVar5;
            while( true ) {
              *(undefined4 *)(unaff_EBP + -0x44) = uVar7;
              puVar8 = ShortArray_GetNextEdge
                                 ((uint *)(unaff_EBP + -0x58),(ushort *)(unaff_EBP + -0x48),
                                  (int)this);
              uVar11 = *puVar8;
              *(uint *)(unaff_EBP + -0x4c) = puVar8[1];
              if ((uVar11 == 0xffff) ||
                 (*(char *)(*(int *)((int)this + 0x10) +
                           ((uVar11 & 0xffff) - (uint)*(ushort *)((int)this + 0x14)) * 6) != '\0'))
              break;
              *(int *)(unaff_EBP + 0x14) = *(int *)(unaff_EBP + 0x14) + 1;
              uVar7 = *(undefined4 *)(unaff_EBP + -0x4c);
              *(uint *)(unaff_EBP + -0x48) = uVar11;
            }
            if (*(int *)(unaff_EBP + -0x2c) <
                *(int *)(unaff_EBP + 0x14) + -1 + *(int *)(unaff_EBP + -0x14)) {
              if (*(char *)(*(int *)((int)this + 0x10) +
                           ((uint)*(ushort *)(unaff_EBP + -0x40) -
                           (uint)*(ushort *)((int)this + 0x14)) * 6) == '\0') {
                uVar5 = *(uint *)(unaff_EBP + -0x40);
                *(uint *)(unaff_EBP + -0x38) = uVar5;
                *(undefined4 *)(unaff_EBP + -0x34) = *(undefined4 *)(unaff_EBP + -0x3c);
              }
              *(undefined4 *)(unaff_EBP + -0x40) = 0xffff;
            }
          }
          while( true ) {
            while( true ) {
              MeshData_RemoveShortFace(this,(ushort)uVar5);
              uVar1 = *(ushort *)(unaff_EBP + -0x18);
              *(int *)(unaff_EBP + -0x18) = *(int *)(unaff_EBP + -0x18) + 1;
              *(undefined4 *)(unaff_EBP + 0x14) = 0;
              *(uint *)(*(int *)(unaff_EBP + 0xc) + uVar5 * 4) =
                   (uint)uVar1 + *(int *)(unaff_EBP + -0x28);
              iVar4 = *(int *)(*(int *)(unaff_EBP + -0x10) + 0x40);
              do {
                MeshData_FindOrAddShortAttr
                          ((void *)(unaff_EBP + -0x8c),
                           *(short *)(iVar4 + uVar5 * 6 + *(int *)(unaff_EBP + 0x14) * 2),
                           (char *)(unaff_EBP + 0xb));
                *(int *)(unaff_EBP + -0x14) =
                     *(int *)(unaff_EBP + -0x14) + (1 - (uint)*(byte *)(unaff_EBP + 0xb));
                *(int *)(unaff_EBP + 0x14) = *(int *)(unaff_EBP + 0x14) + 1;
              } while (*(uint *)(unaff_EBP + 0x14) < 3);
              puVar8 = ShortArray_GetNextEdge
                                 ((uint *)(unaff_EBP + -0x60),(ushort *)(unaff_EBP + -0x38),
                                  (int)this);
              *(uint *)(unaff_EBP + -100) = puVar8[1];
              *(uint **)(unaff_EBP + -0x30) = puVar8;
              pvVar12 = this;
              puVar9 = (ushort *)
                       Mesh_GetPrevVertex((undefined4 *)(unaff_EBP + -0x70),
                                          (undefined4 *)(unaff_EBP + -0x38));
              puVar10 = ShortArray_GetNextEdge((uint *)(unaff_EBP + -0x78),puVar9,(int)pvVar12);
              *(uint *)(unaff_EBP + -100) = puVar10[1];
              if ((*puVar8 == 0xffff) ||
                 (*(char *)(*(int *)((int)this + 0x10) +
                           ((*puVar8 & 0xffff) - (uint)*(ushort *)((int)this + 0x14)) * 6) != '\0'))
              {
                *(undefined1 *)(unaff_EBP + 0x17) = 0;
              }
              else {
                *(undefined1 *)(unaff_EBP + 0x17) = 1;
              }
              if ((*puVar10 == 0xffff) ||
                 (*(char *)(*(int *)((int)this + 0x10) +
                           ((*puVar10 & 0xffff) - (uint)*(ushort *)((int)this + 0x14)) * 6) != '\0')
                 ) {
                bVar3 = false;
              }
              else {
                bVar3 = true;
              }
              if (*(char *)(unaff_EBP + 0x17) == '\0') break;
              if ((bVar3) && (*(int *)(unaff_EBP + -0x40) == 0xffff)) {
                uVar5 = *puVar10;
                uVar11 = puVar10[1];
                *(undefined4 *)(unaff_EBP + -0x14) = 0;
                *(uint *)(unaff_EBP + -0x40) = uVar5;
                *(uint *)(unaff_EBP + -0x3c) = uVar11;
              }
              uVar5 = *puVar8;
              uVar7 = *(undefined4 *)(*(int *)(unaff_EBP + -0x30) + 4);
              *(uint *)(unaff_EBP + -0x38) = uVar5;
              *(undefined4 *)(unaff_EBP + -0x34) = uVar7;
            }
            if (bVar3) break;
            uVar5 = *(uint *)(unaff_EBP + -0x40);
            *(uint *)(unaff_EBP + -0x38) = uVar5;
            *(undefined4 *)(unaff_EBP + -0x34) = *(undefined4 *)(unaff_EBP + -0x3c);
            *(undefined4 *)(unaff_EBP + -0x40) = 0xffff;
            if ((uVar5 == 0xffff) ||
               (*(char *)(*(int *)((int)this + 0x10) +
                         ((uVar5 & 0xffff) - (uint)*(ushort *)((int)this + 0x14)) * 6) != '\0'))
            goto LAB_00485268;
          }
          uVar5 = *puVar10;
          uVar11 = puVar10[1];
          *(uint *)(unaff_EBP + -0x38) = uVar5;
          *(uint *)(unaff_EBP + -0x34) = uVar11;
        } while( true );
      }
      *(int *)(unaff_EBP + -0x20) = *(int *)(unaff_EBP + -0x20) + 1;
      iVar4 = *(int *)(unaff_EBP + -0x10);
      *(int *)(unaff_EBP + -0x1c) = *(int *)(unaff_EBP + -0x1c) + 0x14;
    } while (*(uint *)(unaff_EBP + -0x20) < *(uint *)(iVar4 + 0x54));
  }
  uVar11 = 0;
  iVar2 = *(int *)(unaff_EBP + 0x10);
  uVar5 = 0;
  if (*(int *)(iVar4 + 0x58) != 0) {
    uVar6 = 0;
    do {
      uVar5 = uVar5 + 1;
      *(undefined4 *)(iVar2 + uVar6 * 4) = 0xffff;
      uVar6 = uVar5 & 0xffff;
    } while (uVar6 < *(uint *)(iVar4 + 0x58));
    if (*(int *)(iVar4 + 0x58) != 0) {
      uVar5 = 0;
      do {
        uVar1 = *(ushort *)(*(int *)(unaff_EBP + 0xc) + uVar5 * 4);
        if (uVar1 != 0xffff) {
          *(uint *)(iVar2 + (uint)uVar1 * 4) = uVar5;
        }
        uVar11 = uVar11 + 1;
        uVar5 = uVar11 & 0xffff;
      } while (uVar5 < *(uint *)(iVar4 + 0x58));
    }
  }
LAB_004852c9:
  *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
  _free(*(void **)(unaff_EBP + -0x7c));
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return *(undefined4 *)(unaff_EBP + -0x24);
}

