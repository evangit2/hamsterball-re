
undefined4 * __fastcall BugTracker_Ctor(undefined4 *param_1)

{
  *param_1 = &PTR_BugTracker_DeletingDtor_004da864;
  param_1[6] = &PTR_AthenaCString_DeletingDtor_004d290c;
  param_1[7] = 0;
  param_1[9] = 0;
  *(undefined1 *)(param_1 + 0xc) = 1;
  param_1[8] = 0;
  param_1[0xb] = 0;
  *(undefined1 *)(param_1 + 10) = 0;
  AthenaString_AssignCStr(param_1 + 6,(char *)0x0);
  *(undefined1 *)(param_1 + 0x11) = 0;
  *(undefined1 *)((int)param_1 + 0x45) = 0;
  DAT_00534600 = 0;
  param_1[4] = 0;
  return param_1;
}

