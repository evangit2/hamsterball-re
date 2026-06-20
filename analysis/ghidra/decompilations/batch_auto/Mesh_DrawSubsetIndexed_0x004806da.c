
undefined4 Mesh_DrawSubsetIndexed(int param_1,uint param_2)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  uint *puVar4;
  int iVar5;
  uint uVar6;
  int iVar7;
  int *piVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  
  iVar1 = param_1;
  if (*(int *)(param_1 + 0x28) == 0) {
    return 0;
  }
  (**(code **)(**(int **)(param_1 + 0x24) + 0x130))
            (*(int **)(param_1 + 0x24),*(undefined4 *)(param_1 + 4));
  (**(code **)(**(int **)(param_1 + 0x24) + 0x14c))
            (*(int **)(param_1 + 0x24),0,*(undefined4 *)(param_1 + 0x28),
             *(undefined4 *)(param_1 + 0x2c));
  (**(code **)(**(int **)(param_1 + 0x24) + 0x154))
            (*(int **)(param_1 + 0x24),*(undefined4 *)(param_1 + 0x3c),0);
  if ((*(byte *)(param_1 + 0xc) & 0x10) == 0) {
    iVar3 = 0;
    uVar6 = 0;
    if (*(int *)(param_1 + 0x58) == 0) {
      return 0;
    }
    param_1 = 0;
    do {
      if (*(uint *)(*(int *)(iVar1 + 0x48) + uVar6 * 4) == param_2) {
        iVar3 = iVar3 + 1;
      }
      else {
        if (iVar3 != 0) {
          (**(code **)(**(int **)(iVar1 + 0x24) + 0x11c))
                    (*(int **)(iVar1 + 0x24),4,0,*(undefined4 *)(iVar1 + 0x30),param_1 + iVar3 * -3,
                     iVar3);
        }
        iVar3 = 0;
      }
      param_1 = param_1 + 3;
      uVar6 = uVar6 + 1;
    } while (uVar6 < *(uint *)(iVar1 + 0x58));
    if (iVar3 == 0) {
      return 0;
    }
    piVar8 = *(int **)(iVar1 + 0x24);
    iVar5 = *piVar8;
    iVar7 = uVar6 * 3 + iVar3 * -3;
    uVar10 = *(undefined4 *)(iVar1 + 0x30);
    uVar9 = 0;
  }
  else {
    uVar6 = *(uint *)(param_1 + 0x54);
    if (((uVar6 <= param_2) ||
        (uVar2 = param_2, *(uint *)(*(int *)(param_1 + 0x50) + param_2 * 0x14) != param_2)) &&
       (uVar2 = 0, uVar6 != 0)) {
      puVar4 = *(uint **)(param_1 + 0x50);
      do {
        if (*puVar4 == param_2) break;
        uVar2 = uVar2 + 1;
        puVar4 = puVar4 + 5;
      } while (uVar2 < uVar6);
    }
    if (uVar6 <= uVar2) {
      return 0;
    }
    iVar1 = *(int *)(param_1 + 0x50) + uVar2 * 0x14;
    iVar3 = *(int *)(iVar1 + 8);
    if (iVar3 == 0) {
      return 0;
    }
    piVar8 = *(int **)(param_1 + 0x24);
    iVar5 = *piVar8;
    iVar7 = *(int *)(iVar1 + 4) * 3;
    uVar10 = *(undefined4 *)(iVar1 + 0x10);
    uVar9 = *(undefined4 *)(iVar1 + 0xc);
  }
  (**(code **)(iVar5 + 0x11c))(piVar8,4,uVar9,uVar10,iVar7,iVar3);
  return 0;
}

