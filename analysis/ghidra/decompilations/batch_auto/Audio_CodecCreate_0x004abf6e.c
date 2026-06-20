
void __cdecl Audio_CodecCreate(int param_1)

{
  undefined4 *puVar1;
  
  puVar1 = (undefined4 *)(*(code *)**(undefined4 **)(param_1 + 4))(param_1,0,0x18);
  *(undefined4 **)(param_1 + 0x18c) = puVar1;
  *(undefined1 *)(puVar1 + 4) = 0;
  *(undefined1 *)((int)puVar1 + 0x11) = 0;
  *puVar1 = &LAB_004abe93;
  puVar1[1] = Audio_CodecReset;
  puVar1[2] = Audio_CodecStart;
  puVar1[3] = &LAB_004abf5d;
  *(undefined1 *)(puVar1 + 5) = 1;
  return;
}

