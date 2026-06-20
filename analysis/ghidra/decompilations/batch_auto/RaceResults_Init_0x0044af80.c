
void __fastcall RaceResults_Init(undefined4 *param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cc548;
  local_c = ExceptionList;
  local_4 = 0;
  ExceptionList = &local_c;
  Matrix_Identity(param_1 + 10);
  *param_1 = &PTR_LAB_004d6ae0;
  ExceptionList = local_c;
  return;
}

