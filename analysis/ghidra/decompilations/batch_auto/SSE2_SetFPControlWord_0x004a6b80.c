
void __fastcall SSE2_SetFPControlWord(undefined4 param_1)

{
  undefined2 in_FPUControlWord;
  undefined4 local_8;
  
  local_8 = CONCAT22((short)((uint)param_1 >> 0x10),in_FPUControlWord);
  DAT_00535280 = local_8;
  return;
}

