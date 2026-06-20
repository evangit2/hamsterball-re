
void __cdecl Audio_SelectHybridMode(int param_1)

{
  int iVar1;
  uint uVar2;
  
  iVar1 = *(int *)(param_1 + 0x184);
  if (*(int *)(iVar1 + 0x10) != 0) {
    if (*(char *)(param_1 + 0x49) != '\0') {
      uVar2 = Audio_ReadScaleFactors();
      if ((char)uVar2 != '\0') {
        *(undefined1 **)(iVar1 + 0xc) = &LAB_004b1ab1;
        goto LAB_004b1fff;
      }
    }
    *(code **)(iVar1 + 0xc) = Audio_ReadFrame;
  }
LAB_004b1fff:
  *(undefined4 *)(param_1 + 0x88) = 0;
  return;
}

