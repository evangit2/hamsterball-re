
uint __thiscall AthenaList_ContainsValue(void *this,int param_1)

{
  int *piVar1;
  int iVar2;
  short sVar3;
  
  piVar1 = *(int **)((int)this + 0x40c);
  iVar2 = *(int *)((int)this + 4);
  sVar3 = 0;
  if (0 < iVar2) {
    do {
      if (*piVar1 == param_1) {
        sVar3 = sVar3 + 1;
      }
      piVar1 = piVar1 + 1;
      iVar2 = iVar2 + -1;
    } while (iVar2 != 0);
    if (0 < sVar3) {
      return CONCAT31((int3)((uint)piVar1 >> 8),1);
    }
  }
  return (uint)piVar1 & 0xffffff00;
}

