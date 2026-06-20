
undefined4 __fastcall AthenaList_NextIndex(int param_1)

{
  int iVar1;
  
  iVar1 = *(int *)(param_1 + 0x408) + 1;
  *(int *)(param_1 + 0x408) = iVar1;
  if (0xff < iVar1) {
    *(undefined4 *)(param_1 + 0x408) = 1;
  }
  return *(undefined4 *)(param_1 + 0x408);
}

