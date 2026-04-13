
void __thiscall FUN_00461370(int param_1,void *param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int *piVar4;
  int iVar5;
  
  iVar2 = AthenaList_GetIndex(param_1 + 0x18);
  *(undefined4 *)(param_1 + 0x20 + iVar2 * 4) = 0;
  if (*(int *)(param_1 + 0x1c) < 1) {
    piVar4 = (int *)0x0;
  }
  else {
    piVar4 = (int *)**(undefined4 **)(param_1 + 0x424);
    *(undefined4 *)(param_1 + 0x20 + iVar2 * 4) = 1;
  }
  while (piVar4 != (int *)0x0) {
    (**(code **)(*piVar4 + 0x28))(param_2);
    iVar3 = *(int *)(param_1 + 0x20 + iVar2 * 4);
    if (*(int *)(param_1 + 0x1c) <= iVar3) break;
    piVar4 = *(int **)(*(int *)(param_1 + 0x424) + iVar3 * 4);
    *(int *)(param_1 + 0x20 + iVar2 * 4) = iVar3 + 1;
  }
  iVar3 = AthenaList_GetIndex(*(int *)(param_1 + 8) + 0x2c);
  iVar2 = *(int *)(param_1 + 8);
  *(undefined4 *)(iVar2 + 0x34 + iVar3 * 4) = 0;
  if (*(int *)(iVar2 + 0x30) < 1) {
    iVar5 = 0;
  }
  else {
    iVar5 = **(int **)(iVar2 + 0x438);
    *(undefined4 *)(iVar2 + 0x34 + iVar3 * 4) = 1;
  }
  while( true ) {
    if (iVar5 == 0) {
      return;
    }
    FUN_0046b200(param_2,(undefined4 *)
                         (*(int *)(iVar5 + 4) * 0x50 + *(int *)(*(int *)(param_1 + 8) + 0x28)));
    iVar2 = *(int *)(param_1 + 8);
    iVar1 = *(int *)(iVar2 + 0x34 + iVar3 * 4);
    if (*(int *)(iVar2 + 0x30) <= iVar1) break;
    iVar5 = *(int *)(*(int *)(iVar2 + 0x438) + iVar1 * 4);
    *(int *)(iVar2 + 0x34 + iVar3 * 4) = iVar1 + 1;
  }
  return;
}
