
void __thiscall Scene_AddAllObjects(void *this,void *param_1)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  
  iVar2 = AthenaList_NextIndex((int)this + 0x34);
  *(undefined4 *)((int)this + iVar2 * 4 + 0x3c) = 0;
  if (*(int *)((int)this + 0x38) < 1) {
    piVar3 = (int *)0x0;
  }
  else {
    piVar3 = (int *)**(undefined4 **)((int)this + 0x440);
    *(undefined4 *)((int)this + iVar2 * 4 + 0x3c) = 1;
  }
  while( true ) {
    if (piVar3 == (int *)0x0) {
      return;
    }
    Scene_AddObject(param_1,piVar3);
    iVar1 = *(int *)((int)this + iVar2 * 4 + 0x3c);
    if (*(int *)((int)this + 0x38) <= iVar1) break;
    piVar3 = *(int **)(*(int *)((int)this + 0x440) + iVar1 * 4);
    *(int *)((int)this + iVar2 * 4 + 0x3c) = iVar1 + 1;
  }
  return;
}

