
void __fastcall RaceResults_DtorPhase1(undefined4 *param_1)

{
  void *pvStack_c;
  undefined1 *puStack_8;
  uint local_4;
  
  puStack_8 = &LAB_004cc593;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_ConfirmMenu_ScalarDtor_004d6c4c;
  local_4 = 1;
  if ((undefined4 *)param_1[0x23] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x23])(1);
  }
  local_4 = local_4 & 0xffffff00;
  ToggleTimer_Cleanup(param_1 + 0x1d);
  *param_1 = &PTR_LAB_004d6c00;
  ExceptionList = pvStack_c;
  return;
}

