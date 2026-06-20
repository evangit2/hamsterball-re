
void __fastcall Scene_MarkDirty(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  *(undefined1 *)(*(int *)(param_1 + 0x480) + 0x434) = 1;
  if (*(char *)(param_1 + 0x430) != '\0') {
    iVar2 = AthenaList_NextIndex(param_1 + 0x18);
    *(undefined4 *)(param_1 + 0x20 + iVar2 * 4) = 0;
    if (*(int *)(param_1 + 0x1c) < 1) {
      iVar3 = 0;
    }
    else {
      iVar3 = **(int **)(param_1 + 0x424);
      *(undefined4 *)(param_1 + 0x20 + iVar2 * 4) = 1;
    }
    while (iVar3 != 0) {
      Scene_MarkDirty(iVar3);
      iVar1 = *(int *)(param_1 + 0x20 + iVar2 * 4);
      if (*(int *)(param_1 + 0x1c) <= iVar1) {
        return;
      }
      iVar3 = *(int *)(*(int *)(param_1 + 0x424) + iVar1 * 4);
      *(int *)(param_1 + 0x20 + iVar2 * 4) = iVar1 + 1;
    }
  }
  return;
}

