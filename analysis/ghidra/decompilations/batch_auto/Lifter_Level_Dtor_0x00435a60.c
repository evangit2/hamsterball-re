
void __fastcall Lifter_Level_Dtor(undefined4 *param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cba48;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *param_1 = &PTR_Lifter_DeletingDtor_004d5518;
  local_4 = 0;
  Vec3List_Free(param_1 + 0x43c);
  local_4 = 0xffffffff;
  Level_Cleanup(param_1);
  ExceptionList = local_c;
  return;
}

