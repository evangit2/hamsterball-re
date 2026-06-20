
void CRT_UnlockFileHandle3(void)

{
  int unaff_EBX;
  
  __unlock_fhandle(unaff_EBX);
  return;
}

