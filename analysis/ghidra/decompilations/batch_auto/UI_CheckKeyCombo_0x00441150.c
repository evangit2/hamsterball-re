
void __fastcall UI_CheckKeyCombo(int *param_1)

{
  undefined4 uVar1;
  
  NoOp();
  uVar1 = Input_CheckKeyCombo((void *)param_1[0x21e],2);
  if ((char)uVar1 == '\0') {
    *(undefined1 *)(param_1 + 0x229) = 0;
  }
  else if ((char)param_1[0x229] == '\0') {
                    /* WARNING: Could not recover jumptable at 0x00441178. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (**(code **)(*param_1 + 0x48))();
    return;
  }
  return;
}

