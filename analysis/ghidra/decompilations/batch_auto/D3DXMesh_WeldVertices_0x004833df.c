
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int D3DXMesh_WeldVertices(void)

{
  undefined4 uVar1;
  float fVar2;
  int *this;
  int *piVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  uint uVar7;
  void *pvVar8;
  uint uVar9;
  int iVar10;
  undefined4 *puVar11;
  undefined4 *puVar12;
  int *piVar13;
  int unaff_EBP;
  int iVar14;
  float fVar15;
  float *pfVar16;
  
  __security_init_cookie();
  this = *(int **)(unaff_EBP + 8);
  uVar7 = this[1];
  *(undefined4 *)(unaff_EBP + -0x14) = 0;
  *(undefined4 *)(unaff_EBP + -0x20) = 0;
  VertexDecl_Init((void *)(unaff_EBP + -0x9c),uVar7);
  uVar7 = this[0xc];
  *(undefined4 *)(unaff_EBP + -0x34) = 0;
  *(undefined4 *)(unaff_EBP + -0x44) = 0;
  *(undefined4 *)(unaff_EBP + -0x5c) = 0;
  *(undefined4 *)(unaff_EBP + -0x1c) = 0;
  *(undefined4 *)(unaff_EBP + -0x24) = 0;
  *(undefined4 *)(unaff_EBP + -0x28) = 0;
  *(undefined4 *)(unaff_EBP + -0x38) = 0;
  *(undefined4 *)(unaff_EBP + -0x30) = 0;
  *(uint *)(unaff_EBP + -0x3c) = uVar7 / 3;
  if ((*(int *)(unaff_EBP + 0x10) == 0) || ((*(byte *)((int)this + 9) & 4) != 0)) {
    iVar14 = -0x7789f794;
    goto LAB_004837ec;
  }
  uVar7 = uVar7 / 3 << 2;
  *(uint *)(unaff_EBP + 8) = uVar7;
  pvVar8 = operator_new(uVar7);
  iVar14 = this[0xc];
  *(void **)(unaff_EBP + -0x34) = pvVar8;
  pvVar8 = operator_new(iVar14 * 0x14);
  *(void **)(unaff_EBP + -0x48) = pvVar8;
  *(undefined4 *)(unaff_EBP + -4) = 0;
  if (pvVar8 == (void *)0x0) {
    *(undefined4 *)(unaff_EBP + -0x10) = 0;
  }
  else {
    RepeatCall(pvVar8,0x14,iVar14,&LAB_0047d949);
    *(void **)(unaff_EBP + -0x10) = pvVar8;
  }
  iVar14 = this[0xc];
  iVar10 = *(int *)(unaff_EBP + -0x10);
  *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
  *(int *)(unaff_EBP + -0x44) = iVar10;
  pvVar8 = operator_new(iVar14 << 2);
  *(void **)(unaff_EBP + -0x1c) = pvVar8;
  pvVar8 = operator_new(this[0x16] * 0xc);
  *(void **)(unaff_EBP + -0x24) = pvVar8;
  pvVar8 = operator_new(this[0xc] << 2);
  *(void **)(unaff_EBP + -0x14) = pvVar8;
  if ((((*(undefined4 **)(unaff_EBP + -0x34) == (undefined4 *)0x0) || (iVar10 == 0)) ||
      (*(int *)(unaff_EBP + -0x1c) == 0)) ||
     ((*(int *)(unaff_EBP + -0x24) == 0 || (pvVar8 == (void *)0x0)))) {
LAB_004837e5:
    iVar14 = -0x7ff8fff2;
  }
  else {
    uVar7 = *(uint *)(unaff_EBP + 8);
    puVar11 = *(undefined4 **)(unaff_EBP + -0x34);
    for (uVar9 = uVar7 >> 2; uVar9 != 0; uVar9 = uVar9 - 1) {
      *puVar11 = 0;
      puVar11 = puVar11 + 1;
    }
    for (uVar7 = uVar7 & 3; uVar7 != 0; uVar7 = uVar7 - 1) {
      *(undefined1 *)puVar11 = 0;
      puVar11 = (undefined4 *)((int)puVar11 + 1);
    }
    puVar11 = *(undefined4 **)(unaff_EBP + -0x1c);
    for (uVar7 = this[0xc] & 0x3fffffff; uVar7 != 0; uVar7 = uVar7 - 1) {
      *puVar11 = 0xffffffff;
      puVar11 = puVar11 + 1;
    }
    for (iVar14 = 0; iVar14 != 0; iVar14 = iVar14 + -1) {
      *(undefined1 *)puVar11 = 0xff;
      puVar11 = (undefined4 *)((int)puVar11 + 1);
    }
    iVar14 = Graphics_DrawIndexedPrimitive(this,unaff_EBP + -0x28,0x10);
    iVar10 = 0;
    if (iVar14 < 0) goto LAB_004837ec;
    iVar14 = this[0x16];
    *(undefined4 *)(unaff_EBP + -0x18) = 0;
    if (iVar14 != 0) {
      *(undefined4 *)(unaff_EBP + 8) = 0;
      *(undefined4 *)(unaff_EBP + -0x2c) = 0;
      do {
        *(int *)(unaff_EBP + -0x40) = *(int *)(unaff_EBP + -0x28) + iVar10;
        uVar7 = 0;
        puVar11 = (undefined4 *)(iVar10 + *(int *)(unaff_EBP + -0x24));
        do {
          iVar14 = *(int *)(unaff_EBP + -0x1c);
          piVar13 = (int *)(*(int *)(unaff_EBP + -0x40) + uVar7 * 4);
          *puVar11 = *(undefined4 *)(iVar14 + *piVar13 * 4);
          iVar10 = *(int *)(unaff_EBP + 8) + uVar7;
          uVar7 = uVar7 + 1;
          puVar11 = puVar11 + 1;
          *(int *)(iVar14 + *piVar13 * 4) = iVar10;
        } while (uVar7 < 3);
        *(int *)(unaff_EBP + -0x18) = *(int *)(unaff_EBP + -0x18) + 1;
        *(int *)(unaff_EBP + 8) = *(int *)(unaff_EBP + 8) + 3;
        iVar10 = *(int *)(unaff_EBP + -0x2c) + 0xc;
        uVar7 = this[0x16];
        *(int *)(unaff_EBP + -0x2c) = iVar10;
      } while (*(uint *)(unaff_EBP + -0x18) < uVar7);
    }
    iVar14 = Graphics_DrawIndexedPrimitiveUP(this,unaff_EBP + -0x20,0x10);
    if (iVar14 < 0) goto LAB_004837ec;
    if (*(float *)(unaff_EBP + 0xc) == _DAT_004cf368) {
      pfVar16 = *(float **)(unaff_EBP + -0x20);
      fVar15 = 0.0;
      if (this[0xc] != 0) {
        do {
          iVar14 = Mesh_FindWeldVertex(pfVar16,fVar15,*(int *)(unaff_EBP + -0x34),
                                       *(int *)(unaff_EBP + -0x10),*(uint *)(unaff_EBP + -0x3c),
                                       *(int *)(unaff_EBP + -0x1c),*(int *)(unaff_EBP + -0x24),
                                       *(int *)(unaff_EBP + -0x28),(int *)(unaff_EBP + -0x5c),
                                       (float *)(unaff_EBP + -0x58));
          if (iVar14 == 0) {
            *(float *)(*(int *)(unaff_EBP + -0x14) + (int)fVar15 * 4) = fVar15;
          }
          else {
            *(undefined4 *)(*(int *)(unaff_EBP + -0x14) + (int)fVar15 * 4) =
                 *(undefined4 *)(unaff_EBP + -0x58);
          }
          pfVar16 = (float *)((int)pfVar16 + *(int *)(unaff_EBP + -0x98));
          fVar15 = (float)((int)fVar15 + 1);
        } while ((uint)fVar15 < (uint)this[0xc]);
      }
    }
    else {
      pvVar8 = operator_new(this[0xc] << 2);
      iVar14 = this[0xc];
      *(void **)(unaff_EBP + -0x38) = pvVar8;
      puVar11 = operator_new(iVar14 << 2);
      uVar7 = 0;
      *(undefined4 **)(unaff_EBP + -0x30) = puVar11;
      if ((pvVar8 == (void *)0x0) || (puVar11 == (undefined4 *)0x0)) goto LAB_004837e5;
      puVar12 = *(undefined4 **)(unaff_EBP + -0x20);
      if (this[0xc] != 0) {
        do {
          uVar1 = *puVar12;
          puVar12 = (undefined4 *)((int)puVar12 + *(int *)(unaff_EBP + -0x98));
          *(undefined4 *)((int)pvVar8 + uVar7 * 4) = uVar1;
          uVar7 = uVar7 + 1;
        } while (uVar7 < (uint)this[0xc]);
      }
      Sort_IndexHeapByFloat(puVar11,(int)pvVar8,this[0xc]);
      fVar15 = *(float *)(unaff_EBP + 0xc);
      uVar7 = this[0xc];
      fVar2 = *(float *)(unaff_EBP + 0xc);
      *(undefined4 *)(unaff_EBP + 8) = 0;
      puVar12 = *(undefined4 **)(unaff_EBP + -0x14);
      for (uVar7 = uVar7 & 0x3fffffff; uVar7 != 0; uVar7 = uVar7 - 1) {
        *puVar12 = 0xffffffff;
        puVar12 = puVar12 + 1;
      }
      for (iVar14 = 0; iVar14 != 0; iVar14 = iVar14 + -1) {
        *(undefined1 *)puVar12 = 0xff;
        puVar12 = (undefined4 *)((int)puVar12 + 1);
      }
      uVar7 = this[0xc];
      if (uVar7 != 0) {
        *(undefined4 *)(unaff_EBP + -0x18) = 1;
        *(undefined4 **)(unaff_EBP + -0x10) = puVar11;
        do {
          if (*(uint *)(unaff_EBP + 8) < uVar7) {
            iVar14 = **(int **)(unaff_EBP + -0x10);
            do {
              fVar4 = *(float *)(*(int *)(unaff_EBP + -0x38) + iVar14 * 4) -
                      *(float *)(*(int *)(unaff_EBP + -0x38) + puVar11[*(int *)(unaff_EBP + 8)] * 4)
              ;
              if (fVar4 < *(float *)(unaff_EBP + 0xc) == (fVar4 == *(float *)(unaff_EBP + 0xc)))
              break;
              *(int *)(unaff_EBP + 8) = *(int *)(unaff_EBP + 8) + 1;
            } while (*(uint *)(unaff_EBP + 8) < uVar7);
          }
          piVar3 = *(int **)(unaff_EBP + -0x10);
          iVar14 = *piVar3;
          piVar13 = (int *)(*(int *)(unaff_EBP + -0x14) + iVar14 * 4);
          if (*piVar13 == -1) {
            *piVar13 = iVar14;
            pfVar16 = (float *)(*piVar3 * *(int *)(unaff_EBP + -0x98) + *(int *)(unaff_EBP + -0x20))
            ;
            uVar7 = *(uint *)(unaff_EBP + -0x18);
            while (*(uint *)(unaff_EBP + -0x2c) = uVar7, uVar7 < *(uint *)(unaff_EBP + 8)) {
              iVar14 = puVar11[uVar7];
              piVar13 = (int *)(*(int *)(unaff_EBP + -0x14) + iVar14 * 4);
              iVar10 = *piVar13;
              *(int *)(unaff_EBP + -0x3c) = iVar14;
              *(int **)(unaff_EBP + -0x40) = piVar13;
              if (iVar10 == -1) {
                puVar11 = (undefined4 *)
                          (iVar14 * *(int *)(unaff_EBP + -0x98) + *(int *)(unaff_EBP + -0x20));
                *(undefined4 *)(unaff_EBP + -0x54) = *puVar11;
                *(undefined4 *)(unaff_EBP + -0x50) = puVar11[1];
                *(undefined4 *)(unaff_EBP + -0x4c) = puVar11[2];
                fVar6 = *(float *)(unaff_EBP + -0x54) - *pfVar16;
                *(float *)(unaff_EBP + -0x54) = fVar6;
                fVar5 = *(float *)(unaff_EBP + -0x50) - pfVar16[1];
                *(float *)(unaff_EBP + -0x50) = fVar5;
                fVar4 = *(float *)(unaff_EBP + -0x4c) - pfVar16[2];
                *(float *)(unaff_EBP + -0x4c) = fVar4;
                if (fVar6 * fVar6 + fVar5 * fVar5 + fVar4 * fVar4 < fVar15 * fVar2) {
                  iVar14 = **(int **)(unaff_EBP + -0x10);
                  uVar7 = *(uint *)(*(int *)(unaff_EBP + -0x1c) + iVar14 * 4);
                  *(int *)(unaff_EBP + -0x48) = iVar14;
                  if (uVar7 != 0xffffffff) {
                    do {
                      piVar13 = (int *)(*(int *)(unaff_EBP + -0x28) + (uVar7 / 3) * 0xc);
                      iVar14 = *(int *)(unaff_EBP + -0x3c);
                      if (((*piVar13 == iVar14) || (piVar13[1] == iVar14)) || (piVar13[2] == iVar14)
                         ) goto LAB_004837aa;
                      uVar7 = *(uint *)(*(int *)(unaff_EBP + -0x24) + uVar7 * 4);
                    } while (uVar7 != 0xffffffff);
                    piVar13 = *(int **)(unaff_EBP + -0x40);
                    iVar14 = *(int *)(unaff_EBP + -0x48);
                  }
                  *piVar13 = iVar14;
                }
LAB_004837aa:
                puVar11 = *(undefined4 **)(unaff_EBP + -0x30);
              }
              uVar7 = *(int *)(unaff_EBP + -0x2c) + 1;
            }
          }
          *(int *)(unaff_EBP + -0x18) = *(int *)(unaff_EBP + -0x18) + 1;
          *(int *)(unaff_EBP + -0x10) = *(int *)(unaff_EBP + -0x10) + 4;
          uVar7 = this[0xc];
        } while (*(int *)(unaff_EBP + -0x18) - 1U < uVar7);
      }
    }
    iVar14 = (**(code **)(*this + 0x4c))
                       (this,*(undefined4 *)(unaff_EBP + -0x14),*(undefined4 *)(unaff_EBP + 0x10));
  }
LAB_004837ec:
  _free(*(void **)(unaff_EBP + -0x34));
  _free(*(void **)(unaff_EBP + -0x44));
  _free(*(void **)(unaff_EBP + -0x1c));
  _free(*(void **)(unaff_EBP + -0x24));
  _free(*(void **)(unaff_EBP + -0x38));
  _free(*(void **)(unaff_EBP + -0x30));
  _free(*(void **)(unaff_EBP + -0x14));
  if (*(int *)(unaff_EBP + -0x20) != 0) {
    (**(code **)(*(int *)this[10] + 0x30))((int *)this[10]);
  }
  if (*(int *)(unaff_EBP + -0x28) != 0) {
    (**(code **)(*(int *)this[0xf] + 0x30))((int *)this[0xf]);
  }
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return iVar14;
}

