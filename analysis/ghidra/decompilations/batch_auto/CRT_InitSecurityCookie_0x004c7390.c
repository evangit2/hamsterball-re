
void CRT_InitSecurityCookie(void)

{
  DWORD DVar1;
  DWORD DVar2;
  DWORD DVar3;
  LARGE_INTEGER local_14;
  _FILETIME local_c;
  
  GetSystemTimeAsFileTime(&local_c);
  DVar1 = GetCurrentProcessId();
  DVar2 = GetCurrentThreadId();
  DVar3 = GetTickCount();
  QueryPerformanceCounter(&local_14);
  DAT_004fce90 = local_c.dwHighDateTime ^ local_c.dwLowDateTime ^ DVar1 ^ DVar2 ^ DVar3 ^
                 local_14.s.HighPart ^ local_14.s.LowPart;
  if (DAT_004fce90 == 0) {
    DAT_004fce90 = 0xbb40e64e;
  }
  return;
}

