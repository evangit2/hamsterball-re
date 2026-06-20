
void __thiscall Rotator_AddObject(void *this,int param_1)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  
  iVar2 = AthenaList_NextIndex((int)this + 0x10e8);
  *(undefined4 *)((int)this + iVar2 * 4 + 0x10f0) = 0;
  if (*(int *)((int)this + 0x10ec) < 1) {
    piVar3 = (int *)0x0;
  }
  else {
    piVar3 = (int *)**(undefined4 **)((int)this + 0x14f4);
    *(undefined4 *)((int)this + iVar2 * 4 + 0x10f0) = 1;
  }
  while (piVar3 != (int *)0x0) {
    if (*piVar3 == param_1) {
      piVar3[1] = 10;
      return;
    }
    iVar1 = *(int *)((int)this + iVar2 * 4 + 0x10f0);
    if (*(int *)((int)this + 0x10ec) <= iVar1) break;
    piVar3 = *(int **)(*(int *)((int)this + 0x14f4) + iVar1 * 4);
    *(int *)((int)this + iVar2 * 4 + 0x10f0) = iVar1 + 1;
  }
  piVar3 = operator_new(8);
  *piVar3 = param_1;
  piVar3[1] = 10;
  AthenaList_Append((void *)((int)this + 0x10e8),(int)piVar3);
  return;
}

