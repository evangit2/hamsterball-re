
void __fastcall Graphics_SetStreamBuffers(int param_1)

{
  int *unaff_ESI;
  
  D3DX_ShaderDispatch2();
  D3DX_ShaderDispatch2();
  D3DX_ShaderDispatch_noarg5();
  *unaff_ESI = param_1 + 0x2a4;
  unaff_ESI[1] = param_1 + 0x264;
  unaff_ESI[2] = param_1 + 0x224;
  return;
}

