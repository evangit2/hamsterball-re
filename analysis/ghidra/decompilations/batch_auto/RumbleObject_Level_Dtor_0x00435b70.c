
void __fastcall RumbleObject_Level_Dtor(undefined4 *param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cba48;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *param_1 = &PTR_RumbleObject_DeletingDtor_004d5638;
  local_4 = 0;
  Timer_Cleanup(param_1 + 0x43d);
  local_4 = 0xffffffff;
  Level_Cleanup(param_1);
  ExceptionList = local_c;
  return;
}

