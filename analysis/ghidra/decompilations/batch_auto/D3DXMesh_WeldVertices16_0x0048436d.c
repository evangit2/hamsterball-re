
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int D3DXMesh_WeldVertices16(void)

{
  ushort *puVar1;
  undefined4 uVar2;
  float fVar3;
  int *this;
  int *piVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  uint uVar8;
  void *pvVar9;
  undefined4 *puVar10;
  uint uVar11;
  undefined4 *puVar12;
  int *piVar13;
  int unaff_EBP;
  int iVar14;
  float fVar15;
  int iVar16;
  float *pfVar17;
  
  __security_init_cookie();
  this = *(int **)(unaff_EBP + 8);
  uVar8 = this[1];
  *(undefined4 *)(unaff_EBP + -0x14) = 0;
  *(undefined4 *)(unaff_EBP + -0x24) = 0;
  VertexDecl_Init((void *)(unaff_EBP + -0x9c),uVar8);
  uVar8 = this[0xc];
  *(undefined4 *)(unaff_EBP + -0x34) = 0;
  *(undefined4 *)(unaff_EBP + -0x44) = 0;
  *(undefined4 *)(unaff_EBP + -0x58) = 0;
  *(undefined4 *)(unaff_EBP + -0x20) = 0;
  *(undefined4 *)(unaff_EBP + -0x28) = 0;
  *(undefined4 *)(unaff_EBP + -0x2c) = 0;
  *(undefined4 *)(unaff_EBP + -0x3c) = 0;
  *(undefined4 *)(unaff_EBP + -0x38) = 0;
  *(uint *)(unaff_EBP + -0x40) = uVar8 / 3;
  if ((*(int *)(unaff_EBP + 0x10) == 0) || ((*(byte *)((int)this + 9) & 4) != 0)) {
    iVar14 = -0x7789f794;
    goto LAB_00484781;
  }
  uVar8 = uVar8 / 3 << 2;
  *(uint *)(unaff_EBP + 8) = uVar8;
  pvVar9 = operator_new(uVar8);
  iVar14 = this[0xc];
  *(void **)(unaff_EBP + -0x34) = pvVar9;
  pvVar9 = operator_new(iVar14 * 0x14);
  *(void **)(unaff_EBP + -0x48) = pvVar9;
  *(undefined4 *)(unaff_EBP + -4) = 0;
  if (pvVar9 == (void *)0x0) {
    *(undefined4 *)(unaff_EBP + -0x1c) = 0;
  }
  else {
    RepeatCall(pvVar9,0x14,iVar14,&LAB_0047d949);
    *(void **)(unaff_EBP + -0x1c) = pvVar9;
  }
  iVar14 = this[0xc];
  iVar16 = *(int *)(unaff_EBP + -0x1c);
  *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
  *(int *)(unaff_EBP + -0x44) = iVar16;
  pvVar9 = operator_new(iVar14 << 2);
  *(void **)(unaff_EBP + -0x20) = pvVar9;
  pvVar9 = operator_new(this[0x16] * 0xc);
  *(void **)(unaff_EBP + -0x28) = pvVar9;
  pvVar9 = operator_new(this[0xc] << 2);
  *(void **)(unaff_EBP + -0x14) = pvVar9;
  if ((((*(undefined4 **)(unaff_EBP + -0x34) == (undefined4 *)0x0) || (iVar16 == 0)) ||
      (*(int *)(unaff_EBP + -0x20) == 0)) ||
     ((*(int *)(unaff_EBP + -0x28) == 0 || (pvVar9 == (void *)0x0)))) {
LAB_0048477a:
    iVar14 = -0x7ff8fff2;
  }
  else {
    uVar8 = *(uint *)(unaff_EBP + 8);
    puVar10 = *(undefined4 **)(unaff_EBP + -0x34);
    for (uVar11 = uVar8 >> 2; uVar11 != 0; uVar11 = uVar11 - 1) {
      *puVar10 = 0;
      puVar10 = puVar10 + 1;
    }
    for (uVar8 = uVar8 & 3; uVar8 != 0; uVar8 = uVar8 - 1) {
      *(undefined1 *)puVar10 = 0;
      puVar10 = (undefined4 *)((int)puVar10 + 1);
    }
    puVar10 = *(undefined4 **)(unaff_EBP + -0x20);
    for (uVar8 = this[0xc] & 0x3fffffff; uVar8 != 0; uVar8 = uVar8 - 1) {
      *puVar10 = 0xffffffff;
      puVar10 = puVar10 + 1;
    }
    for (iVar14 = 0; iVar14 != 0; iVar14 = iVar14 + -1) {
      *(undefined1 *)puVar10 = 0xff;
      puVar10 = (undefined4 *)((int)puVar10 + 1);
    }
    iVar14 = Graphics_DrawIndexedPrimitive(this,unaff_EBP + -0x2c,0x10);
    if (iVar14 < 0) goto LAB_00484781;
    iVar14 = this[0x16];
    *(undefined4 *)(unaff_EBP + -0x10) = 0;
    if (iVar14 != 0) {
      puVar10 = *(undefined4 **)(unaff_EBP + -0x28);
      *(undefined4 *)(unaff_EBP + -0x18) = 0;
      *(undefined4 *)(unaff_EBP + 8) = 0;
      do {
        *(int *)(unaff_EBP + -0x30) = *(int *)(unaff_EBP + 8) + *(int *)(unaff_EBP + -0x2c);
        uVar8 = 0;
        do {
          iVar14 = *(int *)(unaff_EBP + -0x20);
          puVar1 = (ushort *)(*(int *)(unaff_EBP + -0x30) + uVar8 * 2);
          *puVar10 = *(undefined4 *)(iVar14 + (uint)*puVar1 * 4);
          iVar16 = *(int *)(unaff_EBP + -0x18) + uVar8;
          uVar8 = uVar8 + 1;
          puVar10 = puVar10 + 1;
          *(int *)(iVar14 + (uint)*puVar1 * 4) = iVar16;
        } while (uVar8 < 3);
        *(int *)(unaff_EBP + -0x10) = *(int *)(unaff_EBP + -0x10) + 1;
        *(int *)(unaff_EBP + 8) = *(int *)(unaff_EBP + 8) + 6;
        *(int *)(unaff_EBP + -0x18) = *(int *)(unaff_EBP + -0x18) + 3;
      } while (*(uint *)(unaff_EBP + -0x10) < (uint)this[0x16]);
    }
    iVar14 = Graphics_DrawIndexedPrimitiveUP(this,unaff_EBP + -0x24,0x10);
    if (iVar14 < 0) goto LAB_00484781;
    if (*(float *)(unaff_EBP + 0xc) == _DAT_004cf368) {
      pfVar17 = *(float **)(unaff_EBP + -0x24);
      fVar15 = 0.0;
      if (this[0xc] != 0) {
        do {
          iVar14 = Mesh_FindWeldVertex16
                             (pfVar17,fVar15,*(int *)(unaff_EBP + -0x34),*(int *)(unaff_EBP + -0x1c)
                              ,*(uint *)(unaff_EBP + -0x40),*(int *)(unaff_EBP + -0x20),
                              *(int *)(unaff_EBP + -0x28),*(int *)(unaff_EBP + -0x2c),
                              (int *)(unaff_EBP + -0x58),(float *)(unaff_EBP + -0x5c));
          if (iVar14 == 0) {
            *(float *)(*(int *)(unaff_EBP + -0x14) + (int)fVar15 * 4) = fVar15;
          }
          else {
            *(undefined4 *)(*(int *)(unaff_EBP + -0x14) + (int)fVar15 * 4) =
                 *(undefined4 *)(unaff_EBP + -0x5c);
          }
          pfVar17 = (float *)((int)pfVar17 + *(int *)(unaff_EBP + -0x98));
          fVar15 = (float)((int)fVar15 + 1);
        } while ((uint)fVar15 < (uint)this[0xc]);
      }
    }
    else {
      pvVar9 = operator_new(this[0xc] << 2);
      iVar14 = this[0xc];
      *(void **)(unaff_EBP + -0x3c) = pvVar9;
      puVar10 = operator_new(iVar14 << 2);
      uVar8 = 0;
      *(undefined4 **)(unaff_EBP + -0x38) = puVar10;
      if ((pvVar9 == (void *)0x0) || (puVar10 == (undefined4 *)0x0)) goto LAB_0048477a;
      puVar12 = *(undefined4 **)(unaff_EBP + -0x24);
      if (this[0xc] != 0) {
        do {
          uVar2 = *puVar12;
          puVar12 = (undefined4 *)((int)puVar12 + *(int *)(unaff_EBP + -0x98));
          *(undefined4 *)((int)pvVar9 + uVar8 * 4) = uVar2;
          uVar8 = uVar8 + 1;
        } while (uVar8 < (uint)this[0xc]);
      }
      Sort_IndexHeapByFloat(puVar10,(int)pvVar9,this[0xc]);
      fVar15 = *(float *)(unaff_EBP + 0xc);
      uVar8 = this[0xc];
      fVar3 = *(float *)(unaff_EBP + 0xc);
      *(undefined4 *)(unaff_EBP + 8) = 0;
      puVar12 = *(undefined4 **)(unaff_EBP + -0x14);
      for (uVar8 = uVar8 & 0x3fffffff; uVar8 != 0; uVar8 = uVar8 - 1) {
        *puVar12 = 0xffffffff;
        puVar12 = puVar12 + 1;
      }
      for (iVar14 = 0; iVar14 != 0; iVar14 = iVar14 + -1) {
        *(undefined1 *)puVar12 = 0xff;
        puVar12 = (undefined4 *)((int)puVar12 + 1);
      }
      uVar8 = this[0xc];
      if (uVar8 != 0) {
        *(undefined4 *)(unaff_EBP + -0x18) = 1;
        *(undefined4 **)(unaff_EBP + -0x10) = puVar10;
        do {
          if (*(uint *)(unaff_EBP + 8) < uVar8) {
            iVar14 = **(int **)(unaff_EBP + -0x10);
            do {
              fVar5 = *(float *)(*(int *)(unaff_EBP + -0x3c) + iVar14 * 4) -
                      *(float *)(*(int *)(unaff_EBP + -0x3c) + puVar10[*(int *)(unaff_EBP + 8)] * 4)
              ;
              if (fVar5 < *(float *)(unaff_EBP + 0xc) == (fVar5 == *(float *)(unaff_EBP + 0xc)))
              break;
              *(int *)(unaff_EBP + 8) = *(int *)(unaff_EBP + 8) + 1;
            } while (*(uint *)(unaff_EBP + 8) < uVar8);
          }
          piVar4 = *(int **)(unaff_EBP + -0x10);
          iVar14 = *piVar4;
          piVar13 = (int *)(*(int *)(unaff_EBP + -0x14) + iVar14 * 4);
          if (*piVar13 == -1) {
            *piVar13 = iVar14;
            pfVar17 = (float *)(*piVar4 * *(int *)(unaff_EBP + -0x98) + *(int *)(unaff_EBP + -0x24))
            ;
            uVar8 = *(uint *)(unaff_EBP + -0x18);
            while (*(uint *)(unaff_EBP + -0x30) = uVar8, uVar8 < *(uint *)(unaff_EBP + 8)) {
              iVar14 = puVar10[uVar8];
              piVar13 = (int *)(*(int *)(unaff_EBP + -0x14) + iVar14 * 4);
              iVar16 = *piVar13;
              *(int *)(unaff_EBP + -0x1c) = iVar14;
              *(int **)(unaff_EBP + -0x40) = piVar13;
              if (iVar16 == -1) {
                puVar10 = (undefined4 *)
                          (iVar14 * *(int *)(unaff_EBP + -0x98) + *(int *)(unaff_EBP + -0x24));
                *(undefined4 *)(unaff_EBP + -0x54) = *puVar10;
                *(undefined4 *)(unaff_EBP + -0x50) = puVar10[1];
                *(undefined4 *)(unaff_EBP + -0x4c) = puVar10[2];
                fVar7 = *(float *)(unaff_EBP + -0x54) - *pfVar17;
                *(float *)(unaff_EBP + -0x54) = fVar7;
                fVar6 = *(float *)(unaff_EBP + -0x50) - pfVar17[1];
                *(float *)(unaff_EBP + -0x50) = fVar6;
                fVar5 = *(float *)(unaff_EBP + -0x4c) - pfVar17[2];
                *(float *)(unaff_EBP + -0x4c) = fVar5;
                if (fVar7 * fVar7 + fVar6 * fVar6 + fVar5 * fVar5 < fVar15 * fVar3) {
                  iVar14 = **(int **)(unaff_EBP + -0x10);
                  uVar8 = *(uint *)(*(int *)(unaff_EBP + -0x20) + iVar14 * 4);
                  *(int *)(unaff_EBP + -0x48) = iVar14;
                  if (uVar8 != 0xffffffff) {
                    do {
                      puVar1 = (ushort *)(*(int *)(unaff_EBP + -0x2c) + (uVar8 / 3) * 6);
                      if ((((uint)*puVar1 == *(uint *)(unaff_EBP + -0x1c)) ||
                          ((uint)puVar1[1] == *(uint *)(unaff_EBP + -0x1c))) ||
                         ((uint)puVar1[2] == *(uint *)(unaff_EBP + -0x1c))) goto LAB_0048473f;
                      uVar8 = *(uint *)(*(int *)(unaff_EBP + -0x28) + uVar8 * 4);
                    } while (uVar8 != 0xffffffff);
                    piVar13 = *(int **)(unaff_EBP + -0x40);
                    iVar14 = *(int *)(unaff_EBP + -0x48);
                  }
                  *piVar13 = iVar14;
                }
LAB_0048473f:
                puVar10 = *(undefined4 **)(unaff_EBP + -0x38);
              }
              uVar8 = *(int *)(unaff_EBP + -0x30) + 1;
            }
          }
          *(int *)(unaff_EBP + -0x18) = *(int *)(unaff_EBP + -0x18) + 1;
          *(int *)(unaff_EBP + -0x10) = *(int *)(unaff_EBP + -0x10) + 4;
          uVar8 = this[0xc];
        } while (*(int *)(unaff_EBP + -0x18) - 1U < uVar8);
      }
    }
    iVar14 = (**(code **)(*this + 0x4c))
                       (this,*(undefined4 *)(unaff_EBP + -0x14),*(undefined4 *)(unaff_EBP + 0x10));
  }
LAB_00484781:
  _free(*(void **)(unaff_EBP + -0x34));
  _free(*(void **)(unaff_EBP + -0x44));
  _free(*(void **)(unaff_EBP + -0x20));
  _free(*(void **)(unaff_EBP + -0x28));
  _free(*(void **)(unaff_EBP + -0x3c));
  _free(*(void **)(unaff_EBP + -0x38));
  _free(*(void **)(unaff_EBP + -0x14));
  if (*(int *)(unaff_EBP + -0x24) != 0) {
    (**(code **)(*(int *)this[10] + 0x30))((int *)this[10]);
  }
  if (*(int *)(unaff_EBP + -0x2c) != 0) {
    (**(code **)(*(int *)this[0xf] + 0x30))((int *)this[0xf]);
  }
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return iVar14;
}

