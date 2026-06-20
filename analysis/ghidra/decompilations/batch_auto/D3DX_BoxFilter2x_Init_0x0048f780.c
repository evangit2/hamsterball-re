
void D3DX_BoxFilter2x_Init(void)

{
  int iVar1;
  
  iVar1 = CPUID_IsMMXAvailable();
  if (iVar1 == 0) {
    PTR_D3DX_BoxFilter2x_Init_004fa638 = D3DX_BoxFilter2x_Scalar;
    PTR_D3DX_BoxFilter2x_Init16_004fa63c = D3DX_BoxFilter2x_NoMMX;
  }
  else {
    PTR_D3DX_BoxFilter2x_Init_004fa638 = D3DX_BoxFilter2x_MMX;
    PTR_D3DX_BoxFilter2x_Init16_004fa63c = D3DX_BoxFilter2x_MMX;
  }
                    /* WARNING: Could not recover jumptable at 0x0048f7b2. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)PTR_D3DX_BoxFilter2x_Init_004fa638)();
  return;
}

