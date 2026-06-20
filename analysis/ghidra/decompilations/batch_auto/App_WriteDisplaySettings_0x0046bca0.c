
void __fastcall App_WriteDisplaySettings(int param_1)

{
  RegKey_Open(*(int *)(param_1 + 0x54));
  RegKey_WriteBool(*(void **)(param_1 + 0x54),"Fullscreen",*(BYTE *)(param_1 + 0x158));
  RegKey_WriteDword(*(void **)(param_1 + 0x54),"Screen Width",*(undefined4 *)(param_1 + 0x15c));
  RegKey_WriteDword(*(void **)(param_1 + 0x54),"Screen Height",*(undefined4 *)(param_1 + 0x160));
  RegKey_Close(*(int *)(param_1 + 0x54));
  return;
}

