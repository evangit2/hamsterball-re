
void __thiscall RegKey_WriteBool(void *this,LPCSTR param_1,BYTE param_2)

{
  param_2 = param_2 == '\x01';
  RegSetValueExA(*(HKEY *)((int)this + 4),param_1,0,3,&param_2,1);
  return;
}

