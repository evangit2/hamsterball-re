
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void D3DThunk_AcquireThreadFocus(void)

{
  D3DX_DetectShaderProfile(1);
                    /* WARNING: Could not recover jumptable at 0x0045b10b. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*_DAT_004f7228)();
  return;
}

