
void Unwind_004cdb00(void)

{
  int unaff_EBP;
  
  if ((*(uint *)(unaff_EBP + -0x44) & 1) != 0) {
    AthenaString_dtor(*(undefined4 **)(unaff_EBP + 4));
    return;
  }
  return;
}

