
void __cdecl Audio_CodecSetVolume(int param_1)

{
  (**(code **)(*(int *)(param_1 + 4) + 0x24))(param_1,1);
  *(uint *)(param_1 + 0x10) = (-(uint)(*(char *)(param_1 + 0xc) != '\0') & 100) + 100;
  return;
}

