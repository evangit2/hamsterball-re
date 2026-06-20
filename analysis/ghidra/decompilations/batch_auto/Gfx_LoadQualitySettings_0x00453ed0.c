
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall Gfx_LoadQualitySettings(int param_1)

{
  char cVar1;
  bool bVar2;
  float10 fVar3;
  
  RegKey_Open(*(int *)(*(int *)(param_1 + 0x5c) + 0x54));
  cVar1 = RegKey_ReadString(*(void **)(*(int *)(param_1 + 0x5c) + 0x54),"Texture Quality");
  if (cVar1 != '\0') {
    fVar3 = Registry_ReadFloat(*(void **)(*(int *)(param_1 + 0x5c) + 0x54),"Texture Quality");
    *(float *)(param_1 + 0x184) = (float)fVar3;
  }
  cVar1 = RegKey_ReadString(*(void **)(*(int *)(param_1 + 0x5c) + 0x54),"ColorMode");
  if (cVar1 != '\0') {
    bVar2 = RegKey_ReadBool(*(void **)(*(int *)(param_1 + 0x5c) + 0x54),"ColorMode");
    *(bool *)(param_1 + 0x18c) = bVar2;
  }
  cVar1 = RegKey_ReadString(*(void **)(*(int *)(param_1 + 0x5c) + 0x54),"SafeMode");
  if (cVar1 != '\0') {
    fVar3 = Registry_ReadFloat(*(void **)(*(int *)(param_1 + 0x5c) + 0x54),"SafeMode");
    if (fVar3 != (float10)_DAT_004cf368) {
      *(undefined1 *)(param_1 + 0x7d7) = 1;
      RegKey_Close(*(int *)(*(int *)(param_1 + 0x5c) + 0x54));
      return;
    }
    *(undefined1 *)(param_1 + 0x7d7) = 0;
  }
  RegKey_Close(*(int *)(*(int *)(param_1 + 0x5c) + 0x54));
  return;
}

