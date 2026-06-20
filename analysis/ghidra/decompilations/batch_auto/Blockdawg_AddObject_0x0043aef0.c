
void __thiscall Blockdawg_AddObject(void *this,int param_1)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  
  iVar2 = AthenaList_NextIndex((int)this + 0x1aa64);
  *(undefined4 *)((int)this + iVar2 * 4 + 0x1aa6c) = 0;
  if (*(int *)((int)this + 0x1aa68) < 1) {
    piVar3 = (int *)0x0;
  }
  else {
    piVar3 = (int *)**(undefined4 **)((int)this + 0x1ae70);
    *(undefined4 *)((int)this + iVar2 * 4 + 0x1aa6c) = 1;
  }
  while (piVar3 != (int *)0x0) {
    if (*piVar3 == param_1) {
      piVar3[1] = 10;
      return;
    }
    iVar1 = *(int *)((int)this + iVar2 * 4 + 0x1aa6c);
    if (*(int *)((int)this + 0x1aa68) <= iVar1) break;
    piVar3 = *(int **)(*(int *)((int)this + 0x1ae70) + iVar1 * 4);
    *(int *)((int)this + iVar2 * 4 + 0x1aa6c) = iVar1 + 1;
  }
  piVar3 = operator_new(8);
  *piVar3 = param_1;
  piVar3[1] = 10;
  AthenaList_Append((void *)((int)this + 0x1aa64),(int)piVar3);
  return;
}

