
void __thiscall AthenaString_WriteTag(void *this,undefined4 param_1,char *param_2)

{
  char *pcVar1;
  char *in_stack_00000024;
  char local_2c [4];
  undefined **local_28;
  char *local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined1 local_18;
  undefined4 local_14;
  undefined1 local_10;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cdad8;
  local_c = ExceptionList;
  local_4 = 1;
  local_2c[1] = 0;
  local_2c[0] = '<';
  if (*(char *)((int)this + 0x18) == '\0') {
    ExceptionList = &local_c;
    AthenaString_Assign(this,local_2c);
  }
  else {
    ExceptionList = &local_c;
    AthenaString_AssignCStr(this,local_2c);
  }
  local_28 = &PTR_AthenaCString_DeletingDtor_004d290c;
  local_24 = (char *)0x0;
  local_1c = 0;
  local_10 = 1;
  local_20 = 0;
  local_14 = 0;
  local_18 = 0;
  AthenaString_AssignCStr(&local_28,param_2);
  pcVar1 = local_24;
  local_4._0_1_ = 2;
  AthenaString_Assign(this,local_24);
  local_4 = CONCAT31(local_4._1_3_,1);
  _free(pcVar1);
  local_2c[1] = 0;
  local_2c[0] = '>';
  if (*(char *)((int)this + 0x18) == '\0') {
    AthenaString_Assign(this,local_2c);
  }
  else {
    AthenaString_AssignCStr(this,local_2c);
  }
  local_28 = &PTR_AthenaCString_DeletingDtor_004d290c;
  local_24 = (char *)0x0;
  local_1c = 0;
  local_10 = 1;
  local_20 = 0;
  local_14 = 0;
  local_18 = 0;
  AthenaString_AssignCStr(&local_28,in_stack_00000024);
  pcVar1 = local_24;
  local_4._0_1_ = 3;
  AthenaString_Assign(this,local_24);
  local_4._0_1_ = 1;
  _free(pcVar1);
  AthenaString_Assign(this,"</");
  local_28 = &PTR_AthenaCString_DeletingDtor_004d290c;
  local_24 = (char *)0x0;
  local_1c = 0;
  local_10 = 1;
  local_20 = 0;
  local_14 = 0;
  local_18 = 0;
  AthenaString_AssignCStr(&local_28,param_2);
  pcVar1 = local_24;
  local_4._0_1_ = 4;
  AthenaString_Assign(this,local_24);
  local_4 = CONCAT31(local_4._1_3_,1);
  _free(pcVar1);
  local_2c[1] = 0;
  local_2c[0] = '>';
  if (*(char *)((int)this + 0x18) == '\0') {
    AthenaString_Assign(this,local_2c);
  }
  else {
    AthenaString_AssignCStr(this,local_2c);
  }
  _free(param_2);
  _free(in_stack_00000024);
  ExceptionList = local_c;
  return;
}

