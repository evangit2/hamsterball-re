
void __fastcall ArenaBoard_TickDown(int *param_1)

{
  int iVar1;
  
  iVar1 = param_1[0x21f];
  param_1[0x21f] = iVar1 + -1;
  if (iVar1 + -1 < 1) {
    (**(code **)(*param_1 + 0x4c))();
                    /* WARNING: Could not recover jumptable at 0x00472a72. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (**(code **)(*param_1 + 0x40))();
    return;
  }
  return;
}

