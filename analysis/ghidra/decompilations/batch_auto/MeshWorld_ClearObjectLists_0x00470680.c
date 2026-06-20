
void __fastcall MeshWorld_ClearObjectLists(int param_1)

{
  int iVar1;
  int iVar2;
  
  MeshWorld_OptimizeAll(param_1);
  *(undefined4 *)(param_1 + 0x34) = 0;
  if (*(int *)(param_1 + 0x30) < 1) {
    iVar2 = 0;
  }
  else {
    iVar2 = **(int **)(param_1 + 0x438);
    *(undefined4 *)(param_1 + 0x34) = 1;
  }
  while( true ) {
    if (iVar2 == 0) {
      return;
    }
    if (*(char *)(iVar2 + 0x85c) == '\x01') {
      AthenaList_Free(iVar2 + 0xc);
    }
    iVar1 = *(int *)(param_1 + 0x34);
    if (*(int *)(param_1 + 0x30) <= iVar1) break;
    iVar2 = *(int *)(*(int *)(param_1 + 0x438) + iVar1 * 4);
    *(int *)(param_1 + 0x34) = iVar1 + 1;
  }
  return;
}

