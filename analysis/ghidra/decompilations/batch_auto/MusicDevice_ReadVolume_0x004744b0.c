
void __fastcall MusicDevice_ReadVolume(int param_1)

{
  char cVar1;
  float10 fVar2;
  
  RegKey_Open(*(int *)(*(int *)(param_1 + 4) + 0x54));
  cVar1 = RegKey_ReadString(*(void **)(*(int *)(param_1 + 4) + 0x54),"Music Volume");
  if (cVar1 != '\0') {
    fVar2 = Registry_ReadFloat(*(void **)(*(int *)(param_1 + 4) + 0x54),"Music Volume");
    *(float *)(param_1 + 8) = (float)fVar2;
    RegKey_Close(*(int *)(*(int *)(param_1 + 4) + 0x54));
    return;
  }
  *(undefined4 *)(param_1 + 8) = 0x3f4ccccd;
  RegKey_Close(*(int *)(*(int *)(param_1 + 4) + 0x54));
  return;
}

