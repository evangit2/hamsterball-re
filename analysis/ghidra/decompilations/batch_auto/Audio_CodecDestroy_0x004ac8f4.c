
void __cdecl Audio_CodecDestroy(int *param_1)

{
  int iVar1;
  
  iVar1 = 1;
  do {
    Audio_CodecFreeChannel(param_1,iVar1);
    iVar1 = iVar1 + -1;
  } while (-1 < iVar1);
  CRT_FreeParam2(param_1,(void *)param_1[1]);
  param_1[1] = 0;
  CRT_Noop2();
  return;
}

