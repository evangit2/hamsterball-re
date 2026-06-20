
void Unwind_004cde13(void)

{
  int unaff_EBP;
  
  if ((*(uint *)(unaff_EBP + -300) & 1) != 0) {
    AthenaString_dtor(*(undefined4 **)(unaff_EBP + 4));
    return;
  }
  return;
}

