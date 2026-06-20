
int D3DXMesh_ConvertAdjacencyToStrip(void)

{
  int *piVar1;
  uint uVar2;
  uint *puVar3;
  void *pvVar4;
  uint uVar5;
  int iVar6;
  undefined4 *puVar7;
  void *extraout_ECX;
  uint uVar8;
  int *piVar9;
  int unaff_EBP;
  int iVar10;
  undefined4 *puVar11;
  
  __security_init_cookie();
  iVar10 = *(int *)((int)extraout_ECX + 0x68);
  *(void **)(unaff_EBP + -0x38) = extraout_ECX;
  *(undefined4 *)(unaff_EBP + -0x44) = 0xc;
  if (iVar10 != 0) {
    *(int *)(unaff_EBP + -0x44) = iVar10;
  }
  uVar2 = *(uint *)(unaff_EBP + 8);
  if (((uVar2 & 0x4000000) != 0) && (iVar10 == 0)) {
    uVar2 = uVar2 & 0xfbffffff | 0x8000000;
  }
  *(undefined4 *)(unaff_EBP + -0x8c) = 0;
  *(undefined4 *)(unaff_EBP + -0x74) = 0;
  *(undefined4 *)(unaff_EBP + -0x60) = 0;
  *(undefined4 *)(unaff_EBP + -4) = 0;
  if (*(int *)(unaff_EBP + 0x14) != 0) {
    *(uint *)(unaff_EBP + -0x2c) = uVar2;
    puVar3 = (uint *)(unaff_EBP + -0x2c);
    *puVar3 = *puVar3 & 0x8000000;
    if ((*puVar3 == 0) || ((uVar2 & 0x4000000) == 0)) {
      iVar10 = *(int *)((int)extraout_ECX + 0x54);
      *(undefined4 *)(unaff_EBP + -0x24) = 0;
      if (iVar10 != 0) {
        puVar3 = (uint *)(*(int *)((int)extraout_ECX + 0x50) + 8);
        do {
          if (*(uint *)(unaff_EBP + -0x24) < *puVar3) {
            *(uint *)(unaff_EBP + -0x24) = *puVar3;
          }
          puVar3 = puVar3 + 5;
          iVar10 = iVar10 + -1;
        } while (iVar10 != 0);
      }
      iVar10 = *(int *)((int)extraout_ECX + 0x58);
      pvVar4 = operator_new(iVar10 * 0xc);
      *(void **)(unaff_EBP + 8) = pvVar4;
      *(undefined1 *)(unaff_EBP + -4) = 1;
      if (pvVar4 == (void *)0x0) {
        *(undefined4 *)(unaff_EBP + -0x10) = 0;
      }
      else {
        RepeatCall(pvVar4,0xc,iVar10,&LAB_0047d949);
        *(void **)(unaff_EBP + -0x10) = pvVar4;
      }
      *(undefined1 *)(unaff_EBP + -4) = 0;
      if (*(int *)(unaff_EBP + -0x10) == 0) {
        iVar10 = -0x7ff8fff2;
      }
      else {
        iVar10 = *(int *)((int)extraout_ECX + 0x54);
        *(undefined4 *)(unaff_EBP + -0x20) = 0;
        pvVar4 = extraout_ECX;
        if (iVar10 != 0) {
          *(undefined4 *)(unaff_EBP + -0x28) = 0;
          do {
            iVar10 = *(int *)((int)pvVar4 + 0x50) + *(int *)(unaff_EBP + -0x28);
            uVar2 = *(uint *)(iVar10 + 4);
            uVar5 = *(int *)(iVar10 + 8) + uVar2;
            *(uint *)(unaff_EBP + -0x40) = uVar2;
            *(uint *)(unaff_EBP + -0x58) = uVar5;
            if (uVar2 < uVar5) {
              *(uint *)(unaff_EBP + -0x1c) = uVar2 * 3;
              *(uint *)(unaff_EBP + -0x18) = *(int *)(unaff_EBP + 0x10) + uVar2 * 4;
              *(uint *)(unaff_EBP + -0x34) = *(int *)(unaff_EBP + -0x10) + uVar2 * 0xc;
              *(uint *)(unaff_EBP + -0x48) = uVar5 - uVar2;
              do {
                piVar9 = (int *)(*(int *)(unaff_EBP + 0x14) + **(int **)(unaff_EBP + -0x18) * 0xc);
                iVar10 = *(int *)(unaff_EBP + -0x34) - (int)piVar9;
                *(int **)(unaff_EBP + -0x3c) = piVar9;
                *(undefined4 *)(unaff_EBP + 8) = 1;
                *(int **)(unaff_EBP + -0x14) = piVar9;
                *(int *)(unaff_EBP + -0x54) = iVar10;
                *(undefined4 *)(unaff_EBP + -0x4c) = 3;
                do {
                  if (*piVar9 == -1) {
                    *(undefined4 *)(iVar10 + (int)piVar9) = 0xffffffff;
                  }
                  else {
                    uVar5 = *(uint *)(*(int *)(unaff_EBP + 0xc) + *piVar9 * 4);
                    if ((((uVar5 < uVar2) || (*(uint *)(unaff_EBP + -0x58) <= uVar5)) ||
                        (uVar8 = *(uint *)(unaff_EBP + 8) % 3, iVar6 = **(int **)(unaff_EBP + -0x14)
                        , iVar10 = *(int *)(*(int *)(unaff_EBP + -0x3c) + uVar8 * 4),
                        *(uint *)(unaff_EBP + -0x30) = uVar8, iVar6 == iVar10)) ||
                       (**(int **)(unaff_EBP + -0x14) ==
                        *(int *)(*(int *)(unaff_EBP + -0x3c) +
                                ((*(int *)(unaff_EBP + 8) + 1U) % 3) * 4))) {
                      uVar5 = 0xffffffff;
                    }
                    else {
                      iVar6 = FindInSmallIntArray(*(int *)(unaff_EBP + 0x14) +
                                                  *(int *)(*(int *)(unaff_EBP + 0x10) + uVar5 * 4) *
                                                  0xc,**(int **)(unaff_EBP + -0x18));
                      iVar10 = *(int *)((int)pvVar4 + 0x40);
                      *(undefined4 *)(unaff_EBP + -0x50) =
                           *(undefined4 *)
                            (iVar10 + -4 +
                            (*(int *)(unaff_EBP + 8) + *(int *)(unaff_EBP + -0x1c)) * 4);
                      *(undefined4 *)(unaff_EBP + -0x5c) =
                           *(undefined4 *)
                            (iVar10 + (*(int *)(unaff_EBP + -0x1c) + *(int *)(unaff_EBP + -0x30)) *
                                      4);
                      *(undefined4 *)(unaff_EBP + -0x30) =
                           *(undefined4 *)(iVar10 + (uVar5 * 3 + iVar6) * 4);
                      if ((*(int *)(unaff_EBP + -0x50) !=
                           *(int *)(iVar10 + ((iVar6 + 1U) % 3 + uVar5 * 3) * 4)) ||
                         (*(int *)(unaff_EBP + -0x5c) != *(int *)(unaff_EBP + -0x30))) {
                        uVar5 = 0xffffffff;
                      }
                      pvVar4 = *(void **)(unaff_EBP + -0x38);
                      uVar2 = *(uint *)(unaff_EBP + -0x40);
                    }
                    iVar10 = *(int *)(unaff_EBP + -0x54);
                    piVar9 = *(int **)(unaff_EBP + -0x14);
                    *(uint *)(iVar10 + (int)piVar9) = uVar5;
                  }
                  *(int *)(unaff_EBP + 8) = *(int *)(unaff_EBP + 8) + 1;
                  piVar9 = piVar9 + 1;
                  piVar1 = (int *)(unaff_EBP + -0x4c);
                  *piVar1 = *piVar1 + -1;
                  *(int **)(unaff_EBP + -0x14) = piVar9;
                } while (*piVar1 != 0);
                *(int *)(unaff_EBP + -0x34) = *(int *)(unaff_EBP + -0x34) + 0xc;
                *(int *)(unaff_EBP + -0x18) = *(int *)(unaff_EBP + -0x18) + 4;
                *(int *)(unaff_EBP + -0x1c) = *(int *)(unaff_EBP + -0x1c) + 3;
                piVar9 = (int *)(unaff_EBP + -0x48);
                *piVar9 = *piVar9 + -1;
              } while (*piVar9 != 0);
            }
            *(int *)(unaff_EBP + -0x20) = *(int *)(unaff_EBP + -0x20) + 1;
            *(int *)(unaff_EBP + -0x28) = *(int *)(unaff_EBP + -0x28) + 0x14;
          } while (*(uint *)(unaff_EBP + -0x20) < *(uint *)((int)pvVar4 + 0x54));
        }
        uVar2 = 0;
        if (*(int *)((int)pvVar4 + 0x58) != 0) {
          puVar3 = *(uint **)(unaff_EBP + 0x10);
          iVar10 = *(int *)(unaff_EBP + 0xc) - (int)puVar3;
          do {
            *(uint *)(iVar10 + (int)puVar3) = uVar2;
            *puVar3 = uVar2;
            uVar2 = uVar2 + 1;
            puVar3 = puVar3 + 1;
          } while (uVar2 < *(uint *)((int)pvVar4 + 0x58));
        }
        puVar7 = (undefined4 *)
                 MeshData_Init((void *)(unaff_EBP + -0xbc),pvVar4,*(undefined4 *)(unaff_EBP + -0x10)
                               ,*(undefined4 *)(unaff_EBP + -0x24));
        puVar11 = (undefined4 *)(unaff_EBP + -0x8c);
        for (iVar10 = 0xc; iVar10 != 0; iVar10 = iVar10 + -1) {
          *puVar11 = *puVar7;
          puVar7 = puVar7 + 1;
          puVar11 = puVar11 + 1;
        }
        MeshData_FreeBuffersA(unaff_EBP + -0xbc);
        if (*(int *)(unaff_EBP + -0x2c) == 0) {
          iVar10 = D3DXMesh_StripifyOptimized();
        }
        else {
          iVar10 = D3DXMesh_Stripify(pvVar4,(void *)(unaff_EBP + -0x8c),*(int *)(unaff_EBP + 0xc),
                                     *(int *)(unaff_EBP + 0x10));
        }
      }
      _free(*(void **)(unaff_EBP + -0x10));
      goto LAB_00485ab7;
    }
  }
  iVar10 = -0x7789f794;
LAB_00485ab7:
  *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
  MeshData_FreeBuffersA(unaff_EBP + -0x8c);
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return iVar10;
}

