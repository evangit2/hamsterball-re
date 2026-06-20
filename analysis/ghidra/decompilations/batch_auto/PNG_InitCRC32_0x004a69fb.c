
void __cdecl PNG_InitCRC32(int param_1)

{
  uint uVar1;
  
  uVar1 = CRC32_Compute(0,(byte *)0x0,0);
  *(uint *)(param_1 + 0x100) = uVar1;
  return;
}

