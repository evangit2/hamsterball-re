
void __fastcall Scene_ResetObjectSlots(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int *piVar5;
  
  iVar4 = 0x710;
  do {
    piVar5 = *(int **)(iVar4 + *(int *)(param_1 + 4));
    if (piVar5 != (int *)0x0) {
      (**(code **)(*piVar5 + 0x10))(0);
      *(undefined4 *)(iVar4 + *(int *)(param_1 + 4)) = 0;
    }
    iVar4 = iVar4 + 4;
  } while (iVar4 < 0x730);
  iVar3 = 0;
  iVar2 = AthenaList_NextIndex(*(int *)(param_1 + 0x480) + 0x478);
  iVar4 = *(int *)(param_1 + 0x480);
  *(undefined4 *)(iVar4 + 0x480 + iVar2 * 4) = 0;
  if (*(int *)(iVar4 + 0x47c) < 1) {
    piVar5 = (int *)0x0;
  }
  else {
    piVar5 = (int *)**(undefined4 **)(iVar4 + 0x884);
    *(undefined4 *)(iVar4 + 0x480 + iVar2 * 4) = 1;
  }
  while( true ) {
    if (piVar5 == (int *)0x0) {
      return;
    }
    (**(code **)(*piVar5 + 0x10))(1);
    Scene_RegisterObject(*(void **)(param_1 + 4),iVar3,piVar5);
    iVar4 = *(int *)(param_1 + 0x480);
    iVar1 = *(int *)(iVar4 + 0x480 + iVar2 * 4);
    iVar3 = iVar3 + 1;
    if (*(int *)(iVar4 + 0x47c) <= iVar1) break;
    piVar5 = *(int **)(*(int *)(iVar4 + 0x884) + iVar1 * 4);
    *(int *)(iVar4 + 0x480 + iVar2 * 4) = iVar1 + 1;
  }
  return;
}

