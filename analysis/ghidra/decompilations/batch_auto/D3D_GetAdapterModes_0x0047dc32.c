
void __thiscall D3D_GetAdapterModes(void *this,int param_1)

{
  uint uVar1;
  byte bVar2;
  int iVar3;
  undefined4 *puVar4;
  
  uVar1 = 0;
  if (*(int *)((int)this + 0x3c) != 0) {
    bVar2 = 0x10;
    do {
      *(undefined4 *)(param_1 + uVar1 * 4) =
           *(undefined4 *)(&DAT_004db348 + (*(uint *)this >> (bVar2 & 0x1f) & 3) * 4);
      uVar1 = uVar1 + 1;
      bVar2 = bVar2 + 2;
    } while (uVar1 < *(uint *)((int)this + 0x3c));
    if (7 < uVar1) {
      return;
    }
  }
  puVar4 = (undefined4 *)(param_1 + uVar1 * 4);
  for (iVar3 = 8 - uVar1; iVar3 != 0; iVar3 = iVar3 + -1) {
    *puVar4 = 0;
    puVar4 = puVar4 + 1;
  }
  return;
}

