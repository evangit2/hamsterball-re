
void __cdecl Matrix_IdentitySSE(undefined8 *param_1)

{
  ulonglong uVar1;
  
  uVar1 = (ulonglong)DAT_004dbc50;
  *param_1 = uVar1;
  param_1[1] = 0;
  param_1[3] = 0;
  param_1[4] = 0;
  param_1[5] = uVar1;
  param_1[6] = 0;
  param_1[2] = uVar1 << 0x20;
  param_1[7] = uVar1 << 0x20;
  FastExitMediaState();
  return;
}

