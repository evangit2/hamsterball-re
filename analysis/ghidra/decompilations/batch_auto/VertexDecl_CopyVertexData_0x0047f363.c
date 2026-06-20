
void __thiscall VertexDecl_CopyVertexData(void *this,undefined4 *param_1,uint *param_2)

{
  float fVar1;
  float fVar2;
  float *pfVar3;
  uint *puVar4;
  uint uVar5;
  int iVar6;
  uint uVar7;
  undefined4 *puVar8;
  undefined4 *puVar9;
  undefined4 *puVar10;
  float *pfVar11;
  undefined4 uVar12;
  undefined4 *puVar13;
  
  puVar9 = param_1;
  fVar2 = 0.0;
  *param_2 = *param_1;
  param_2[1] = param_1[1];
  param_2[2] = param_1[2];
  pfVar11 = (float *)(param_2 + 3);
  if (*(int *)((int)this + 0x104) != 0) {
    uVar5 = 0;
    if (*(int *)((int)this + 0x104) != 0) {
      pfVar3 = (float *)(param_1 + 3);
      do {
        fVar2 = fVar2 + *pfVar3;
        fVar1 = *pfVar3;
        pfVar3 = pfVar3 + 1;
        pfVar11[uVar5] = fVar1;
        uVar5 = uVar5 + 1;
      } while (uVar5 < *(uint *)((int)this + 0x104));
    }
    pfVar11 = pfVar11 + *(int *)((int)this + 0x104);
  }
  if (*(int *)((int)this + 0x120) != 0) {
    *pfVar11 = 1.0 - fVar2;
    pfVar11 = pfVar11 + 1;
  }
  iVar6 = *(int *)((int)this + 0x108);
  if (iVar6 != 0) {
    for (; iVar6 != 0; iVar6 = iVar6 + -1) {
      *pfVar11 = 0.0;
      pfVar11 = pfVar11 + 1;
    }
  }
  if (*(int *)((int)this + 0x11c) == 0) {
    if (((*(byte *)((int)this + 0x61) & 0x10) != 0) && (*(int *)((int)this + 0x98) != 0)) {
      uVar12 = 0;
      goto LAB_0047f3fa;
    }
  }
  else {
    uVar12 = param_1[*(int *)((int)this + 0x38) + 2];
LAB_0047f3fa:
    VertexDecl_WriteBlendWeights((void *)((int)this + 0x60),(int)param_2,uVar12);
  }
  if (*(int *)((int)this + 0x10c) == 0) {
    if (*(int *)((int)this + 0x68) != 0) {
      puVar8 = (undefined4 *)(*(int *)((int)this + 0x68) + (int)param_2);
      *puVar8 = 0;
      puVar8[1] = 0;
      puVar8[2] = 0;
    }
  }
  else {
    VertexDecl_CopyToBuffer
              ((void *)((int)this + 0x60),(int)param_2,
               (undefined4 *)(*(int *)((int)this + 8) + (int)param_1));
  }
  if (*(int *)((int)this + 0x118) == 0) {
    if ((*(int *)((int)this + 0x6c) != 0) && (*(int *)((int)this + 0x6c) != 0)) {
      *(undefined4 *)(*(int *)((int)this + 0x6c) + (int)param_2) = 0;
    }
  }
  else if (*(int *)((int)this + 0x6c) != 0) {
    *(undefined4 *)(*(int *)((int)this + 0x6c) + (int)param_2) =
         *(undefined4 *)((int)param_1 + *(int *)((int)this + 0xc));
  }
  uVar12 = 0;
  if (*(int *)((int)this + 0x110) == 0) {
    if ((*(int *)((int)this + 0x70) != 0) && (iVar6 = *(int *)((int)this + 0x70), iVar6 != 0))
    goto LAB_0047f495;
  }
  else {
    iVar6 = *(int *)((int)this + 0x70);
    if (iVar6 != 0) {
      uVar12 = *(undefined4 *)((int)param_1 + *(int *)((int)this + 0x10));
LAB_0047f495:
      *(undefined4 *)(iVar6 + (int)param_2) = uVar12;
    }
  }
  uVar12 = 0;
  if (*(int *)((int)this + 0x114) == 0) {
    if ((*(int *)((int)this + 0x74) == 0) || (iVar6 = *(int *)((int)this + 0x74), iVar6 == 0))
    goto LAB_0047f4c0;
  }
  else {
    iVar6 = *(int *)((int)this + 0x74);
    if (iVar6 == 0) goto LAB_0047f4c0;
    uVar12 = *(undefined4 *)((int)param_1 + *(int *)((int)this + 0x14));
  }
  *(undefined4 *)(iVar6 + (int)param_2) = uVar12;
LAB_0047f4c0:
  puVar8 = (undefined4 *)(*(int *)((int)this + 0x78) + (int)param_2);
  param_1 = (undefined4 *)0x0;
  puVar9 = (undefined4 *)(*(int *)((int)this + 0x18) + (int)puVar9);
  if (*(int *)((int)this + 0xc0) != 0) {
    param_2 = (uint *)((int)this + 0xc4);
    do {
      uVar5 = *param_2;
      puVar10 = puVar9;
      puVar13 = puVar8;
      for (uVar7 = uVar5 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
        *puVar13 = *puVar10;
        puVar10 = puVar10 + 1;
        puVar13 = puVar13 + 1;
      }
      for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
        *(undefined1 *)puVar13 = *(undefined1 *)puVar10;
        puVar10 = (undefined4 *)((int)puVar10 + 1);
        puVar13 = (undefined4 *)((int)puVar13 + 1);
      }
      uVar5 = param_2[8];
      if (uVar5 != 0) {
        puVar10 = (undefined4 *)(*param_2 + (int)puVar8);
        for (uVar7 = uVar5 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
          *puVar10 = 0;
          puVar10 = puVar10 + 1;
        }
        for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
          *(undefined1 *)puVar10 = 0;
          puVar10 = (undefined4 *)((int)puVar10 + 1);
        }
      }
      puVar9 = (undefined4 *)((int)puVar9 + param_2[-0x21]);
      puVar8 = (undefined4 *)((int)puVar8 + param_2[-9]);
      param_1 = (undefined4 *)((int)param_1 + 1);
      param_2 = param_2 + 1;
    } while (param_1 < *(undefined4 **)((int)this + 0xc0));
  }
  param_1 = *(undefined4 **)((int)this + 0xc0);
  if (param_1 < *(undefined4 **)((int)this + 0x9c)) {
    puVar4 = (uint *)((int)this + (int)param_1 * 4 + 0xa0);
    do {
      uVar5 = *puVar4;
      puVar9 = puVar8;
      for (uVar7 = uVar5 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
        *puVar9 = 0;
        puVar9 = puVar9 + 1;
      }
      for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
        *(undefined1 *)puVar9 = 0;
        puVar9 = (undefined4 *)((int)puVar9 + 1);
      }
      puVar8 = (undefined4 *)((int)puVar8 + *puVar4);
      param_1 = (undefined4 *)((int)param_1 + 1);
      puVar4 = puVar4 + 1;
    } while (param_1 < *(undefined4 **)((int)this + 0x9c));
  }
  return;
}

