
void __thiscall AthenaString_ReplaceAt(void *this,char *param_1,size_t param_2)

{
  char *pcVar1;
  char *_Memory;
  undefined **local_44;
  char *local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined1 local_34;
  undefined4 local_30;
  undefined1 local_2c;
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
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cda78;
  local_c = ExceptionList;
  if (param_1 != (char *)0x0) {
    local_44 = &PTR_AthenaCString_DeletingDtor_004d290c;
    local_40 = (char *)0x0;
    local_38 = 0;
    local_2c = 1;
    local_3c = 0;
    local_30 = 0;
    local_34 = 0;
    ExceptionList = &local_c;
    AthenaString_AssignCStr(&local_44,(char *)0x0);
    local_4 = 0;
    local_28 = &PTR_AthenaCString_DeletingDtor_004d290c;
    local_24 = (char *)0x0;
    local_1c = 0;
    local_10 = 1;
    local_20 = 0;
    local_14 = 0;
    local_18 = 0;
    AthenaString_AssignCStr(&local_28,*(char **)((int)this + 4));
    pcVar1 = local_24;
    local_10 = *(undefined1 *)((int)this + 0x18);
    local_4._0_1_ = 1;
    StdString_AppendN(&local_44,local_24,param_2);
    local_4._0_1_ = 0;
    _free(pcVar1);
    AthenaString_Assign(&local_44,param_1);
    AthenaString_Assign(&local_44,(char *)(*(int *)((int)this + 4) + param_2));
    pcVar1 = local_40;
    local_28 = &PTR_AthenaCString_DeletingDtor_004d290c;
    local_24 = (char *)0x0;
    local_1c = 0;
    local_10 = 1;
    local_20 = 0;
    local_14 = 0;
    local_18 = 0;
    AthenaString_AssignCStr(&local_28,local_40);
    _Memory = local_24;
    local_10 = local_2c;
    local_4 = CONCAT31(local_4._1_3_,2);
    AthenaString_AssignCStr(this,local_24);
    _free(_Memory);
    _free(pcVar1);
  }
  ExceptionList = local_c;
  return;
}

