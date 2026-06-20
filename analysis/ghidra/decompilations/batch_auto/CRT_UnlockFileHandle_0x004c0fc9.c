
void CRT_UnlockFileHandle(void)

{
  int unaff_EBX;
  
  __unlock_fhandle(unaff_EBX);
  return;
}

