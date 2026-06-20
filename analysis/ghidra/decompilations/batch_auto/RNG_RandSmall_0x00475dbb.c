
undefined4 RNG_RandSmall(void)

{
  uint uVar1;
  uint uVar2;
  
  uVar2 = (DAT_005341ec & 0xffff) * 0x41a7;
  uVar1 = (DAT_005341ec >> 0x10) * 0x41a7 + (uVar2 >> 0x10);
  DAT_005341ec = (uVar1 & 0x7fff) * 0x10000 + (uVar1 >> 0xf) + -0x7fffffff + (uVar2 & 0xffff);
  if ((int)DAT_005341ec < 0) {
    DAT_005341ec = DAT_005341ec + 0x7fffffff;
  }
  return CONCAT22((short)(DAT_005341ec >> 0x10),
                  -(ushort)((ushort)DAT_005341ec != 0x8000) & (ushort)DAT_005341ec);
}

