
uint __thiscall FUN_00471830(void *this,int param_1,float *param_2,char param_3)

{
  float fVar1;
  int iVar2;
  undefined4 uVar3;
  bool bVar4;
  void *pvVar5;
  undefined4 *puVar6;
  float *pfVar7;
  int iVar8;
  float *pfVar9;
  int iVar10;
  float *pfVar11;
  float *pfVar12;
  undefined4 *puStack_14;
  int iStack_10;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_004cd8f6;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  if (*(undefined4 **)((int)this + 8) != (undefined4 *)0x0) {
    ExceptionList = &pvStack_c;
    (**(code **)**(undefined4 **)((int)this + 8))(1);
    *(undefined4 *)((int)this + 8) = 0;
  }
  pvVar5 = operator_new(0x488);
  uStack_4 = 0;
  if (pvVar5 == (void *)0x0) {
    pvVar5 = (void *)0x0;
  }
  else {
    pvVar5 = MeshWorld_ctor(pvVar5,*(undefined4 *)((int)this + 4));
  }
  *(void **)((int)this + 8) = pvVar5;
  *(undefined1 *)((int)this + 0xd) = 1;
  iVar8 = *(int *)(param_1 + 8);
  uStack_4 = 0xffffffff;
  iStack_10 = 0;
  *(undefined4 *)(iVar8 + 0x34) = 0;
  if (*(int *)(iVar8 + 0x30) < 1) {
    iVar10 = 0;
  }
  else {
    iVar10 = **(int **)(iVar8 + 0x438);
    *(undefined4 *)(iVar8 + 0x34) = 1;
  }
  if (iVar10 != 0) {
    do {
      puStack_14 = (undefined4 *)0x0;
      if (*(char *)(iVar10 + 0x85c) != '\x01') {
        *(undefined4 *)(iVar10 + 0x14) = 0;
        if (*(int *)(iVar10 + 0x10) < 1) {
          pfVar9 = (float *)0x0;
        }
        else {
          pfVar9 = (float *)**(undefined4 **)(iVar10 + 0x418);
          *(undefined4 *)(iVar10 + 0x14) = 1;
        }
        while (pfVar9 != (float *)0x0) {
          bVar4 = AABB_TriangleTest(pfVar9,param_2);
          if (bVar4) {
            if (puStack_14 == (undefined4 *)0x0) {
              puVar6 = operator_new(0x874);
              uStack_4 = 1;
              if (puVar6 == (undefined4 *)0x0) {
                puStack_14 = (undefined4 *)0x0;
              }
              else {
                puStack_14 = CreateMeshBuffer(puVar6);
              }
              puStack_14[1] = *(undefined4 *)(iVar10 + 4);
              uStack_4 = 0xffffffff;
              AthenaList_Append((void *)(*(int *)((int)this + 8) + 0x2c),(int)puStack_14);
            }
            pfVar7 = operator_new(0x60);
            pfVar11 = pfVar9;
            pfVar12 = pfVar7;
            for (iVar8 = 0x18; iVar8 != 0; iVar8 = iVar8 + -1) {
              *pfVar12 = *pfVar11;
              pfVar11 = pfVar11 + 1;
              pfVar12 = pfVar12 + 1;
            }
            AthenaList_Append(puStack_14 + 3,(int)pfVar7);
            if (param_3 == '\0') {
              IndexList_RemoveItem((void *)(iVar10 + 0xc),(int)pfVar9);
            }
            iStack_10 = iStack_10 + 1;
          }
          iVar8 = *(int *)(iVar10 + 0x14);
          if (*(int *)(iVar10 + 0x10) <= iVar8) break;
          pfVar9 = *(float **)(*(int *)(iVar10 + 0x418) + iVar8 * 4);
          *(int *)(iVar10 + 0x14) = iVar8 + 1;
        }
      }
      iVar8 = *(int *)(param_1 + 8);
      iVar2 = *(int *)(iVar8 + 0x34);
      if (*(int *)(iVar8 + 0x30) <= iVar2) break;
      iVar10 = *(int *)(*(int *)(iVar8 + 0x438) + iVar2 * 4);
      *(int *)(iVar8 + 0x34) = iVar2 + 1;
    } while (iVar10 != 0);
    if (iStack_10 != 0) {
      *(undefined4 *)(*(int *)((int)this + 8) + 0x45c) = 0x4b18967f;
      *(undefined4 *)(*(int *)((int)this + 8) + 0x460) = 0x4b18967f;
      *(undefined4 *)(*(int *)((int)this + 8) + 0x464) = 0x4b18967f;
      *(undefined4 *)(*(int *)((int)this + 8) + 0x468) = 0xcb18967f;
      *(undefined4 *)(*(int *)((int)this + 8) + 0x46c) = 0xcb18967f;
      *(undefined4 *)(*(int *)((int)this + 8) + 0x470) = 0xcb18967f;
      iVar8 = *(int *)((int)this + 8);
      *(undefined4 *)(iVar8 + 0x34) = 0;
      if (*(int *)(iVar8 + 0x30) < 1) {
        iVar10 = 0;
      }
      else {
        iVar10 = **(int **)(iVar8 + 0x438);
        *(undefined4 *)(iVar8 + 0x34) = 1;
      }
      while (iVar10 != 0) {
        *(undefined4 *)(iVar10 + 0x14) = 0;
        if (*(int *)(iVar10 + 0x10) < 1) {
          iVar8 = 0;
        }
        else {
          iVar8 = **(int **)(iVar10 + 0x418);
          *(undefined4 *)(iVar10 + 0x14) = 1;
        }
        while (iVar8 != 0) {
          pfVar9 = (float *)(iVar8 + 8);
          iVar8 = 3;
          do {
            iVar2 = *(int *)((int)this + 8);
            if (pfVar9[-2] <= *(float *)(iVar2 + 0x45c)) {
              fVar1 = pfVar9[-2];
            }
            else {
              fVar1 = *(float *)(iVar2 + 0x45c);
            }
            *(float *)(iVar2 + 0x45c) = fVar1;
            iVar2 = *(int *)((int)this + 8);
            if (pfVar9[-1] <= *(float *)(iVar2 + 0x460)) {
              fVar1 = pfVar9[-1];
            }
            else {
              fVar1 = *(float *)(iVar2 + 0x460);
            }
            *(float *)(iVar2 + 0x460) = fVar1;
            iVar2 = *(int *)((int)this + 8);
            if (*pfVar9 <= *(float *)(iVar2 + 0x464)) {
              fVar1 = *pfVar9;
            }
            else {
              fVar1 = *(float *)(iVar2 + 0x464);
            }
            *(float *)(iVar2 + 0x464) = fVar1;
            iVar2 = *(int *)((int)this + 8);
            if (*(float *)(iVar2 + 0x468) <= pfVar9[-2]) {
              fVar1 = pfVar9[-2];
            }
            else {
              fVar1 = *(float *)(iVar2 + 0x468);
            }
            *(float *)(iVar2 + 0x468) = fVar1;
            iVar2 = *(int *)((int)this + 8);
            if (*(float *)(iVar2 + 0x46c) <= pfVar9[-1]) {
              fVar1 = pfVar9[-1];
            }
            else {
              fVar1 = *(float *)(iVar2 + 0x46c);
            }
            *(float *)(iVar2 + 0x46c) = fVar1;
            iVar2 = *(int *)((int)this + 8);
            if (*(float *)(iVar2 + 0x470) <= *pfVar9) {
              fVar1 = *pfVar9;
            }
            else {
              fVar1 = *(float *)(iVar2 + 0x470);
            }
            pfVar9 = pfVar9 + 8;
            *(float *)(iVar2 + 0x470) = fVar1;
            iVar8 = iVar8 + -1;
          } while (iVar8 != 0);
          iVar2 = *(int *)(iVar10 + 0x14);
          if (*(int *)(iVar10 + 0x10) <= iVar2) break;
          iVar8 = *(int *)(*(int *)(iVar10 + 0x418) + iVar2 * 4);
          *(int *)(iVar10 + 0x14) = iVar2 + 1;
        }
        iVar8 = *(int *)((int)this + 8);
        iVar2 = *(int *)(iVar8 + 0x34);
        if (*(int *)(iVar8 + 0x30) <= iVar2) break;
        iVar10 = *(int *)(*(int *)(iVar8 + 0x438) + iVar2 * 4);
        *(int *)(iVar8 + 0x34) = iVar2 + 1;
      }
      uVar3 = *(undefined4 *)(*(int *)(param_1 + 8) + 0x28);
      *(undefined4 *)(*(int *)((int)this + 8) + 0x28) = uVar3;
      ExceptionList = pvStack_c;
      return CONCAT31((int3)((uint)uVar3 >> 8),1);
    }
  }
  ExceptionList = pvStack_c;
  return iVar8 + 0x2cU & 0xffffff00;
}

