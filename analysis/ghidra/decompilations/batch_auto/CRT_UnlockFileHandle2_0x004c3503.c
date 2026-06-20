
void CRT_UnlockFileHandle2(void)

{
  int unaff_EBX;
  
  __unlock_fhandle(unaff_EBX);
  return;
}

