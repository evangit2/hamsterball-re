
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 __thiscall Registry_ReadFloat(void *param_1,LPCSTR param_2)

{
  LSTATUS LVar1;
  DWORD local_4;
  
  local_4 = 4;
  LVar1 = RegQueryValueExA(*(HKEY *)((int)param_1 + 4),param_2,(LPDWORD)0x0,(LPDWORD)0x0,
                           (LPBYTE)&param_2,&local_4);
  if (LVar1 != 0) {
    return (float10)_DAT_004cf368;
  }
  return (float10)(float)param_2;
}

