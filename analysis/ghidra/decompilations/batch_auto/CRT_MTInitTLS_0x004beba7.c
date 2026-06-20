
undefined4 CRT_MTInitTLS(void)

{
  int iVar1;
  DWORD *lpTlsValue;
  BOOL BVar2;
  DWORD DVar3;
  
  iVar1 = __mtinitlocks();
  if (iVar1 != 0) {
    DAT_004fc830 = TlsAlloc();
    if (DAT_004fc830 != 0xffffffff) {
      lpTlsValue = _calloc(1,0x88);
      if (lpTlsValue != (DWORD *)0x0) {
        BVar2 = TlsSetValue(DAT_004fc830,lpTlsValue);
        if (BVar2 != 0) {
          lpTlsValue[0x15] = (DWORD)&DAT_004fc8d8;
          lpTlsValue[5] = 1;
          DVar3 = GetCurrentThreadId();
          lpTlsValue[1] = 0xffffffff;
          *lpTlsValue = DVar3;
          return 1;
        }
      }
      CRT_MTDeleteLocks();
      return 0;
    }
  }
  CRT_MTDeleteLocks();
  return 0;
}

