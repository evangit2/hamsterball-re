
void __fastcall Level_ReadSoundVolume(int param_1)

{
  char cVar1;
  float10 fVar2;
  
  RegKey_Open(*(int *)(*(int *)(param_1 + 0x834) + 0x54));
  cVar1 = RegKey_ReadString(*(void **)(*(int *)(param_1 + 0x834) + 0x54),"Sound Volume");
  if (cVar1 != '\0') {
    fVar2 = Registry_ReadFloat(*(void **)(*(int *)(param_1 + 0x834) + 0x54),"Sound Volume");
    *(float *)(param_1 + 0x838) = (float)fVar2;
    RegKey_Close(*(int *)(*(int *)(param_1 + 0x834) + 0x54));
    return;
  }
  *(undefined4 *)(param_1 + 0x838) = 0x3f800000;
  RegKey_Close(*(int *)(*(int *)(param_1 + 0x834) + 0x54));
  return;
}

