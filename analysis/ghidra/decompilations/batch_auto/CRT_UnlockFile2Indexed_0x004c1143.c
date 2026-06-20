
void CRT_UnlockFile2Indexed(void)

{
  int unaff_ESI;
  
  __unlock_file2(unaff_ESI,*(void **)(DAT_00535988 + unaff_ESI * 4));
  return;
}

