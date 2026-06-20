
uint __thiscall RegKey_ReadDword(void *this,LPCSTR param_1)

{
  LSTATUS LVar1;
  DWORD local_4;
  
  local_4 = 4;
  LVar1 = RegQueryValueExA(*(HKEY *)((int)this + 4),param_1,(LPDWORD)0x0,(LPDWORD)0x0,
                           (LPBYTE)&param_1,&local_4);
  return ~-(uint)(LVar1 != 0) & (uint)param_1;
}

