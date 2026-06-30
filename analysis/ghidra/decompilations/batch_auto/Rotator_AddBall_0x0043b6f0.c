
void __thiscall Rotator_AddBall(void *this,int param_1)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  
  iVar2 = AthenaList_NextIndex((int)this + 0x10f0);
  *(undefined4 *)((int)this + iVar2 * 4 + 0x10f8) = 0;
  if (*(int *)((int)this + 0x10f4) < 1) {
    piVar3 = (int *)0x0;
  }
  else {
    piVar3 = (int *)**(undefined4 **)((int)this + 0x14fc);
    *(undefined4 *)((int)this + iVar2 * 4 + 0x10f8) = 1;
  }
  while (piVar3 != (int *)0x0) {
    if (*piVar3 == param_1) {
      piVar3[1] = 10;
      return;
    }
    iVar1 = *(int *)((int)this + iVar2 * 4 + 0x10f8);
    if (*(int *)((int)this + 0x10f4) <= iVar1) break;
    piVar3 = *(int **)(*(int *)((int)this + 0x14fc) + iVar1 * 4);
    *(int *)((int)this + iVar2 * 4 + 0x10f8) = iVar1 + 1;
  }
  piVar3 = operator_new(8);
  *piVar3 = param_1;
  piVar3[1] = 10;
  AthenaList_Append((void *)((int)this + 0x10f0),(int)piVar3);
  return;
}

