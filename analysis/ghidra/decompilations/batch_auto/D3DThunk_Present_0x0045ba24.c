
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void D3DThunk_Present(void)

{
  D3DX_DetectShaderProfile(1);
                    /* WARNING: Could not recover jumptable at 0x0045ba2b. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*_DAT_004f7240)();
  return;
}

