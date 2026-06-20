
void __cdecl Vorbis_FreeDecodeState(undefined4 *param_1)

{
  int iVar1;
  
  Vorbis_FlushDecodeBuffer((int)param_1);
  if ((void *)param_1[0x13] != (void *)0x0) {
    _free((void *)param_1[0x13]);
  }
  for (iVar1 = 0x18; iVar1 != 0; iVar1 = iVar1 + -1) {
    *param_1 = 0;
    param_1 = param_1 + 1;
  }
  return;
}

