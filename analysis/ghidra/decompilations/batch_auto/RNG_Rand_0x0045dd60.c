
int __thiscall RNG_Rand(void *this,int param_1,char param_2)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  
  if (param_1 != 0) {
    if (param_1 < 0) {
      param_1 = -param_1;
    }
    iVar3 = 2;
    if (2 < param_1) {
      do {
        iVar3 = iVar3 * 2;
      } while (iVar3 < param_1);
    }
    iVar2 = *(int *)((int)this + 4);
    iVar4 = *(int *)((int)this + 8);
    uVar1 = *(int *)((int)this + iVar4 * 4 + 0xc) + *(int *)((int)this + iVar2 * 4 + 0xc) &
            0x3fffffff;
    *(uint *)((int)this + iVar2 * 4 + 0xc) = uVar1;
    iVar2 = iVar2 + 1;
    if (iVar2 == 0x37) {
      iVar2 = 0;
    }
    iVar4 = iVar4 + 1;
    if (iVar4 == 0x37) {
      iVar4 = 0;
    }
    *(int *)((int)this + 4) = iVar2;
    iVar3 = (int)((int)uVar1 >> 6 & iVar3 - 1U) % param_1;
    *(int *)((int)this + 8) = iVar4;
    if ((param_2 == '\x01') && (iVar2 = RNG_Rand(this,2,'\0'), iVar2 == 1)) {
      iVar3 = -iVar3;
    }
    return iVar3;
  }
  return 0;
}

