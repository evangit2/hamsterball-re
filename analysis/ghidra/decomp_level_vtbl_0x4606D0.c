
void __fastcall FUN_004606d0(int param_1)

{
  void **ppvVar1;
  void *pvVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int *piVar6;
  int iVar7;
  undefined4 *puVar8;
  int iVar9;
  int iVar10;
  undefined4 *puVar11;
  bool bVar12;
  int iStack_58;
  undefined4 *apuStack_48 [15];
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_004ccee8;
  pvStack_c = ExceptionList;
  ppvVar1 = &pvStack_c;
  if (*(int *)(param_1 + 0x47c) == param_1) {
    if (*(char *)(*(int *)(param_1 + 0x480) + 0x10c4) != '\0') {
      return;
    }
    ExceptionList = &pvStack_c;
    *(undefined1 *)(*(int *)(param_1 + 0x480) + 0x10c4) = 1;
    pvVar2 = operator_new(*(int *)(*(int *)(param_1 + 0x480) + 0x43c) * 0x60);
    *(void **)(*(int *)(param_1 + 0x480) + 0x10c8) = pvVar2;
    *(undefined4 *)(*(int *)(param_1 + 0x480) + 0x10cc) = 0;
    ppvVar1 = ExceptionList;
  }
  ExceptionList = ppvVar1;
  iVar9 = 0;
  if (*(char *)(param_1 + 0x430) != '\0') {
    iVar3 = AthenaList_GetIndex(param_1 + 0x18);
    *(undefined4 *)(param_1 + 0x20 + iVar3 * 4) = 0;
    if (*(int *)(param_1 + 0x1c) < 1) {
      piVar6 = (int *)0x0;
    }
    else {
      piVar6 = (int *)**(undefined4 **)(param_1 + 0x424);
      *(undefined4 *)(param_1 + 0x20 + iVar3 * 4) = 1;
    }
    while (piVar6 != (int *)0x0) {
      (**(code **)(*piVar6 + 4))();
      iVar4 = *(int *)(param_1 + 0x20 + iVar3 * 4);
      if (*(int *)(param_1 + 0x1c) <= iVar4) break;
      piVar6 = *(int **)(*(int *)(param_1 + 0x424) + iVar4 * 4);
      *(int *)(param_1 + 0x20 + iVar3 * 4) = iVar4 + 1;
    }
  }
  iVar4 = AthenaList_GetIndex(*(int *)(param_1 + 8) + 0x2c);
  iVar3 = *(int *)(param_1 + 8);
  *(undefined4 *)(iVar3 + 0x34 + iVar4 * 4) = 0;
  if (0 < *(int *)(iVar3 + 0x30)) {
    iVar9 = **(int **)(iVar3 + 0x438);
    *(undefined4 *)(iVar3 + 0x34 + iVar4 * 4) = 1;
  }
  while (iVar9 != 0) {
    iVar10 = 0;
    iVar5 = AthenaList_GetIndex(iVar9 + 0x424);
    iVar3 = 0;
    *(undefined4 *)(iVar9 + 0x42c + iVar5 * 4) = 0;
    if (0 < *(int *)(iVar9 + 0x428)) {
      iVar3 = **(int **)(iVar9 + 0x830);
      *(undefined4 *)(iVar9 + 0x42c + iVar5 * 4) = 1;
    }
    if (iVar3 != 0) {
      iVar10 = 0;
      do {
        iVar10 = iVar10 + *(int *)(iVar3 + 4);
        iVar7 = *(int *)(iVar9 + 0x42c + iVar5 * 4);
        if (*(int *)(iVar9 + 0x428) <= iVar7) break;
        iVar3 = *(int *)(*(int *)(iVar9 + 0x830) + iVar7 * 4);
        *(int *)(iVar9 + 0x42c + iVar5 * 4) = iVar7 + 1;
      } while (iVar3 != 0);
    }
    FUN_0046f3d0(apuStack_48 + 3,*(uint *)(param_1 + 4),iVar10 * 3,(uint *)0x0);
    uStack_4 = 0;
    *(undefined4 *)(iVar9 + 0x844) =
         *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x47c) + 0x480) + 0x10cc);
    *(undefined4 *)(iVar9 + 0x840) = 0;
    iVar3 = AthenaList_GetIndex(iVar9 + 0x424);
    *(undefined4 *)(iVar9 + 0x42c + iVar3 * 4) = 0;
    if (*(int *)(iVar9 + 0x428) < 1) {
      iVar5 = 0;
    }
    else {
      iVar5 = **(int **)(iVar9 + 0x830);
      *(undefined4 *)(iVar9 + 0x42c + iVar3 * 4) = 1;
    }
    while (iVar5 != 0) {
      if (*(char *)(*(int *)(param_1 + 0x480) + 0x434) == '\0') {
        iVar10 = *(int *)(iVar5 + 8);
        iVar7 = *(int *)(*(int *)(param_1 + 8) + 0x448);
      }
      else {
        iVar10 = *(int *)(iVar5 + 0xc);
        iVar7 = *(int *)(*(int *)(*(int *)(param_1 + 0x47c) + 0x480) + 0x440);
      }
      puVar8 = (undefined4 *)(iVar10 * 0x20 + iVar7);
      bVar12 = false;
      iStack_58 = 0;
      if (0 < *(int *)(iVar5 + 4)) {
        puVar11 = puVar8 + 0x10;
        do {
          apuStack_48[0] = puVar8;
          if (bVar12) {
            apuStack_48[2] = puVar11 + -8;
            apuStack_48[1] = puVar11;
          }
          else {
            apuStack_48[2] = puVar11;
            apuStack_48[1] = puVar11 + -8;
          }
          bVar12 = bVar12 == false;
          *(int *)(iVar9 + 0x840) = *(int *)(iVar9 + 0x840) + 1;
          iVar10 = *(int *)(*(int *)(param_1 + 0x47c) + 0x480);
          *(undefined4 *)(*(int *)(iVar10 + 0x10c8) + *(int *)(iVar10 + 0x10cc) * 0x20) = *puVar8;
          iVar10 = *(int *)(*(int *)(param_1 + 0x47c) + 0x480);
          *(undefined4 *)(*(int *)(iVar10 + 0x10cc) * 0x20 + 4 + *(int *)(iVar10 + 0x10c8)) =
               puVar11[-0xf];
          iVar10 = *(int *)(*(int *)(param_1 + 0x47c) + 0x480);
          *(undefined4 *)(*(int *)(iVar10 + 0x10cc) * 0x20 + 8 + *(int *)(iVar10 + 0x10c8)) =
               puVar11[-0xe];
          iVar10 = *(int *)(*(int *)(param_1 + 0x47c) + 0x480);
          *(undefined4 *)(*(int *)(iVar10 + 0x10cc) * 0x20 + 0xc + *(int *)(iVar10 + 0x10c8)) =
               puVar11[-0xd];
          iVar10 = *(int *)(*(int *)(param_1 + 0x47c) + 0x480);
          *(undefined4 *)(*(int *)(iVar10 + 0x10cc) * 0x20 + 0x10 + *(int *)(iVar10 + 0x10c8)) =
               puVar11[-0xc];
          iVar10 = *(int *)(*(int *)(param_1 + 0x47c) + 0x480);
          *(undefined4 *)(*(int *)(iVar10 + 0x10cc) * 0x20 + 0x14 + *(int *)(iVar10 + 0x10c8)) =
               puVar11[-0xb];
          iVar10 = *(int *)(*(int *)(param_1 + 0x47c) + 0x480);
          *(undefined4 *)(*(int *)(iVar10 + 0x10cc) * 0x20 + 0x18 + *(int *)(iVar10 + 0x10c8)) =
               puVar11[-10];
          iVar10 = *(int *)(*(int *)(param_1 + 0x47c) + 0x480);
          *(undefined4 *)(*(int *)(iVar10 + 0x10cc) * 0x20 + 0x1c + *(int *)(iVar10 + 0x10c8)) =
               puVar11[-9];
          piVar6 = (int *)(*(int *)(*(int *)(param_1 + 0x47c) + 0x480) + 0x10cc);
          *piVar6 = *piVar6 + 1;
          iVar10 = *(int *)(*(int *)(param_1 + 0x47c) + 0x480);
          *(undefined4 *)(*(int *)(iVar10 + 0x10cc) * 0x20 + *(int *)(iVar10 + 0x10c8)) =
               *apuStack_48[1];
          iVar10 = *(int *)(*(int *)(param_1 + 0x47c) + 0x480);
          *(undefined4 *)(*(int *)(iVar10 + 0x10cc) * 0x20 + 4 + *(int *)(iVar10 + 0x10c8)) =
               apuStack_48[1][1];
          iVar10 = *(int *)(*(int *)(param_1 + 0x47c) + 0x480);
          *(undefined4 *)(*(int *)(iVar10 + 0x10cc) * 0x20 + 8 + *(int *)(iVar10 + 0x10c8)) =
               apuStack_48[1][2];
          iVar10 = *(int *)(*(int *)(param_1 + 0x47c) + 0x480);
          *(undefined4 *)(*(int *)(iVar10 + 0x10cc) * 0x20 + 0xc + *(int *)(iVar10 + 0x10c8)) =
               apuStack_48[1][3];
          iVar10 = *(int *)(*(int *)(param_1 + 0x47c) + 0x480);
          *(undefined4 *)(*(int *)(iVar10 + 0x10cc) * 0x20 + 0x10 + *(int *)(iVar10 + 0x10c8)) =
               apuStack_48[1][4];
          iVar10 = *(int *)(*(int *)(param_1 + 0x47c) + 0x480);
          *(undefined4 *)(*(int *)(iVar10 + 0x10cc) * 0x20 + 0x14 + *(int *)(iVar10 + 0x10c8)) =
               apuStack_48[1][5];
          iVar10 = *(int *)(*(int *)(param_1 + 0x47c) + 0x480);
          *(undefined4 *)(*(int *)(iVar10 + 0x10cc) * 0x20 + 0x18 + *(int *)(iVar10 + 0x10c8)) =
               apuStack_48[1][6];
          iVar10 = *(int *)(*(int *)(param_1 + 0x47c) + 0x480);
          *(undefined4 *)(*(int *)(iVar10 + 0x10cc) * 0x20 + 0x1c + *(int *)(iVar10 + 0x10c8)) =
               apuStack_48[1][7];
          piVar6 = (int *)(*(int *)(*(int *)(param_1 + 0x47c) + 0x480) + 0x10cc);
          *piVar6 = *piVar6 + 1;
          iVar10 = *(int *)(*(int *)(param_1 + 0x47c) + 0x480);
          *(undefined4 *)(*(int *)(iVar10 + 0x10cc) * 0x20 + *(int *)(iVar10 + 0x10c8)) =
               *apuStack_48[2];
          iVar10 = *(int *)(*(int *)(param_1 + 0x47c) + 0x480);
          *(undefined4 *)(*(int *)(iVar10 + 0x10cc) * 0x20 + 4 + *(int *)(iVar10 + 0x10c8)) =
               apuStack_48[2][1];
          iVar10 = *(int *)(*(int *)(param_1 + 0x47c) + 0x480);
          *(undefined4 *)(*(int *)(iVar10 + 0x10cc) * 0x20 + 8 + *(int *)(iVar10 + 0x10c8)) =
               apuStack_48[2][2];
          iVar10 = *(int *)(*(int *)(param_1 + 0x47c) + 0x480);
          *(undefined4 *)(*(int *)(iVar10 + 0x10cc) * 0x20 + 0xc + *(int *)(iVar10 + 0x10c8)) =
               apuStack_48[2][3];
          iVar10 = *(int *)(*(int *)(param_1 + 0x47c) + 0x480);
          *(undefined4 *)(*(int *)(iVar10 + 0x10cc) * 0x20 + 0x10 + *(int *)(iVar10 + 0x10c8)) =
               apuStack_48[2][4];
          iVar10 = *(int *)(*(int *)(param_1 + 0x47c) + 0x480);
          *(undefined4 *)(*(int *)(iVar10 + 0x10cc) * 0x20 + 0x14 + *(int *)(iVar10 + 0x10c8)) =
               apuStack_48[2][5];
          iVar10 = *(int *)(*(int *)(param_1 + 0x47c) + 0x480);
          *(undefined4 *)(*(int *)(iVar10 + 0x10cc) * 0x20 + 0x18 + *(int *)(iVar10 + 0x10c8)) =
               apuStack_48[2][6];
          iVar10 = *(int *)(*(int *)(param_1 + 0x47c) + 0x480);
          *(undefined4 *)(*(int *)(iVar10 + 0x10cc) * 0x20 + 0x1c + *(int *)(iVar10 + 0x10c8)) =
               apuStack_48[2][7];
          piVar6 = (int *)(*(int *)(*(int *)(param_1 + 0x47c) + 0x480) + 0x10cc);
          *piVar6 = *piVar6 + 1;
          iVar10 = 0;
          do {
            FUN_0046fd60(apuStack_48 + 3,(float *)apuStack_48[iVar10]);
            iVar10 = iVar10 + 1;
          } while (iVar10 < 3);
          puVar8 = puVar8 + 8;
          puVar11 = puVar11 + 8;
          iStack_58 = iStack_58 + 1;
        } while (iStack_58 < *(int *)(iVar5 + 4));
      }
      iVar10 = *(int *)(iVar9 + 0x42c + iVar3 * 4);
      if (*(int *)(iVar9 + 0x428) <= iVar10) break;
      iVar5 = *(int *)(*(int *)(iVar9 + 0x830) + iVar10 * 4);
      *(int *)(iVar9 + 0x42c + iVar3 * 4) = iVar10 + 1;
    }
    piVar6 = FUN_00472340((int)(apuStack_48 + 3));
    *(int **)(iVar9 + 0x854) = piVar6;
    uStack_4 = 0xffffffff;
    FUN_0046f670(apuStack_48 + 3);
    iVar3 = *(int *)(param_1 + 8);
    iVar5 = *(int *)(iVar3 + 0x34 + iVar4 * 4);
    if (*(int *)(iVar3 + 0x30) <= iVar5) break;
    iVar9 = *(int *)(*(int *)(iVar3 + 0x438) + iVar5 * 4);
    *(int *)(iVar3 + 0x34 + iVar4 * 4) = iVar5 + 1;
  }
  piVar6 = *(int **)(*(int *)(param_1 + 0x480) + 0x444);
  if (piVar6 != (int *)0x0) {
    (**(code **)(*piVar6 + 8))(piVar6);
    *(undefined4 *)(*(int *)(param_1 + 0x480) + 0x444) = 0;
  }
  ExceptionList = pvStack_c;
  return;
}
