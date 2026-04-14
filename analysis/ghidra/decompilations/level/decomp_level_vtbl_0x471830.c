
uint __thiscall FUN_00471830(int param_1,int param_2,float *param_3,char param_4)

{
  float fVar1;
  int iVar2;
  undefined4 uVar3;
  bool bVar4;
  void *this;
  undefined4 *puVar5;
  float *pfVar6;
  int iVar7;
  float *pfVar8;
  int iVar9;
  float *pfVar10;
  float *pfVar11;
  undefined4 *puStack_14;
  int iStack_10;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_004cd8f6;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  if (*(undefined4 **)(param_1 + 8) != (undefined4 *)0x0) {
    ExceptionList = &pvStack_c;
    (**(code **)**(undefined4 **)(param_1 + 8))(1);
    *(undefined4 *)(param_1 + 8) = 0;
  }
  this = operator_new(0x488);
  uStack_4 = 0;
  if (this == (void *)0x0) {
    puVar5 = (undefined4 *)0x0;
  }
  else {
    puVar5 = MeshWorld_ctor(this,*(undefined4 *)(param_1 + 4));
  }
  *(undefined4 **)(param_1 + 8) = puVar5;
  *(undefined1 *)(param_1 + 0xd) = 1;
  iVar7 = *(int *)(param_2 + 8);
  uStack_4 = 0xffffffff;
  iStack_10 = 0;
  *(undefined4 *)(iVar7 + 0x34) = 0;
  if (*(int *)(iVar7 + 0x30) < 1) {
    iVar9 = 0;
  }
  else {
    iVar9 = **(int **)(iVar7 + 0x438);
    *(undefined4 *)(iVar7 + 0x34) = 1;
  }
  if (iVar9 != 0) {
    do {
      puStack_14 = (undefined4 *)0x0;
      if (*(char *)(iVar9 + 0x85c) != '\x01') {
        *(undefined4 *)(iVar9 + 0x14) = 0;
        if (*(int *)(iVar9 + 0x10) < 1) {
          pfVar8 = (float *)0x0;
        }
        else {
          pfVar8 = (float *)**(undefined4 **)(iVar9 + 0x418);
          *(undefined4 *)(iVar9 + 0x14) = 1;
        }
        while (pfVar8 != (float *)0x0) {
          bVar4 = FUN_00458450(pfVar8,param_3);
          if (bVar4) {
            if (puStack_14 == (undefined4 *)0x0) {
              puVar5 = operator_new(0x874);
              uStack_4 = 1;
              if (puVar5 == (undefined4 *)0x0) {
                puStack_14 = (undefined4 *)0x0;
              }
              else {
                puStack_14 = CreateMeshBuffer(puVar5);
              }
              puStack_14[1] = *(undefined4 *)(iVar9 + 4);
              uStack_4 = 0xffffffff;
              FUN_00453780((void *)(*(int *)(param_1 + 8) + 0x2c),(int)puStack_14);
            }
            pfVar6 = operator_new(0x60);
            pfVar10 = pfVar8;
            pfVar11 = pfVar6;
            for (iVar7 = 0x18; iVar7 != 0; iVar7 = iVar7 + -1) {
              *pfVar11 = *pfVar10;
              pfVar10 = pfVar10 + 1;
              pfVar11 = pfVar11 + 1;
            }
            FUN_00453780(puStack_14 + 3,(int)pfVar6);
            if (param_4 == '\0') {
              AthenaList_Remove((void *)(iVar9 + 0xc),(int)pfVar8);
            }
            iStack_10 = iStack_10 + 1;
          }
          iVar7 = *(int *)(iVar9 + 0x14);
          if (*(int *)(iVar9 + 0x10) <= iVar7) break;
          pfVar8 = *(float **)(*(int *)(iVar9 + 0x418) + iVar7 * 4);
          *(int *)(iVar9 + 0x14) = iVar7 + 1;
        }
      }
      iVar7 = *(int *)(param_2 + 8);
      iVar2 = *(int *)(iVar7 + 0x34);
      if (*(int *)(iVar7 + 0x30) <= iVar2) break;
      iVar9 = *(int *)(*(int *)(iVar7 + 0x438) + iVar2 * 4);
      *(int *)(iVar7 + 0x34) = iVar2 + 1;
    } while (iVar9 != 0);
    if (iStack_10 != 0) {
      *(undefined4 *)(*(int *)(param_1 + 8) + 0x45c) = 0x4b18967f;
      *(undefined4 *)(*(int *)(param_1 + 8) + 0x460) = 0x4b18967f;
      *(undefined4 *)(*(int *)(param_1 + 8) + 0x464) = 0x4b18967f;
      *(undefined4 *)(*(int *)(param_1 + 8) + 0x468) = 0xcb18967f;
      *(undefined4 *)(*(int *)(param_1 + 8) + 0x46c) = 0xcb18967f;
      *(undefined4 *)(*(int *)(param_1 + 8) + 0x470) = 0xcb18967f;
      iVar7 = *(int *)(param_1 + 8);
      *(undefined4 *)(iVar7 + 0x34) = 0;
      if (*(int *)(iVar7 + 0x30) < 1) {
        iVar9 = 0;
      }
      else {
        iVar9 = **(int **)(iVar7 + 0x438);
        *(undefined4 *)(iVar7 + 0x34) = 1;
      }
      while (iVar9 != 0) {
        *(undefined4 *)(iVar9 + 0x14) = 0;
        if (*(int *)(iVar9 + 0x10) < 1) {
          iVar7 = 0;
        }
        else {
          iVar7 = **(int **)(iVar9 + 0x418);
          *(undefined4 *)(iVar9 + 0x14) = 1;
        }
        while (iVar7 != 0) {
          pfVar8 = (float *)(iVar7 + 8);
          iVar7 = 3;
          do {
            iVar2 = *(int *)(param_1 + 8);
            if (pfVar8[-2] <= *(float *)(iVar2 + 0x45c)) {
              fVar1 = pfVar8[-2];
            }
            else {
              fVar1 = *(float *)(iVar2 + 0x45c);
            }
            *(float *)(iVar2 + 0x45c) = fVar1;
            iVar2 = *(int *)(param_1 + 8);
            if (pfVar8[-1] <= *(float *)(iVar2 + 0x460)) {
              fVar1 = pfVar8[-1];
            }
            else {
              fVar1 = *(float *)(iVar2 + 0x460);
            }
            *(float *)(iVar2 + 0x460) = fVar1;
            iVar2 = *(int *)(param_1 + 8);
            if (*pfVar8 <= *(float *)(iVar2 + 0x464)) {
              fVar1 = *pfVar8;
            }
            else {
              fVar1 = *(float *)(iVar2 + 0x464);
            }
            *(float *)(iVar2 + 0x464) = fVar1;
            iVar2 = *(int *)(param_1 + 8);
            if (*(float *)(iVar2 + 0x468) <= pfVar8[-2]) {
              fVar1 = pfVar8[-2];
            }
            else {
              fVar1 = *(float *)(iVar2 + 0x468);
            }
            *(float *)(iVar2 + 0x468) = fVar1;
            iVar2 = *(int *)(param_1 + 8);
            if (*(float *)(iVar2 + 0x46c) <= pfVar8[-1]) {
              fVar1 = pfVar8[-1];
            }
            else {
              fVar1 = *(float *)(iVar2 + 0x46c);
            }
            *(float *)(iVar2 + 0x46c) = fVar1;
            iVar2 = *(int *)(param_1 + 8);
            if (*(float *)(iVar2 + 0x470) <= *pfVar8) {
              fVar1 = *pfVar8;
            }
            else {
              fVar1 = *(float *)(iVar2 + 0x470);
            }
            pfVar8 = pfVar8 + 8;
            *(float *)(iVar2 + 0x470) = fVar1;
            iVar7 = iVar7 + -1;
          } while (iVar7 != 0);
          iVar2 = *(int *)(iVar9 + 0x14);
          if (*(int *)(iVar9 + 0x10) <= iVar2) break;
          iVar7 = *(int *)(*(int *)(iVar9 + 0x418) + iVar2 * 4);
          *(int *)(iVar9 + 0x14) = iVar2 + 1;
        }
        iVar7 = *(int *)(param_1 + 8);
        iVar2 = *(int *)(iVar7 + 0x34);
        if (*(int *)(iVar7 + 0x30) <= iVar2) break;
        iVar9 = *(int *)(*(int *)(iVar7 + 0x438) + iVar2 * 4);
        *(int *)(iVar7 + 0x34) = iVar2 + 1;
      }
      uVar3 = *(undefined4 *)(*(int *)(param_2 + 8) + 0x28);
      *(undefined4 *)(*(int *)(param_1 + 8) + 0x28) = uVar3;
      ExceptionList = pvStack_c;
      return CONCAT31((int3)((uint)uVar3 >> 8),1);
    }
  }
  ExceptionList = pvStack_c;
  return iVar7 + 0x2cU & 0xffffff00;
}
