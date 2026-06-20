
void __cdecl Vorbis_InitDecodeState(undefined4 param_1,undefined4 *param_2)

{
  int iVar1;
  undefined4 *puVar2;
  
  puVar2 = param_2;
  for (iVar1 = 0x18; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar2 = 0;
    puVar2 = puVar2 + 1;
  }
  param_2[0x12] = param_1;
  param_2[0x15] = 0;
  param_2[0x13] = 0;
  return;
}

