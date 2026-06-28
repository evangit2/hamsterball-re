
void __fastcall ToggleTimer_Tick(int param_1)

{
  int iVar1;
  
  iVar1 = *(int *)(param_1 + 0xc) + 1;
  *(undefined1 *)(param_1 + 0x10) = 0;
  *(int *)(param_1 + 0xc) = iVar1;
  if (*(int *)(param_1 + 8) <= iVar1) {
    *(undefined4 *)(param_1 + 0xc) = 0;
    *(bool *)(param_1 + 4) = *(char *)(param_1 + 4) == '\0';
    *(undefined1 *)(param_1 + 0x10) = 1;
  }
  return;
}

