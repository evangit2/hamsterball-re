
void D3DX_ShaderDispatch_noarg(void)

{
  D3DX_DetectShaderProfile(1);
                    /* WARNING: Could not recover jumptable at 0x0045b528. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)PTR_D3DX_ShaderDispatch_noarg_004f71fc)();
  return;
}

