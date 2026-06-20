
undefined4 * __cdecl Color_AllocVertexBlock(int param_1)

{
  undefined4 *puVar1;
  int iVar2;
  undefined4 *puVar3;
  
  if (param_1 != 0) {
    puVar1 = Zlib_AllocState(2);
    if (puVar1 != (undefined4 *)0x0) {
      puVar3 = puVar1;
      for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
        *puVar3 = 0;
        puVar3 = puVar3 + 1;
      }
    }
    return puVar1;
  }
  return (undefined4 *)0x0;
}

