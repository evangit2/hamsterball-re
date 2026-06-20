
void Unwind_004cd6db(void)

{
  int unaff_EBP;
  
  if ((*(uint *)(unaff_EBP + -0x88) & 1) != 0) {
    AthenaString_dtor(*(undefined4 **)(unaff_EBP + 4));
    return;
  }
  return;
}

