
void __thiscall RegKey_QueryValue(void *this,LPCSTR param_1,LPBYTE param_2,DWORD param_3)

{
  RegQueryValueExA(*(HKEY *)((int)this + 4),param_1,(LPDWORD)0x0,(LPDWORD)0x0,param_2,&param_3);
  return;
}

