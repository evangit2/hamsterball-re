
void __cdecl Array_CopyBackward(undefined4 *param_1,int param_2,int param_3,undefined4 *param_4)

{
  for (; param_2 != param_3; param_3 = param_3 + -4) {
    param_4 = param_4 + -1;
    *param_4 = *(undefined4 *)(param_3 + -4);
  }
  *param_1 = param_4;
  return;
}

