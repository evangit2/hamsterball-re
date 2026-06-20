
char __thiscall RegKey_ReadString(void *this,LPCSTR param_1)

{
  LSTATUS LVar1;
  DWORD local_104;
  BYTE local_100 [256];
  
  local_104 = 1;
  LVar1 = RegQueryValueExA(*(HKEY *)((int)this + 4),param_1,(LPDWORD)0x0,(LPDWORD)0x0,local_100,
                           &local_104);
  if (LVar1 != 0) {
    local_104 = 4;
    LVar1 = RegQueryValueExA(*(HKEY *)((int)this + 4),param_1,(LPDWORD)0x0,(LPDWORD)0x0,local_100,
                             &local_104);
    if (LVar1 != 0) {
      local_104 = 8;
      LVar1 = RegQueryValueExA(*(HKEY *)((int)this + 4),param_1,(LPDWORD)0x0,(LPDWORD)0x0,local_100,
                               &local_104);
      if (LVar1 != 0) {
        LVar1 = RegQueryValueExA(*(HKEY *)((int)this + 4),param_1,(LPDWORD)0x0,(LPDWORD)0x0,
                                 (LPBYTE)0x0,&local_104);
        return '\x01' - (LVar1 != 0);
      }
    }
  }
  return '\x01';
}

