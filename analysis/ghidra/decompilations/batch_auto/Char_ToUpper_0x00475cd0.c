
uint Char_ToUpper(byte param_1)

{
  undefined4 in_EAX;
  uint uVar1;
  undefined4 in_ECX;
  
  if ((param_1 < 0x41) || (0x5a < param_1)) {
    uVar1 = CONCAT31((int3)((uint)in_EAX >> 8),param_1);
  }
  else {
    uVar1 = DAT_004f7530 + CONCAT31((int3)((uint)in_ECX >> 8),param_1);
  }
  return uVar1;
}

