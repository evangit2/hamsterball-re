
void Base32_Encode(byte *param_1,undefined1 *param_2,short param_3)

{
  uint uVar1;
  byte bVar2;
  short sVar3;
  byte bStack0000000f;
  int local_8;
  
  bVar2 = 0;
  if (0 < param_3) {
    local_8 = (int)param_3;
    sVar3 = 3;
    do {
      uVar1 = Bit_ShiftByte(0xf8U >> (bVar2 & 0x1f) & *param_1,sVar3);
      bStack0000000f = (byte)uVar1;
      if (sVar3 < 1) {
        bVar2 = bVar2 - 3;
        sVar3 = sVar3 + 3;
        param_1 = param_1 + 1;
        bStack0000000f =
             bStack0000000f |
             (byte)((int)(0xf8 << (-bVar2 + 5 & 0x1f) & (uint)*param_1) >> (-bVar2 + 8 & 0x1f));
      }
      else {
        bVar2 = bVar2 + 5;
        sVar3 = sVar3 + -5;
      }
      local_8 = local_8 + -1;
      *param_2 = (&DAT_004f7698)[bStack0000000f];
      param_2 = param_2 + 1;
    } while (local_8 != 0);
  }
  return;
}

