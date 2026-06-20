
void __fastcall CRT_LeaveCriticalSection3(int param_1)

{
  int unaff_EBP;
  
  if (*(int *)(unaff_EBP + -0x1c) != param_1) {
    LeaveCriticalSection_indexed(param_1);
  }
  return;
}

