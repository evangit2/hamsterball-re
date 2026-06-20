
bool __thiscall RegKey_ReadBool(void *this,LPCSTR param_1)

{
  LSTATUS LVar1;
  DWORD local_4;
  
  local_4 = 1;
  LVar1 = RegQueryValueExA(*(HKEY *)((int)this + 4),param_1,(LPDWORD)0x0,(LPDWORD)0x0,
                           (LPBYTE)&param_1,&local_4);
  if (LVar1 != 0) {
    return false;
  }
  return (char)param_1 == '\x01';
}

