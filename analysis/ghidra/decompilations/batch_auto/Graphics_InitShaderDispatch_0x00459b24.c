
void Graphics_InitShaderDispatch(void)

{
  D3DX_DetectShaderProfile(1);
                    /* WARNING: Could not recover jumptable at 0x00459b2b. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)PTR_Graphics_InitShaderDispatch_004f71ac)();
  return;
}

