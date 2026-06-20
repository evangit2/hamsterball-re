
void __fastcall Scene_UpdateChildren(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  
  iVar2 = AthenaList_NextIndex(param_1 + 4);
  *(undefined4 *)(param_1 + 0xc + iVar2 * 4) = 0;
  if (*(int *)(param_1 + 8) < 1) {
    iVar4 = 0;
  }
  else {
    iVar4 = **(int **)(param_1 + 0x410);
    *(undefined4 *)(param_1 + 0xc + iVar2 * 4) = 1;
  }
  do {
    if (iVar4 == 0) {
      return;
    }
    iVar3 = AthenaList_NextIndex(iVar4 + 8);
    *(undefined4 *)(iVar4 + 0x10 + iVar3 * 4) = 0;
    if (*(int *)(iVar4 + 0xc) < 1) {
      iVar5 = 0;
    }
    else {
      iVar5 = **(int **)(iVar4 + 0x414);
      *(undefined4 *)(iVar4 + 0x10 + iVar3 * 4) = 1;
    }
    while (iVar5 != 0) {
      Vtable_CallOffset48(iVar5);
      iVar1 = *(int *)(iVar4 + 0x10 + iVar3 * 4);
      if (*(int *)(iVar4 + 0xc) <= iVar1) break;
      iVar5 = *(int *)(*(int *)(iVar4 + 0x414) + iVar1 * 4);
      *(int *)(iVar4 + 0x10 + iVar3 * 4) = iVar1 + 1;
    }
    iVar3 = *(int *)(param_1 + 0xc + iVar2 * 4);
    if (*(int *)(param_1 + 8) <= iVar3) {
      return;
    }
    iVar4 = *(int *)(*(int *)(param_1 + 0x410) + iVar3 * 4);
    *(int *)(param_1 + 0xc + iVar2 * 4) = iVar3 + 1;
  } while( true );
}

