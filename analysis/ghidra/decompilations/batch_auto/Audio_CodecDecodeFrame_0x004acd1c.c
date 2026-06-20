
void Audio_CodecDecodeFrame(void)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  undefined4 extraout_ECX;
  int *unaff_ESI;
  char local_8;
  
  iVar1 = unaff_ESI[0x5f];
  Audio_CodecComputeScaling(unaff_ESI);
  Huffman_InitDecodeTable();
  *(undefined4 *)(iVar1 + 0xc) = 0;
  uVar2 = Audio_CodecValidateConfig(extraout_ECX,(int)unaff_ESI);
  *(char *)(iVar1 + 0x10) = (char)uVar2;
  *(undefined4 *)(iVar1 + 0x14) = 0;
  *(undefined4 *)(iVar1 + 0x18) = 0;
  if ((*(char *)((int)unaff_ESI + 0x4a) == '\0') || ((char)unaff_ESI[0x10] == '\0')) {
    *(undefined1 *)(unaff_ESI + 0x16) = 0;
    *(undefined1 *)((int)unaff_ESI + 0x59) = 0;
    *(undefined1 *)((int)unaff_ESI + 0x5a) = 0;
  }
  if (*(char *)((int)unaff_ESI + 0x4a) == '\0') goto LAB_004acdba;
  if (*(char *)((int)unaff_ESI + 0x41) != '\0') {
    *(undefined4 *)(*unaff_ESI + 0x14) = 0x2e;
    (**(code **)*unaff_ESI)();
  }
  if (unaff_ESI[0x19] == 3) {
    if (unaff_ESI[0x1d] == 0) {
      if ((char)unaff_ESI[0x14] == '\0') goto LAB_004acd87;
      *(undefined1 *)((int)unaff_ESI + 0x5a) = 1;
    }
    else {
      *(undefined1 *)((int)unaff_ESI + 0x59) = 1;
    }
  }
  else {
    *(undefined1 *)((int)unaff_ESI + 0x59) = 0;
    *(undefined1 *)((int)unaff_ESI + 0x5a) = 0;
    unaff_ESI[0x1d] = 0;
LAB_004acd87:
    *(undefined1 *)(unaff_ESI + 0x16) = 1;
  }
  if ((char)unaff_ESI[0x16] != '\0') {
    Font_Init2(unaff_ESI);
    *(int *)(iVar1 + 0x14) = unaff_ESI[0x69];
  }
  if ((*(char *)((int)unaff_ESI + 0x5a) != '\0') || (*(char *)((int)unaff_ESI + 0x59) != '\0')) {
    SDF_Init(unaff_ESI);
    *(int *)(iVar1 + 0x18) = unaff_ESI[0x69];
  }
LAB_004acdba:
  if (*(char *)((int)unaff_ESI + 0x41) == '\0') {
    if (*(char *)(iVar1 + 0x10) == '\0') {
      D3D_CreateColorConverter(unaff_ESI);
      Audio_CreateUpsampler(unaff_ESI);
    }
    else {
      D3D_CreateYCbCrConverter((int)unaff_ESI);
    }
    Audio_CreateDecodeStream((int)unaff_ESI,*(char *)((int)unaff_ESI + 0x5a));
  }
  Audio_InitChannelDecoders(unaff_ESI);
  if (*(char *)((int)unaff_ESI + 0xc9) == '\0') {
    if ((char)unaff_ESI[0x32] == '\0') {
      Audio_InitLayer3((int)unaff_ESI);
    }
    else {
      Deflate_InitStream((int)unaff_ESI);
    }
  }
  else {
    *(undefined4 *)(*unaff_ESI + 0x14) = 1;
    (**(code **)*unaff_ESI)();
  }
  if ((*(char *)(unaff_ESI[99] + 0x10) != '\0') || (local_8 = '\0', (char)unaff_ESI[0x10] != '\0'))
  {
    local_8 = '\x01';
  }
  Audio_InitSynthesis((int)unaff_ESI,local_8);
  if (*(char *)((int)unaff_ESI + 0x41) == '\0') {
    Audio_AllocOutputBuffers(unaff_ESI,'\0');
  }
  (**(code **)(unaff_ESI[1] + 0x18))();
  (**(code **)(unaff_ESI[99] + 8))();
  if (((unaff_ESI[2] != 0) && ((char)unaff_ESI[0x10] == '\0')) &&
     (*(char *)(unaff_ESI[99] + 0x10) != '\0')) {
    iVar3 = unaff_ESI[8];
    if ((char)unaff_ESI[0x32] != '\0') {
      iVar3 = iVar3 * 3 + 2;
    }
    *(undefined4 *)(unaff_ESI[2] + 4) = 0;
    *(int *)(unaff_ESI[2] + 8) = unaff_ESI[0x46] * iVar3;
    *(undefined4 *)(unaff_ESI[2] + 0xc) = 0;
    *(uint *)(unaff_ESI[2] + 0x10) = (*(char *)((int)unaff_ESI + 0x5a) != '\0') + 2;
    *(int *)(iVar1 + 0xc) = *(int *)(iVar1 + 0xc) + 1;
  }
  return;
}

