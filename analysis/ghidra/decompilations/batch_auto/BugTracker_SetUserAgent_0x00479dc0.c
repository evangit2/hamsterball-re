
void __fastcall BugTracker_SetUserAgent(int param_1)

{
  undefined4 in_stack_ffffffd0;
  char *in_stack_ffffffd4;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cde48;
  local_c = ExceptionList;
  local_4 = 0;
  ExceptionList = &local_c;
  AthenaString_CopyCtor(&stack0xffffffd0,(int)&stack0x00000004);
  AthenaString_AssignCStrFree((void *)(param_1 + 0x40),in_stack_ffffffd0,in_stack_ffffffd4);
  local_4 = 0xffffffff;
  AthenaString_dtor((undefined4 *)&stack0x00000004);
  ExceptionList = local_c;
  return;
}

