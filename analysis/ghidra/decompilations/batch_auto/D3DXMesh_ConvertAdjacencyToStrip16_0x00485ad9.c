
int D3DXMesh_ConvertAdjacencyToStrip16(void)

{
  int *piVar1;
  undefined2 uVar2;
  uint uVar3;
  uint *puVar4;
  void *pvVar5;
  uint uVar6;
  int iVar7;
  ushort *puVar8;
  undefined4 *puVar9;
  void *extraout_ECX;
  undefined2 *puVar10;
  uint uVar11;
  int unaff_EBP;
  int iVar12;
  undefined4 *puVar13;
  
  __security_init_cookie();
  iVar12 = *(int *)((int)extraout_ECX + 0x68);
  *(void **)(unaff_EBP + -0x48) = extraout_ECX;
  *(undefined4 *)(unaff_EBP + -0x40) = 0xc;
  if (iVar12 != 0) {
    *(int *)(unaff_EBP + -0x40) = iVar12;
  }
  uVar3 = *(uint *)(unaff_EBP + 8);
  if (((uVar3 & 0x4000000) != 0) && (iVar12 == 0)) {
    uVar3 = uVar3 & 0xfbffffff | 0x8000000;
  }
  *(undefined4 *)(unaff_EBP + -0x7c) = 0;
  *(undefined4 *)(unaff_EBP + -0x6c) = 0;
  *(undefined4 *)(unaff_EBP + -0x5c) = 0;
  *(undefined4 *)(unaff_EBP + -4) = 0;
  if (*(int *)(unaff_EBP + 0x14) != 0) {
    *(uint *)(unaff_EBP + -0x28) = uVar3;
    puVar4 = (uint *)(unaff_EBP + -0x28);
    *puVar4 = *puVar4 & 0x8000000;
    if ((*puVar4 == 0) || ((uVar3 & 0x4000000) == 0)) {
      iVar12 = *(int *)((int)extraout_ECX + 0x54);
      *(undefined4 *)(unaff_EBP + -0x20) = 0;
      if (iVar12 != 0) {
        puVar4 = (uint *)(*(int *)((int)extraout_ECX + 0x50) + 8);
        do {
          if (*(uint *)(unaff_EBP + -0x20) < *puVar4) {
            *(uint *)(unaff_EBP + -0x20) = *puVar4;
          }
          puVar4 = puVar4 + 5;
          iVar12 = iVar12 + -1;
        } while (iVar12 != 0);
      }
      iVar12 = *(int *)((int)extraout_ECX + 0x58);
      pvVar5 = operator_new(iVar12 * 6);
      *(void **)(unaff_EBP + 8) = pvVar5;
      *(undefined1 *)(unaff_EBP + -4) = 1;
      if (pvVar5 == (void *)0x0) {
        *(undefined4 *)(unaff_EBP + -0x10) = 0;
      }
      else {
        RepeatCall(pvVar5,6,iVar12,&LAB_0047d949);
        *(void **)(unaff_EBP + -0x10) = pvVar5;
      }
      *(undefined1 *)(unaff_EBP + -4) = 0;
      if (*(int *)(unaff_EBP + -0x10) == 0) {
        iVar12 = -0x7ff8fff2;
      }
      else {
        iVar12 = *(int *)((int)extraout_ECX + 0x54);
        *(undefined4 *)(unaff_EBP + -0x2c) = 0;
        pvVar5 = extraout_ECX;
        if (iVar12 != 0) {
          *(undefined4 *)(unaff_EBP + -0x1c) = 0;
          do {
            iVar12 = *(int *)((int)pvVar5 + 0x50) + *(int *)(unaff_EBP + -0x1c);
            uVar3 = *(uint *)(iVar12 + 4);
            uVar11 = *(int *)(iVar12 + 8) + uVar3;
            *(uint *)(unaff_EBP + -0x50) = uVar11;
            if (uVar3 < uVar11) {
              *(uint *)(unaff_EBP + -0x30) = uVar3 * 3;
              *(uint *)(unaff_EBP + -0x18) = *(int *)(unaff_EBP + 0x10) + uVar3 * 4;
              puVar10 = (undefined2 *)(*(int *)(unaff_EBP + -0x10) + uVar3 * 6);
              *(uint *)(unaff_EBP + -0x38) = uVar11 - uVar3;
              do {
                puVar8 = (ushort *)
                         (*(int *)(unaff_EBP + 0x14) + **(int **)(unaff_EBP + -0x18) * 0xc);
                *(ushort **)(unaff_EBP + -0x3c) = puVar8;
                *(undefined4 *)(unaff_EBP + 8) = 1;
                *(undefined2 **)(unaff_EBP + -0x34) = puVar10;
                *(ushort **)(unaff_EBP + -0x14) = puVar8;
                *(undefined4 *)(unaff_EBP + -0x44) = 3;
                do {
                  if (*(int *)puVar8 == -1) {
                    *puVar10 = 0xffff;
                  }
                  else {
                    uVar6 = (uint)*(ushort *)(*(int *)(unaff_EBP + 0xc) + (uint)*puVar8 * 4);
                    *(uint *)(unaff_EBP + -0x58) = uVar6;
                    if ((((uVar6 < uVar3) || (uVar11 <= uVar6)) ||
                        (uVar11 = *(uint *)(unaff_EBP + 8) % 3,
                        iVar7 = **(int **)(unaff_EBP + -0x14),
                        iVar12 = *(int *)(*(int *)(unaff_EBP + -0x3c) + uVar11 * 4),
                        *(uint *)(unaff_EBP + -0x24) = uVar11, iVar7 == iVar12)) ||
                       (**(int **)(unaff_EBP + -0x14) ==
                        *(int *)(*(int *)(unaff_EBP + -0x3c) +
                                ((*(int *)(unaff_EBP + 8) + 1U) % 3) * 4))) {
                      uVar2 = 0xffff;
                    }
                    else {
                      iVar7 = FindInSmallIntArray(*(int *)(unaff_EBP + 0x14) +
                                                  *(int *)(*(int *)(unaff_EBP + 0x10) + uVar6 * 4) *
                                                  0xc,**(int **)(unaff_EBP + -0x18));
                      iVar12 = *(int *)((int)pvVar5 + 0x40);
                      *(uint *)(unaff_EBP + -0x4c) =
                           (uint)*(ushort *)
                                  (iVar12 + -2 +
                                  (*(int *)(unaff_EBP + 8) + *(int *)(unaff_EBP + -0x30)) * 2);
                      *(uint *)(unaff_EBP + -0x54) =
                           (uint)*(ushort *)
                                  (iVar12 + (*(int *)(unaff_EBP + -0x30) +
                                            *(int *)(unaff_EBP + -0x24)) * 2);
                      *(uint *)(unaff_EBP + -0x24) =
                           (uint)*(ushort *)(iVar12 + (uVar6 * 3 + iVar7) * 2);
                      if ((*(uint *)(unaff_EBP + -0x4c) ==
                           (uint)*(ushort *)(iVar12 + ((iVar7 + 1U) % 3 + uVar6 * 3) * 2)) &&
                         (*(int *)(unaff_EBP + -0x54) == *(int *)(unaff_EBP + -0x24))) {
                        uVar2 = (undefined2)*(undefined4 *)(unaff_EBP + -0x58);
                      }
                      else {
                        uVar2 = 0xffff;
                      }
                      pvVar5 = *(void **)(unaff_EBP + -0x48);
                    }
                    puVar10 = *(undefined2 **)(unaff_EBP + -0x34);
                    uVar11 = *(uint *)(unaff_EBP + -0x50);
                    *puVar10 = uVar2;
                  }
                  puVar8 = (ushort *)(*(int *)(unaff_EBP + -0x14) + 4);
                  *(int *)(unaff_EBP + 8) = *(int *)(unaff_EBP + 8) + 1;
                  puVar10 = puVar10 + 1;
                  piVar1 = (int *)(unaff_EBP + -0x44);
                  *piVar1 = *piVar1 + -1;
                  *(ushort **)(unaff_EBP + -0x14) = puVar8;
                  *(undefined2 **)(unaff_EBP + -0x34) = puVar10;
                } while (*piVar1 != 0);
                *(int *)(unaff_EBP + -0x18) = *(int *)(unaff_EBP + -0x18) + 4;
                *(int *)(unaff_EBP + -0x30) = *(int *)(unaff_EBP + -0x30) + 3;
                piVar1 = (int *)(unaff_EBP + -0x38);
                *piVar1 = *piVar1 + -1;
              } while (*piVar1 != 0);
            }
            *(int *)(unaff_EBP + -0x2c) = *(int *)(unaff_EBP + -0x2c) + 1;
            *(int *)(unaff_EBP + -0x1c) = *(int *)(unaff_EBP + -0x1c) + 0x14;
          } while (*(uint *)(unaff_EBP + -0x2c) < *(uint *)((int)pvVar5 + 0x54));
        }
        uVar3 = 0;
        if (*(int *)((int)pvVar5 + 0x58) != 0) {
          puVar4 = *(uint **)(unaff_EBP + 0x10);
          iVar12 = *(int *)(unaff_EBP + 0xc) - (int)puVar4;
          do {
            *(uint *)(iVar12 + (int)puVar4) = uVar3;
            *puVar4 = uVar3;
            uVar3 = uVar3 + 1;
            puVar4 = puVar4 + 1;
          } while (uVar3 < *(uint *)((int)pvVar5 + 0x58));
        }
        puVar9 = (undefined4 *)
                 MeshSubMesh_Init((void *)(unaff_EBP + -0xa0),pvVar5,
                                  *(undefined4 *)(unaff_EBP + -0x10),
                                  (short)*(undefined4 *)(unaff_EBP + -0x20));
        puVar13 = (undefined4 *)(unaff_EBP + -0x7c);
        for (iVar12 = 9; iVar12 != 0; iVar12 = iVar12 + -1) {
          *puVar13 = *puVar9;
          puVar9 = puVar9 + 1;
          puVar13 = puVar13 + 1;
        }
        MeshData_FreeBuffersB(unaff_EBP + -0xa0);
        if (*(int *)(unaff_EBP + -0x28) == 0) {
          iVar12 = D3DXMesh_StripifyOptimized16();
        }
        else {
          iVar12 = D3DXMesh_Stripify16(pvVar5,(void *)(unaff_EBP + -0x7c),*(int *)(unaff_EBP + 0xc),
                                       *(int *)(unaff_EBP + 0x10));
        }
      }
      _free(*(void **)(unaff_EBP + -0x10));
      goto LAB_00485db8;
    }
  }
  iVar12 = -0x7789f794;
LAB_00485db8:
  *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
  MeshData_FreeBuffersB(unaff_EBP + -0x7c);
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return iVar12;
}

