
undefined4 __fastcall StreamReader_InitFromBuffer(undefined4 *param_1)

{
  undefined4 *in_EAX;
  
  *param_1 = 0;
  param_1[1] = 0;
  param_1[2] = 0;
  param_1[3] = 0;
  param_1[4] = 0;
  if (in_EAX != (undefined4 *)0x0) {
    *param_1 = in_EAX;
    param_1[1] = in_EAX;
    param_1[3] = 0;
    param_1[4] = in_EAX[2];
    param_1[2] = in_EAX[1] + *(int *)*in_EAX;
    return 0;
  }
  return 0xffffffff;
}

