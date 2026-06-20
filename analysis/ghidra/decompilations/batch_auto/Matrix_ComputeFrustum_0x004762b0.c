
void __fastcall Matrix_ComputeFrustum(int param_1)

{
  float unaff_ESI;
  float unaff_EDI;
  int iVar1;
  undefined4 uStack_90;
  undefined4 uStack_8c;
  undefined4 uStack_88;
  undefined4 uStack_84;
  undefined4 uStack_80;
  undefined4 uStack_7c;
  undefined4 uStack_78;
  undefined4 uStack_74;
  undefined4 uStack_70;
  undefined4 uStack_6c;
  undefined4 uStack_68;
  undefined4 uStack_64;
  undefined4 uStack_60;
  undefined4 uStack_5c;
  
  Graphics_SetRenderState();
  Graphics_SetRenderState();
  *(float *)(param_1 + 0x28) = unaff_EDI + uStack_8c;
  *(float *)(param_1 + 0x2c) = uStack_88 + uStack_7c;
  *(float *)(param_1 + 0x30) = uStack_78 + uStack_6c;
  *(float *)(param_1 + 0x34) = uStack_68 + uStack_5c;
  *(float *)(param_1 + 0x38) = uStack_8c - unaff_EDI;
  *(float *)(param_1 + 0x3c) = uStack_7c - uStack_88;
  *(float *)(param_1 + 0x40) = uStack_6c - uStack_78;
  *(float *)(param_1 + 0x44) = uStack_5c - uStack_68;
  *(float *)(param_1 + 8) = uStack_8c - unaff_ESI;
  *(float *)(param_1 + 0xc) = uStack_7c - uStack_84;
  *(float *)(param_1 + 0x10) = uStack_6c - uStack_74;
  *(float *)(param_1 + 0x14) = uStack_5c - uStack_64;
  *(float *)(param_1 + 0x48) = uStack_90 + uStack_8c;
  *(float *)(param_1 + 0x4c) = uStack_80 + uStack_7c;
  *(float *)(param_1 + 0x50) = uStack_70 + uStack_6c;
  *(float *)(param_1 + 0x54) = uStack_60 + uStack_5c;
  *(float *)(param_1 + 0x18) = unaff_ESI + uStack_8c;
  *(float *)(param_1 + 0x1c) = uStack_84 + uStack_7c;
  *(float *)(param_1 + 0x20) = uStack_74 + uStack_6c;
  *(float *)(param_1 + 0x24) = uStack_64 + uStack_5c;
  *(float *)(param_1 + 0x58) = uStack_8c - uStack_90;
  *(float *)(param_1 + 0x5c) = uStack_7c - uStack_80;
  *(float *)(param_1 + 0x60) = uStack_6c - uStack_70;
  *(float *)(param_1 + 100) = uStack_5c - uStack_60;
  iVar1 = 6;
  do {
    D3D_ThunkIndirect();
    iVar1 = iVar1 + -1;
  } while (iVar1 != 0);
  return;
}

