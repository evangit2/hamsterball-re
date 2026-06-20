
void __thiscall AthenaString_AssignFormatted(void *this,undefined4 param_1,char *param_2)

{
  char *_Memory;
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
  
  puStack_8 = &LAB_004cda30;
  local_c = ExceptionList;
  local_4 = 0;
  local_28 = &PTR_AthenaCString_DeletingDtor_004d290c;
  local_24 = (char *)0x0;
  local_1c = 0;
  local_10 = 1;
  local_20 = 0;
  local_14 = 0;
  local_18 = 0;
  ExceptionList = &local_c;
  AthenaString_AssignCStr(&local_28,param_2);
  _Memory = local_24;
  local_4 = CONCAT31(local_4._1_3_,1);
  AthenaString_Assign(this,local_24);
  _free(_Memory);
  _free(param_2);
  ExceptionList = local_c;
  return;
}

