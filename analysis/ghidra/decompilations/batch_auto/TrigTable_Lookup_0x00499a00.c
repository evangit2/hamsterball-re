
undefined * __cdecl TrigTable_Lookup(int param_1,int param_2)

{
  if (param_1 == 0) {
    if (param_2 < 0x201) {
      if (param_2 == 0x200) {
        return &DAT_004dcac0;
      }
      switch(param_2) {
      case 0x20:
        return &DAT_004dc340;
      case 0x40:
        return &DAT_004dc3c0;
      case 0x80:
        return &DAT_004dc4c0;
      case 0x100:
        return &DAT_004dc6c0;
      }
    }
    else {
      if (param_2 == 0x400) {
        return &DAT_004dd2c0;
      }
      if (param_2 == 0x800) {
        return &DAT_004de2c0;
      }
      if (param_2 == 0x1000) {
        return &DAT_004e02c0;
      }
    }
  }
  return (undefined *)0x0;
}

