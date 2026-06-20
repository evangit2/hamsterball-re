
void CRT_MTDeleteLocks(void)

{
  __mtdeletelocks();
  if (DAT_004fc830 != 0xffffffff) {
    TlsFree(DAT_004fc830);
    DAT_004fc830 = 0xffffffff;
  }
  return;
}

