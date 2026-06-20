
void __fastcall Timer_Decrement(int param_1)

{
  *(int *)(param_1 + 0x10) = *(int *)(param_1 + 0x1c) + -100;
  *(undefined1 *)(param_1 + 0x2a) = 1;
  return;
}

