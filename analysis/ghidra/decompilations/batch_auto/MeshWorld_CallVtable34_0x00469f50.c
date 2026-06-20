
void __thiscall MeshWorld_CallVtable34(void *this,undefined4 param_1)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  
  iVar2 = AthenaList_NextIndex((int)this + 4);
  *(undefined4 *)((int)this + iVar2 * 4 + 0xc) = 0;
  if (*(int *)((int)this + 8) < 1) {
    piVar3 = (int *)0x0;
  }
  else {
    piVar3 = (int *)**(undefined4 **)((int)this + 0x410);
    *(undefined4 *)((int)this + iVar2 * 4 + 0xc) = 1;
  }
  while( true ) {
    if (piVar3 == (int *)0x0) {
      return;
    }
    (**(code **)(*piVar3 + 0x34))(param_1);
    iVar1 = *(int *)((int)this + iVar2 * 4 + 0xc);
    if (*(int *)((int)this + 8) <= iVar1) break;
    piVar3 = *(int **)(*(int *)((int)this + 0x410) + iVar1 * 4);
    *(int *)((int)this + iVar2 * 4 + 0xc) = iVar1 + 1;
  }
  return;
}

