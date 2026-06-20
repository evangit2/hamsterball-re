
void EffectShader_Dtor(void)

{
  undefined4 *extraout_ECX;
  uint uVar1;
  int unaff_EBP;
  uint uVar2;
  
  __security_init_cookie();
  *(undefined4 **)(unaff_EBP + -0x14) = extraout_ECX;
  *extraout_ECX = &PTR_EffectShader_DeletingDtor_004dc054;
  *(undefined4 *)(unaff_EBP + -4) = 0;
  if ((extraout_ECX[0x42f] != 0) && (extraout_ECX[0x430] != 0)) {
    uVar1 = extraout_ECX[0x427];
    uVar2 = extraout_ECX[0x428];
    *(undefined4 *)(unaff_EBP + -0x10) = extraout_ECX[0x430];
    if (uVar1 < uVar2) {
      do {
        for (uVar2 = extraout_ECX[0x424]; uVar2 < (uint)extraout_ECX[0x426]; uVar2 = uVar2 + 4) {
          _free(*(void **)(*(int *)(unaff_EBP + -0x10) + 4));
          *(int *)(unaff_EBP + -0x10) = *(int *)(unaff_EBP + -0x10) + 8;
        }
        uVar1 = uVar1 + 1;
      } while (uVar1 < (uint)extraout_ECX[0x428]);
    }
  }
  _free((void *)extraout_ECX[0x42e]);
  _free((void *)extraout_ECX[0x430]);
  *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
  D3DX_AssemblyOp8(extraout_ECX);
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return;
}

