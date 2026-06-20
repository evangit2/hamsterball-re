
int __thiscall AthenaList_FindByValue(void *this,int param_1)

{
  int iVar1;
  int *piVar2;
  
  piVar2 = *(int **)((int)this + 0x40c);
  iVar1 = 0;
  if (0 < *(int *)((int)this + 4)) {
    do {
      if (*piVar2 == param_1) {
        return iVar1;
      }
      piVar2 = piVar2 + 1;
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)((int)this + 4));
  }
  return -1;
}

