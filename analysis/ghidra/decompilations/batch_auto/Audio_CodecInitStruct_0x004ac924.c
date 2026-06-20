
void __cdecl Audio_CodecInitStruct(int *param_1)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  int iVar4;
  int *piVar5;
  
  param_1[1] = 0;
  uVar1 = ReturnZero();
  puVar2 = (undefined4 *)CRT_Malloc(param_1,0x50);
  if (puVar2 == (undefined4 *)0x0) {
    piVar5 = param_1;
    CRT_Noop2();
    *(undefined4 *)(*param_1 + 0x14) = 0x35;
    *(undefined4 *)(*param_1 + 0x18) = 0;
    (**(code **)*param_1)(param_1,piVar5);
  }
  else {
    *puVar2 = Audio_CodecAllocBuffer;
    puVar2[1] = Audio_CodecAllocSimple;
    puVar2[2] = Audio_CodecAllocSamples8;
    puVar2[3] = Audio_CodecAllocSamples16;
    puVar2[4] = Audio_CodecAddChannel8;
    puVar2[5] = Audio_CodecAddChannel16;
    puVar2[6] = Audio_CodecPrepareBuffers;
    puVar2[7] = Audio_CodecGetBuffer8;
    puVar2[8] = Audio_CodecGetBuffer16;
    puVar2[9] = Audio_CodecFreeChannel;
    puVar2[10] = Audio_CodecDestroy;
    puVar2[0xb] = uVar1;
    puVar3 = puVar2 + 0xf;
    iVar4 = 2;
    do {
      puVar3[-2] = 0;
      *puVar3 = 0;
      puVar3 = puVar3 + -1;
      iVar4 = iVar4 + -1;
    } while (iVar4 != 0);
    puVar2[0x10] = 0;
    puVar2[0x11] = 0;
    puVar2[0x12] = 0x50;
    param_1[1] = (int)puVar2;
  }
  return;
}

