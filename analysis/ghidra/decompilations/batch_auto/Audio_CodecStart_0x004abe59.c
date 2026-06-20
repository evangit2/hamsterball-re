
void __cdecl Audio_CodecStart(int param_1)

{
  Audio_CodecMapChannels();
  Audio_CodecResolveChannels();
  (*(code *)**(undefined4 **)(param_1 + 0x194))(param_1);
  (*(code *)**(undefined4 **)(param_1 + 0x184))(param_1);
  **(undefined4 **)(param_1 + 0x18c) = *(undefined4 *)(*(int *)(param_1 + 0x184) + 4);
  return;
}

