
void __cdecl Audio_CodecCreateContext(int param_1)

{
  undefined4 *puVar1;
  
  puVar1 = (undefined4 *)(*(code *)**(undefined4 **)(param_1 + 4))(param_1,1,0x1c);
  *(undefined4 **)(param_1 + 0x17c) = puVar1;
  *(undefined1 *)(puVar1 + 2) = 0;
  *puVar1 = &LAB_004acec9;
  puVar1[1] = &LAB_004acff6;
  Audio_CodecDecodeFrame();
  return;
}

