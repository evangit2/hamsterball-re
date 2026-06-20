
void __fastcall RegKey_Close(int param_1)

{
  if (*(HKEY *)(param_1 + 4) != (HKEY)0x0) {
    RegCloseKey(*(HKEY *)(param_1 + 4));
  }
  *(undefined4 *)(param_1 + 4) = 0;
  return;
}

