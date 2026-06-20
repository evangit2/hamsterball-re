
void __fastcall RegKey_Open(int param_1)

{
  PHKEY phkResult;
  LSTATUS LVar1;
  
  phkResult = (PHKEY)(param_1 + 4);
  LVar1 = RegOpenKeyExA((HKEY)&DAT_80000002,*(LPCSTR *)(param_1 + 8),0,0x20007,phkResult);
  if (LVar1 != 0) {
    LVar1 = RegOpenKeyExA((HKEY)&DAT_80000001,*(LPCSTR *)(param_1 + 8),0,0x20007,phkResult);
    if (LVar1 != 0) {
      RegCreateKeyA((HKEY)&DAT_80000001,*(LPCSTR *)(param_1 + 8),phkResult);
      RegOpenKeyExA((HKEY)&DAT_80000001,*(LPCSTR *)(param_1 + 8),0,0x20007,phkResult);
    }
  }
  return;
}

