
void D3DXMesh_ComputeLightingThenAssemble(void)

{
  undefined4 *extraout_ECX;
  int unaff_EBP;
  
  __security_init_cookie();
  *(undefined4 **)(unaff_EBP + -0x10) = extraout_ECX;
  *extraout_ECX = &PTR_LAB_004dc0bc;
  *(undefined4 *)(unaff_EBP + -4) = 0;
  D3DXMesh_ComputeLightingFromNormals((int)extraout_ECX);
  if ((void *)extraout_ECX[0x41b] != (void *)0x0) {
    _free((void *)extraout_ECX[0x41b]);
  }
  *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
  D3DX_AssemblyOp8(extraout_ECX);
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return;
}

