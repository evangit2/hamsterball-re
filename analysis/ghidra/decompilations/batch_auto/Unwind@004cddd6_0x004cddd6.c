
void Unwind_004cddd6(void)

{
  int unaff_EBP;
  
  if ((*(uint *)(unaff_EBP + -0xc4) & 1) != 0) {
    AthenaString_dtor(*(undefined4 **)(unaff_EBP + 4));
    return;
  }
  return;
}

