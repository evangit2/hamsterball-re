
void __fastcall MusicDevice_FadeAll(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  iVar2 = AthenaList_NextIndex(param_1 + 0xc);
  *(undefined4 *)(param_1 + 0x14 + iVar2 * 4) = 0;
  if (*(int *)(param_1 + 0x10) < 1) {
    iVar3 = 0;
  }
  else {
    iVar3 = **(int **)(param_1 + 0x418);
    *(undefined4 *)(param_1 + 0x14 + iVar2 * 4) = 1;
  }
  while( true ) {
    if (iVar3 == 0) {
      return;
    }
    MusicChannel_FadeUpdate(iVar3);
    iVar1 = *(int *)(param_1 + 0x14 + iVar2 * 4);
    if (*(int *)(param_1 + 0x10) <= iVar1) break;
    iVar3 = *(int *)(*(int *)(param_1 + 0x418) + iVar1 * 4);
    *(int *)(param_1 + 0x14 + iVar2 * 4) = iVar1 + 1;
  }
  return;
}

