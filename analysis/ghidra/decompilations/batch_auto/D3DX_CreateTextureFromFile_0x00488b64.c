
uint D3DX_CreateTextureFromFile(void)

{
  byte bVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  uint uVar5;
  int *piVar6;
  uint uVar7;
  int iVar8;
  int unaff_EBP;
  undefined4 *puVar9;
  uint uVar10;
  int *piVar11;
  undefined4 *puVar12;
  
  __security_init_cookie();
  piVar6 = *(int **)(unaff_EBP + 0x3c);
  WebClient_InitResponse((undefined4 *)(unaff_EBP + -0x8c));
  *(undefined4 *)(unaff_EBP + -4) = 0;
  *(undefined4 *)(unaff_EBP + -0x18) = 0;
  *(undefined4 *)(unaff_EBP + -0x14) = 0;
  *(undefined4 *)(unaff_EBP + -0x1c) = 0;
  *(undefined4 *)(unaff_EBP + -0x20) = 0;
  if ((((*(int *)(unaff_EBP + 8) == 0) || (*(int *)(unaff_EBP + 0xc) == 0)) ||
      (*(int *)(unaff_EBP + 0x10) == 0)) || (*(int *)(unaff_EBP + 0x48) == 0)) {
    uVar10 = 0x8876086c;
    goto LAB_00489189;
  }
  if ((piVar6 == (int *)0x0) && (*(int *)(unaff_EBP + 0x44) == -1)) {
    piVar6 = (int *)(unaff_EBP + -0xcc);
  }
  uVar10 = D3DX_LoadTextureFromMemory
                     ((void *)(unaff_EBP + -0x8c),*(uint **)(unaff_EBP + 0xc),
                      *(int **)(unaff_EBP + 0x10),piVar6,1);
  if ((int)uVar10 < 0) goto LAB_00489189;
  if (*(int *)(unaff_EBP + 0x44) == -1) {
    *(int *)(unaff_EBP + 0x44) = piVar6[5];
  }
  iVar8 = *(int *)(unaff_EBP + -0x40);
  *(undefined4 *)(unaff_EBP + -0x24) = 1;
  for (; iVar8 != 0; iVar8 = *(int *)(iVar8 + 0x4c)) {
    *(int *)(unaff_EBP + -0x24) = *(int *)(unaff_EBP + -0x24) + 1;
  }
  *(undefined4 *)(unaff_EBP + -0x28) = 1;
  if (*(int *)(unaff_EBP + 0x44) == 5) {
    iVar8 = *(int *)(unaff_EBP + -0x3c);
    if (iVar8 != 0) {
      do {
        iVar8 = *(int *)(iVar8 + 0x50);
        *(int *)(unaff_EBP + -0x28) = *(int *)(unaff_EBP + -0x28) + 1;
      } while (iVar8 != 0);
      if (*(int *)(unaff_EBP + -0x28) == 6) goto LAB_00488c31;
    }
    uVar10 = 0x80004005;
    goto LAB_00489189;
  }
LAB_00488c31:
  if ((*(int *)(unaff_EBP + 0x14) == 0) || (*(int *)(unaff_EBP + 0x14) == -1)) {
    *(undefined4 *)(unaff_EBP + 0x14) = *(undefined4 *)(unaff_EBP + -0x80);
  }
  if ((*(int *)(unaff_EBP + 0x18) == 0) || (*(int *)(unaff_EBP + 0x18) == -1)) {
    *(undefined4 *)(unaff_EBP + 0x18) = *(undefined4 *)(unaff_EBP + -0x7c);
  }
  if ((*(int *)(unaff_EBP + 0x1c) == 0) || (*(int *)(unaff_EBP + 0x1c) == -1)) {
    *(undefined4 *)(unaff_EBP + 0x1c) = *(undefined4 *)(unaff_EBP + -0x78);
  }
  if (*(int *)(unaff_EBP + 0x30) == -1) {
    *(undefined4 *)(unaff_EBP + 0x30) = 0x80004;
  }
  if (*(int *)(unaff_EBP + 0x34) == -1) {
    *(undefined4 *)(unaff_EBP + 0x34) = 5;
  }
  if (*(int *)(unaff_EBP + 0x44) == 5) {
    *(uint *)(unaff_EBP + 0x30) = *(uint *)(unaff_EBP + 0x30) | 0x70000;
    *(uint *)(unaff_EBP + 0x34) = *(uint *)(unaff_EBP + 0x34) | 0x70000;
  }
  if (((*(char *)(unaff_EBP + 0x30) == '\x01') ||
      (uVar10 = *(uint *)(unaff_EBP + 0x34) & 0xff, uVar10 == 2)) ||
     (*(undefined4 *)(unaff_EBP + -0x30) = 0, uVar10 == 5)) {
    *(undefined4 *)(unaff_EBP + -0x30) = 1;
  }
  if (*(int *)(unaff_EBP + 0x28) == 0) {
    iVar8 = *(int *)(unaff_EBP + -0x8c);
    if (*(int *)(unaff_EBP + 0x38) != 0) {
      piVar6 = D3D_FindDisplayMode(iVar8);
      iVar2 = piVar6[1];
      if ((((iVar2 == 0) || (iVar2 == 1)) || (iVar2 == 2)) && (piVar6[4] == 0)) {
        uVar3 = *(undefined4 *)(unaff_EBP + 0x44);
        uVar4 = *(undefined4 *)(unaff_EBP + 0x24);
        piVar11 = (int *)(unaff_EBP + -0xb0);
        for (iVar8 = 9; iVar8 != 0; iVar8 = iVar8 + -1) {
          *piVar11 = *piVar6;
          piVar6 = piVar6 + 1;
          piVar11 = piVar11 + 1;
        }
        *(undefined4 *)(unaff_EBP + -0xb0) = 0;
        *(undefined4 *)(unaff_EBP + -0xa0) = 1;
        iVar8 = D3D_SelectBestAdapterMode((int *)0x0,uVar4,uVar3,(int *)(unaff_EBP + -0xb0));
        if (iVar8 == 0) {
          iVar8 = *(int *)(unaff_EBP + -0x8c);
        }
      }
    }
    iVar8 = D3D_ConvertFourCC(iVar8);
    *(int *)(unaff_EBP + 0x28) = iVar8;
    if ((*(int *)(unaff_EBP + 0x2c) != 3) && (iVar8 == 0x14)) {
      *(undefined4 *)(unaff_EBP + 0x28) = 0x16;
    }
  }
  iVar8 = *(int *)(unaff_EBP + -0x84);
  if (iVar8 == 0) {
    puVar9 = (undefined4 *)(unaff_EBP + -0x4cc);
    for (iVar8 = 0x100; uVar10 = 0, iVar8 != 0; iVar8 = iVar8 + -1) {
      *puVar9 = 0xffffffff;
      puVar9 = puVar9 + 1;
    }
  }
  else {
    uVar7 = *(uint *)(unaff_EBP + 0x38);
    bVar1 = *(byte *)(unaff_EBP + 0x3a);
    uVar10 = 0;
    do {
      uVar5 = *(uint *)(iVar8 + uVar10 * 4);
      *(uint *)(unaff_EBP + -0x4cc + uVar10 * 4) =
           -(uint)(uVar5 != ((uint)bVar1 | (uVar7 & 0xff) << 0x10 | uVar7 & 0xff00ff00)) & uVar5;
      uVar10 = uVar10 + 1;
    } while (uVar10 < 0x100);
    *(undefined4 *)(unaff_EBP + 0x38) = 0;
  }
  if (*(undefined4 **)(unaff_EBP + 0x40) == (undefined4 *)0x0) {
    if (*(int *)(unaff_EBP + 0x28) == 0x28) {
LAB_00488dd7:
      *(undefined4 *)(unaff_EBP + 0x28) = 0x15;
    }
    else if (*(int *)(unaff_EBP + 0x28) == 0x29) {
      *(undefined4 *)(unaff_EBP + 0x28) = 0x16;
      uVar7 = 0;
      do {
        if (*(char *)(unaff_EBP + -0x4c9 + uVar7 * 4) != -1) goto LAB_00488dd7;
        uVar7 = uVar7 + 1;
      } while (uVar7 < 0x100);
    }
  }
  else {
    puVar9 = (undefined4 *)(unaff_EBP + -0x4cc);
    puVar12 = *(undefined4 **)(unaff_EBP + 0x40);
    for (iVar8 = 0x100; iVar8 != 0; iVar8 = iVar8 + -1) {
      *puVar12 = *puVar9;
      puVar9 = puVar9 + 1;
      puVar12 = puVar12 + 1;
    }
    uVar10 = 0;
  }
  if ((*(int *)(unaff_EBP + 0x2c) != 0) ||
     (*(undefined4 *)(unaff_EBP + -0x34) = 1, (*(byte *)(unaff_EBP + 0x25) & 2) != 0)) {
    *(undefined4 *)(unaff_EBP + -0x34) = 0;
  }
  uVar10 = D3D_ComputeTextureDimensions
                     (uVar10,*(uint *)(unaff_EBP + 0x24),*(int **)(unaff_EBP + 8),
                      (uint *)(unaff_EBP + 0x14),(uint *)(unaff_EBP + 0x18),
                      (uint *)(unaff_EBP + 0x1c),(uint *)(unaff_EBP + 0x20),
                      (int *)(unaff_EBP + 0x28),*(int *)(unaff_EBP + 0x2c),
                      *(int *)(unaff_EBP + 0x44));
  if (-1 < (int)uVar10) {
    iVar8 = *(int *)(unaff_EBP + 0x44);
    uVar3 = *(undefined4 *)(unaff_EBP + 0x28);
    uVar4 = *(undefined4 *)(unaff_EBP + 0x14);
    if (iVar8 == 3) {
      uVar10 = (**(code **)(**(int **)(unaff_EBP + 8) + 0x50))
                         (*(int **)(unaff_EBP + 8),uVar4,*(undefined4 *)(unaff_EBP + 0x18),
                          *(undefined4 *)(unaff_EBP + 0x20),*(undefined4 *)(unaff_EBP + 0x24),uVar3,
                          *(undefined4 *)(unaff_EBP + 0x2c),unaff_EBP + -0x1c);
    }
    else if (iVar8 == 4) {
      uVar10 = (**(code **)(**(int **)(unaff_EBP + 8) + 0x54))
                         (*(int **)(unaff_EBP + 8),uVar4,*(undefined4 *)(unaff_EBP + 0x18),
                          *(undefined4 *)(unaff_EBP + 0x1c),*(undefined4 *)(unaff_EBP + 0x20),
                          *(undefined4 *)(unaff_EBP + 0x24),uVar3,*(undefined4 *)(unaff_EBP + 0x2c),
                          unaff_EBP + -0x1c);
    }
    else if (iVar8 == 5) {
      uVar10 = (**(code **)(**(int **)(unaff_EBP + 8) + 0x58))
                         (*(int **)(unaff_EBP + 8),uVar4,*(undefined4 *)(unaff_EBP + 0x20),
                          *(undefined4 *)(unaff_EBP + 0x24),uVar3,*(undefined4 *)(unaff_EBP + 0x2c),
                          unaff_EBP + -0x1c);
    }
    if (-1 < (int)uVar10) {
      if (*(int *)(unaff_EBP + -0x34) == 0) {
        piVar6 = *(int **)(unaff_EBP + -0x1c);
      }
      else {
        iVar8 = *(int *)(unaff_EBP + 0x44);
        if (iVar8 == 3) {
          uVar10 = (**(code **)(**(int **)(unaff_EBP + 8) + 0x50))
                             (*(int **)(unaff_EBP + 8),uVar4,*(undefined4 *)(unaff_EBP + 0x18),
                              *(undefined4 *)(unaff_EBP + 0x20),0,uVar3,2,unaff_EBP + -0x20);
        }
        else if (iVar8 == 4) {
          uVar10 = (**(code **)(**(int **)(unaff_EBP + 8) + 0x54))
                             (*(int **)(unaff_EBP + 8),uVar4,*(undefined4 *)(unaff_EBP + 0x18),
                              *(undefined4 *)(unaff_EBP + 0x1c),*(undefined4 *)(unaff_EBP + 0x20),0,
                              uVar3,2,unaff_EBP + -0x20);
        }
        else if (iVar8 == 5) {
          uVar10 = (**(code **)(**(int **)(unaff_EBP + 8) + 0x58))
                             (*(int **)(unaff_EBP + 8),uVar4,*(undefined4 *)(unaff_EBP + 0x20),0,
                              uVar3,2,unaff_EBP + -0x20);
        }
        if ((int)uVar10 < 0) goto LAB_00489147;
        piVar6 = *(int **)(unaff_EBP + -0x20);
      }
      *(undefined4 *)(unaff_EBP + -0x2c) = 0;
      puVar9 = (undefined4 *)(unaff_EBP + -0x8c);
      *(undefined4 **)(unaff_EBP + -0x38) = puVar9;
      if (*(int *)(unaff_EBP + -0x28) != 0) {
        do {
          *(undefined4 *)(unaff_EBP + -0x10) = 0;
          if (*(int *)(unaff_EBP + -0x24) != 0) {
            while (*(uint *)(unaff_EBP + -0x10) < *(uint *)(unaff_EBP + 0x20)) {
              iVar8 = *(int *)(unaff_EBP + 0x44);
              if (iVar8 == 3) {
                iVar8 = unaff_EBP + -0x14;
LAB_00488f66:
                uVar10 = (**(code **)(*piVar6 + 0x3c))
                                   (piVar6,*(undefined4 *)(unaff_EBP + -0x10),iVar8);
              }
              else {
                if (iVar8 == 4) {
                  iVar8 = unaff_EBP + -0x18;
                  goto LAB_00488f66;
                }
                if (iVar8 == 5) {
                  uVar10 = (**(code **)(*piVar6 + 0x3c))
                                     (piVar6,*(undefined4 *)(unaff_EBP + -0x2c),
                                      *(undefined4 *)(unaff_EBP + -0x10),unaff_EBP + -0x14);
                }
              }
              if ((int)uVar10 < 0) goto LAB_00489147;
              iVar8 = *(int *)(unaff_EBP + 0x44);
              if (iVar8 == 3) {
LAB_00488f88:
                uVar10 = D3DX_CreateTextureFromResource
                                   (*(int **)(unaff_EBP + -0x14),*(undefined4 *)(unaff_EBP + 0x40),
                                    (uint *)0x0,puVar9[1],*puVar9,puVar9[0xc],unaff_EBP + -0x4cc,
                                    puVar9 + 6,*(uint *)(unaff_EBP + 0x30),
                                    *(undefined4 *)(unaff_EBP + 0x38));
              }
              else if (iVar8 == 4) {
                uVar10 = D3DX_CreateTextureFromMemory
                                   (*(int **)(unaff_EBP + -0x18),*(undefined4 *)(unaff_EBP + 0x40),
                                    (uint *)0x0,puVar9[1],*puVar9,puVar9[0xc],puVar9[0xd],
                                    unaff_EBP + -0x4cc,puVar9 + 6,*(uint *)(unaff_EBP + 0x30),
                                    *(undefined4 *)(unaff_EBP + 0x38));
              }
              else if (iVar8 == 5) goto LAB_00488f88;
              if ((int)uVar10 < 0) goto LAB_00489147;
              piVar11 = *(int **)(unaff_EBP + -0x18);
              if (piVar11 != (int *)0x0) {
                (**(code **)(*piVar11 + 8))(piVar11);
                *(undefined4 *)(unaff_EBP + -0x18) = 0;
              }
              piVar11 = *(int **)(unaff_EBP + -0x14);
              if (piVar11 != (int *)0x0) {
                (**(code **)(*piVar11 + 8))(piVar11);
                *(undefined4 *)(unaff_EBP + -0x14) = 0;
              }
              *(int *)(unaff_EBP + -0x10) = *(int *)(unaff_EBP + -0x10) + 1;
              if (*(uint *)(unaff_EBP + -0x24) <= *(uint *)(unaff_EBP + -0x10)) break;
              puVar9 = (undefined4 *)puVar9[0x13];
            }
          }
          if ((*(int *)(unaff_EBP + -0x30) == 0) &&
             (*(uint *)(unaff_EBP + -0x10) < *(uint *)(unaff_EBP + 0x20))) {
            do {
              iVar8 = *(int *)(unaff_EBP + 0x44);
              if (iVar8 == 3) {
                iVar8 = unaff_EBP + -0x14;
LAB_00489056:
                uVar10 = (**(code **)(*piVar6 + 0x3c))
                                   (piVar6,*(undefined4 *)(unaff_EBP + -0x10),iVar8);
              }
              else {
                if (iVar8 == 4) {
                  iVar8 = unaff_EBP + -0x18;
                  goto LAB_00489056;
                }
                if (iVar8 == 5) {
                  uVar10 = (**(code **)(*piVar6 + 0x3c))
                                     (piVar6,*(undefined4 *)(unaff_EBP + -0x2c),
                                      *(undefined4 *)(unaff_EBP + -0x10),unaff_EBP + -0x14);
                }
              }
              if ((int)uVar10 < 0) goto LAB_00489147;
              iVar8 = *(int *)(unaff_EBP + 0x44);
              if (iVar8 == 3) {
LAB_0048907c:
                uVar10 = D3DX_CreateTextureFromResource
                                   (*(int **)(unaff_EBP + -0x14),*(undefined4 *)(unaff_EBP + 0x40),
                                    (uint *)0x0,puVar9[1],*puVar9,puVar9[0xc],unaff_EBP + -0x4cc,
                                    puVar9 + 6,*(uint *)(unaff_EBP + 0x30),
                                    *(undefined4 *)(unaff_EBP + 0x38));
              }
              else if (iVar8 == 4) {
                uVar10 = D3DX_CreateTextureFromMemory
                                   (*(int **)(unaff_EBP + -0x18),*(undefined4 *)(unaff_EBP + 0x40),
                                    (uint *)0x0,puVar9[1],*puVar9,puVar9[0xc],puVar9[0xd],
                                    unaff_EBP + -0x4cc,puVar9 + 6,*(uint *)(unaff_EBP + 0x30),
                                    *(undefined4 *)(unaff_EBP + 0x38));
              }
              else if (iVar8 == 5) goto LAB_0048907c;
              if ((int)uVar10 < 0) goto LAB_00489147;
              piVar11 = *(int **)(unaff_EBP + -0x18);
              if (piVar11 != (int *)0x0) {
                (**(code **)(*piVar11 + 8))(piVar11);
                *(undefined4 *)(unaff_EBP + -0x18) = 0;
              }
              piVar11 = *(int **)(unaff_EBP + -0x14);
              if (piVar11 != (int *)0x0) {
                (**(code **)(*piVar11 + 8))(piVar11);
                *(undefined4 *)(unaff_EBP + -0x14) = 0;
              }
              *(int *)(unaff_EBP + -0x10) = *(int *)(unaff_EBP + -0x10) + 1;
            } while (*(uint *)(unaff_EBP + -0x10) < *(uint *)(unaff_EBP + 0x20));
          }
          *(int *)(unaff_EBP + -0x2c) = *(int *)(unaff_EBP + -0x2c) + 1;
          puVar9 = *(undefined4 **)(*(int *)(unaff_EBP + -0x38) + 0x50);
          *(undefined4 **)(unaff_EBP + -0x38) = puVar9;
        } while (*(uint *)(unaff_EBP + -0x2c) < *(uint *)(unaff_EBP + -0x28));
      }
      if ((((*(int *)(unaff_EBP + -0x30) == 0) ||
           (*(uint *)(unaff_EBP + 0x20) <= *(uint *)(unaff_EBP + -0x24))) ||
          (uVar10 = D3DX_FilterTextureLevels
                              (piVar6,(int *)(unaff_EBP + -0x4cc),*(int *)(unaff_EBP + -0x24) - 1,
                               *(uint *)(unaff_EBP + 0x34)), -1 < (int)uVar10)) &&
         ((*(int *)(unaff_EBP + -0x34) == 0 ||
          (uVar10 = (**(code **)(**(int **)(unaff_EBP + 8) + 0x74))
                              (*(int **)(unaff_EBP + 8),*(undefined4 *)(unaff_EBP + -0x20),
                               *(undefined4 *)(unaff_EBP + -0x1c)), -1 < (int)uVar10)))) {
        uVar3 = *(undefined4 *)(unaff_EBP + -0x1c);
        *(undefined4 *)(unaff_EBP + -0x1c) = 0;
        **(undefined4 **)(unaff_EBP + 0x48) = uVar3;
        uVar10 = 0;
      }
    }
  }
LAB_00489147:
  piVar6 = *(int **)(unaff_EBP + -0x18);
  if (piVar6 != (int *)0x0) {
    (**(code **)(*piVar6 + 8))(piVar6);
    *(undefined4 *)(unaff_EBP + -0x18) = 0;
  }
  piVar6 = *(int **)(unaff_EBP + -0x14);
  if (piVar6 != (int *)0x0) {
    (**(code **)(*piVar6 + 8))(piVar6);
    *(undefined4 *)(unaff_EBP + -0x14) = 0;
  }
  piVar6 = *(int **)(unaff_EBP + -0x1c);
  if (piVar6 != (int *)0x0) {
    (**(code **)(*piVar6 + 8))(piVar6);
    *(undefined4 *)(unaff_EBP + -0x1c) = 0;
  }
  piVar6 = *(int **)(unaff_EBP + -0x20);
  if (piVar6 != (int *)0x0) {
    (**(code **)(*piVar6 + 8))(piVar6);
    *(undefined4 *)(unaff_EBP + -0x20) = 0;
  }
LAB_00489189:
  *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
  TextureCache_RecursiveFree(unaff_EBP + -0x8c);
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return uVar10;
}

