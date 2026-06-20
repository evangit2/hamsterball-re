
void __fastcall UIList_dtor(undefined4 *param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  int local_4;
  
  puStack_8 = &LAB_004cbe70;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *param_1 = &PTR_UIList_DeletingDtor_004d5e30;
  local_4 = 4;
  AthenaHashTable_ctor(param_1 + 0x374);
  local_4._0_1_ = 3;
  AthenaHashTable_ctor(param_1 + 0x366);
  local_4._0_1_ = 2;
  AthenaHashTable_ctor(param_1 + 0x358);
  local_4._0_1_ = 1;
  AthenaHashTable_ctor(param_1 + 0x34a);
  local_4 = (uint)local_4._1_3_ << 8;
  AthenaHashTable_ctor(param_1 + 0x33c);
  local_4 = 0xffffffff;
  UIList_Cleanup(param_1);
  ExceptionList = local_c;
  return;
}

