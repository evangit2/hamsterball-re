
undefined4 __cdecl _check_file_access(LPCSTR param_1,byte param_2)

{
  DWORD DVar1;
  int *piVar2;
  ulong *puVar3;
  
  DVar1 = GetFileAttributesA(param_1);
  if (DVar1 == 0xffffffff) {
    DVar1 = GetLastError();
    __dosmaperr(DVar1);
  }
  else {
    if (((DVar1 & 1) == 0) || ((param_2 & 2) == 0)) {
      return 0;
    }
    piVar2 = __errno();
    *piVar2 = 0xd;
    puVar3 = __doserrno();
    *puVar3 = 5;
  }
  return 0xffffffff;
}

