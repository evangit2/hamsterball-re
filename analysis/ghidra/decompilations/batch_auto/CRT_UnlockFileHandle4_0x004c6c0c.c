
void CRT_UnlockFileHandle4(void)

{
  int unaff_EBX;
  
  __unlock_fhandle(unaff_EBX);
  return;
}

