
void __fastcall Judge_Reset(int param_1)

{
  uint uVar1;
  
  *(undefined1 *)(param_1 + 0x10f4) = 0;
  uVar1 = AthenaList_ContainsValue((void *)(*(int *)(param_1 + 0x10d0) + 0x2578),param_1);
  if ((char)uVar1 == '\0') {
    AthenaList_Append((void *)(*(int *)(param_1 + 0x10d0) + 0x2578),param_1);
  }
  return;
}

