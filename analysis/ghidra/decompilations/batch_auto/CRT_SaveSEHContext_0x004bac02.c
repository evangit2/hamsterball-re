
void CRT_SaveSEHContext(void)

{
  undefined4 in_EAX;
  int unaff_EBP;
  
  DAT_004fc46c = *(undefined4 *)(unaff_EBP + 8);
  DAT_004fc468 = in_EAX;
  DAT_004fc470 = unaff_EBP;
  return;
}

