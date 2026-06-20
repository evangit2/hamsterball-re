
void __fastcall App_ReadDisplaySettings(int param_1)

{
  char cVar1;
  bool bVar2;
  uint uVar3;
  
  RegKey_Open(*(int *)(param_1 + 0x54));
  cVar1 = RegKey_ReadString(*(void **)(param_1 + 0x54),"Fullscreen");
  if (cVar1 != '\0') {
    bVar2 = RegKey_ReadBool(*(void **)(param_1 + 0x54),"Fullscreen");
    *(bool *)(param_1 + 0x158) = bVar2;
  }
  cVar1 = RegKey_ReadString(*(void **)(param_1 + 0x54),"Screen Width");
  if (cVar1 != '\0') {
    uVar3 = RegKey_ReadDword(*(void **)(param_1 + 0x54),"Screen Width");
    *(uint *)(param_1 + 0x15c) = uVar3;
  }
  cVar1 = RegKey_ReadString(*(void **)(param_1 + 0x54),"Screen Height");
  if (cVar1 != '\0') {
    uVar3 = RegKey_ReadDword(*(void **)(param_1 + 0x54),"Screen Height");
    *(uint *)(param_1 + 0x160) = uVar3;
  }
  RegKey_Close(*(int *)(param_1 + 0x54));
  return;
}

