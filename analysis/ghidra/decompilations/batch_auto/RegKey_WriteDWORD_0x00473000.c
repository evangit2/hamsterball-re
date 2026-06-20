
void __thiscall RegKey_WriteDWORD(void *this,LPCSTR param_1,undefined4 param_2)

{
  RegSetValueExA(*(HKEY *)((int)this + 4),param_1,0,3,(BYTE *)&param_2,4);
  return;
}

