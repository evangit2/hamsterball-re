
void __thiscall RNG_SeedSmall(void *this,uint param_1)

{
  uint *puVar1;
  int iVar2;
  
  puVar1 = (uint *)((int)this + 0xc);
  *puVar1 = param_1 & 0x3fffffff;
  *(undefined4 *)((int)this + 4) = 0;
  *(undefined4 *)((int)this + 8) = 0x1f;
  *(undefined4 *)((int)this + 0x10) = 1;
  iVar2 = 0x35;
  do {
    puVar1[2] = *puVar1 + puVar1[1] & 0x3fffffff;
    puVar1 = puVar1 + 1;
    iVar2 = iVar2 + -1;
  } while (iVar2 != 0);
  return;
}

