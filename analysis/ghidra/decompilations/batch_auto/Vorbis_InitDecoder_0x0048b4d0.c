
undefined4 __cdecl Vorbis_InitDecoder(int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  
  Vorbis_InitDecodeChannel(param_2);
  iVar1 = *(int *)(param_1 + 0x48);
  if (((iVar1 != 0) && (*(int *)(param_1 + 4) != 0)) &&
     (iVar2 = *(int *)(*(int *)(param_1 + 4) + 0x1c), iVar2 != 0)) {
    iVar2 = *(int *)(iVar2 + 4) / 2;
    *(int *)(param_1 + 0x30) = iVar2;
    *(int *)(param_1 + 0x14) = iVar2;
    *(undefined4 *)(param_1 + 0x18) = 0xffffffff;
    *(undefined4 *)(param_1 + 0x38) = 0xffffffff;
    *(undefined4 *)(param_1 + 0x3c) = 0xffffffff;
    *(undefined4 *)(param_1 + 0x40) = 0xffffffff;
    *(undefined4 *)(param_1 + 0x44) = 0xffffffff;
    *(undefined4 *)(iVar1 + 0x10) = 0xffffffff;
    *(undefined4 *)(iVar1 + 0x14) = 0xffffffff;
  }
  return 0;
}

