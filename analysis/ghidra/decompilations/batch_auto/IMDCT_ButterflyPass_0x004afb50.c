
void __cdecl IMDCT_ButterflyPass(void *param_1,int param_2,int param_3)

{
  void *this;
  byte bVar1;
  int iVar2;
  undefined4 local_8;
  undefined4 local_4;
  
  local_4 = 7 - param_3;
  bVar1 = 0;
  if (0 < local_4) {
    do {
      local_8 = 1 << (bVar1 & 0x1f);
      if (0 < local_8) {
        this = param_1;
        do {
          MDCT_Butterfly(this,4 << ((char)param_3 + bVar1 & 0x1f));
          this = (void *)((int)this + (param_2 >> (bVar1 & 0x1f)) * 4);
          local_8 = local_8 + -1;
        } while (local_8 != 0);
      }
      bVar1 = bVar1 + 1;
      local_4 = local_4 + -1;
    } while (local_4 != 0);
  }
  if (0 < param_2) {
    iVar2 = (param_2 - 1U >> 5) + 1;
    do {
      DCT_Stage32();
      iVar2 = iVar2 + -1;
    } while (iVar2 != 0);
  }
  return;
}

