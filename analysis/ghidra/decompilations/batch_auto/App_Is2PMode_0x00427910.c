
uint __fastcall App_Is2PMode(int param_1)

{
  return (uint)(int)(char)(*(byte *)(*(int *)(param_1 + 0x520) + 0xc + param_1) |
                          *(byte *)(*(int *)(param_1 + 0x51c) + 0xc + param_1)) >> 7 & 1;
}

