
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void D3DThunk_SetVertexShader(void)

{
  D3DX_DetectShaderProfile(1);
                    /* WARNING: Could not recover jumptable at 0x0045bda9. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*_DAT_004f7244)();
  return;
}

