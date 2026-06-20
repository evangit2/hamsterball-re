
void __fastcall BugTracker_InitBrushes(int param_1)

{
  HBRUSH pHVar1;
  
  *(undefined1 *)(param_1 + 4) = 0;
  DAT_00534600 = SetUnhandledExceptionFilter(FUN_0047abe0);
  pHVar1 = CreateSolidBrush(100);
  *(HBRUSH *)(param_1 + 8) = pHVar1;
  pHVar1 = CreateSolidBrush(0x640000);
  *(HBRUSH *)(param_1 + 0xc) = pHVar1;
  return;
}

