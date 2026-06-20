
void Unwind_004cd7b8(void)

{
  int unaff_EBP;
  
  if ((*(uint *)(unaff_EBP + -0x2c) & 1) != 0) {
    StreamReaderVtbl_Init(*(undefined4 **)(unaff_EBP + 4));
    return;
  }
  return;
}

