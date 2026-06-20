
int __thiscall Level_FindObjectByName(void *this,char *param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  
  iVar2 = AthenaList_NextIndex(*(int *)((int)this + 0x480) + 0xcac);
  iVar3 = *(int *)((int)this + 0x480);
  *(undefined4 *)(iVar3 + 0xcb4 + iVar2 * 4) = 0;
  if (*(int *)(iVar3 + 0xcb0) < 1) {
    iVar4 = 0;
  }
  else {
    iVar4 = **(int **)(iVar3 + 0x10b8);
    *(undefined4 *)(iVar3 + 0xcb4 + iVar2 * 4) = 1;
  }
  while( true ) {
    if (iVar4 == 0) {
      return 0;
    }
    iVar3 = __stricmp(param_1,*(char **)(iVar4 + 0x50));
    if (iVar3 == 0) break;
    iVar3 = *(int *)((int)this + 0x480);
    iVar1 = *(int *)(iVar3 + 0xcb4 + iVar2 * 4);
    if (*(int *)(iVar3 + 0xcb0) <= iVar1) {
      return 0;
    }
    iVar4 = *(int *)(*(int *)(iVar3 + 0x10b8) + iVar1 * 4);
    *(int *)(iVar3 + 0xcb4 + iVar2 * 4) = iVar1 + 1;
  }
  return iVar4;
}

