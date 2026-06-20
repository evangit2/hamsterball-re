
void __thiscall AthenaString_AssignCStrFree(void *this,undefined4 param_1,char *param_2)

{
  undefined1 in_stack_0000001c;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cda08;
  local_c = ExceptionList;
  local_4 = 0;
  ExceptionList = &local_c;
  AthenaString_AssignCStr(this,param_2);
  *(undefined1 *)((int)this + 0x18) = in_stack_0000001c;
  _free(param_2);
  ExceptionList = local_c;
  return;
}

