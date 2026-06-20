
void __thiscall Registry_SetValue(void *this,LPCSTR param_1,BYTE *param_2,DWORD param_3)

{
  RegSetValueExA(*(HKEY *)((int)this + 4),param_1,0,3,param_2,param_3);
  return;
}

