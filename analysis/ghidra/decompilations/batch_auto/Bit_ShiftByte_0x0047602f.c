
uint Bit_ShiftByte(byte param_1,short param_2)

{
  undefined4 in_EAX;
  uint uVar1;
  
  if (param_2 < 1) {
    if (param_2 < 0) {
      uVar1 = (uint)(param_1 >> (-(byte)param_2 & 0x1f));
    }
    else {
      uVar1 = CONCAT31((int3)((uint)in_EAX >> 8),param_1);
    }
  }
  else {
    uVar1 = (uint)param_1 << ((byte)param_2 & 0x1f);
  }
  return uVar1;
}

