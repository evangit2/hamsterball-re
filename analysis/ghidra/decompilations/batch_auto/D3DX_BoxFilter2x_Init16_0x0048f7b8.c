
void D3DX_BoxFilter2x_Init16(void)

{
  int iVar1;
  code *UNRECOVERED_JUMPTABLE;
  
  iVar1 = CPUID_IsMMXAvailable();
  if (iVar1 == 0) {
    PTR_D3DX_BoxFilter2x_Init_004fa638 = D3DX_BoxFilter2x_Scalar;
    UNRECOVERED_JUMPTABLE = D3DX_BoxFilter2x_NoMMX;
  }
  else {
    UNRECOVERED_JUMPTABLE = D3DX_BoxFilter2x_MMX;
    PTR_D3DX_BoxFilter2x_Init_004fa638 = D3DX_BoxFilter2x_MMX;
  }
  PTR_D3DX_BoxFilter2x_Init16_004fa63c = UNRECOVERED_JUMPTABLE;
                    /* WARNING: Could not recover jumptable at 0x0048f7e5. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}

