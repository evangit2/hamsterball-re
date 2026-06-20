
undefined4 Base32_Decode(byte *param_1,char *param_2,short param_3)

{
  short sVar1;
  byte bVar2;
  uint uVar3;
  short sVar4;
  
  sVar4 = 0;
  sVar1 = 0;
  if (0 < param_3) {
    do {
      bVar2 = s__004f76b8[*param_2];
      if (bVar2 == 0x20) {
        return 0;
      }
      if (sVar4 == 0) {
        *param_1 = 0;
      }
      uVar3 = Bit_ShiftByte(bVar2,3 - sVar4);
      *param_1 = *param_1 | (byte)uVar3;
      if (sVar4 < 3) {
        sVar4 = sVar4 + 5;
      }
      else {
        sVar4 = sVar4 + -3;
        param_1 = param_1 + 1;
        if (sVar4 != 0) {
          *param_1 = bVar2 << (8U - (char)sVar4 & 0x1f);
        }
      }
      sVar1 = sVar1 + 1;
      param_2 = param_2 + 1;
    } while (sVar1 < param_3);
  }
  return 1;
}

