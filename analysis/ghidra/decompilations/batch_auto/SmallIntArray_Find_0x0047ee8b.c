
int __thiscall SmallIntArray_Find(void *this,int param_1)

{
  uint uVar1;
  uint3 uVar2;
  int *piVar3;
  uint uVar4;
  
  uVar1 = *(uint *)((int)this + 0xc);
  uVar4 = 0;
  uVar2 = (uint3)(uVar1 >> 8);
  if (uVar1 != 0) {
    piVar3 = *(int **)((int)this + 0x10);
    do {
      if (*piVar3 == param_1) {
        return CONCAT31(uVar2,1);
      }
      uVar4 = uVar4 + 1;
      piVar3 = piVar3 + 1;
    } while (uVar4 < uVar1);
  }
  return (uint)uVar2 << 8;
}

