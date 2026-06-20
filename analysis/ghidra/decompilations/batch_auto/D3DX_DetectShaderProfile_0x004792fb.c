
int D3DX_DetectShaderProfile(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined **ppuVar3;
  undefined **ppuVar4;
  
  if (param_1 == 0) {
    DAT_004f77bc = 0xffff;
    ppuVar3 = &PTR_Matrix_TransformVec4x3_004f7278;
    ppuVar4 = &PTR_LAB_004f7190;
    for (iVar2 = 0x39; iVar2 != 0; iVar2 = iVar2 + -1) {
      *ppuVar4 = *ppuVar3;
      ppuVar3 = ppuVar3 + 1;
      ppuVar4 = ppuVar4 + 1;
    }
  }
  else if (DAT_004f77bc == 0xffff) {
    DAT_004f77bc = 0;
    ppuVar3 = &PTR_Matrix_TransformVec4x3_004f7278;
    ppuVar4 = &PTR_LAB_004f7190;
    for (iVar2 = 0x39; iVar2 != 0; iVar2 = iVar2 + -1) {
      *ppuVar4 = *ppuVar3;
      ppuVar3 = ppuVar3 + 1;
      ppuVar4 = ppuVar4 + 1;
    }
    Matrix_InitVTable(&PTR_LAB_004f7190);
    iVar2 = Reg_ReadD3DValue(4,"DisableD3DXPSGP",(LPBYTE)&param_1);
    if (iVar2 == 0) {
      param_1 = 0;
    }
    if (param_1 != 1) {
      uVar1 = CPUID_CheckProcessorFeature(7);
      if (uVar1 == 0) {
        uVar1 = CPUID_CheckProcessorFeature(10);
        if (uVar1 == 0) {
          uVar1 = CPUID_CheckProcessorFeature(6);
          if (uVar1 != 0) {
            D3DX_InitFloatMathTable(&PTR_LAB_004f7190);
            DAT_004f77bc = 3;
          }
        }
        else {
          D3DX_InitFloatMathTableSSE(&PTR_LAB_004f7190);
          DAT_004f77bc = 2;
        }
      }
      else {
        D3DX_InitMathDispatchTable(&PTR_LAB_004f7190);
        DAT_004f77bc = 1;
      }
    }
  }
  return DAT_004f77bc;
}

