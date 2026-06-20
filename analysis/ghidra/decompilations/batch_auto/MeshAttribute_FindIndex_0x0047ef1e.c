
int __thiscall MeshAttribute_FindIndex(void *this,short param_1)

{
  uint uVar1;
  uint3 uVar2;
  short *psVar3;
  uint uVar4;
  
  uVar1 = *(uint *)((int)this + 0xc);
  uVar4 = 0;
  uVar2 = (uint3)(uVar1 >> 8);
  if (uVar1 != 0) {
    psVar3 = *(short **)((int)this + 0x10);
    do {
      if (*psVar3 == param_1) {
        return CONCAT31(uVar2,1);
      }
      uVar4 = uVar4 + 1;
      psVar3 = psVar3 + 1;
    } while (uVar4 < uVar1);
  }
  return (uint)uVar2 << 8;
}

