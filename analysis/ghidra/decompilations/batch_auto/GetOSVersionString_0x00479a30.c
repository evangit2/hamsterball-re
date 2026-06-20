
void * GetOSVersionString(void *param_1)

{
  undefined4 in_stack_fffffefc;
  char *in_stack_ffffff00;
  char *pcVar1;
  undefined **local_e0;
  undefined4 local_dc;
  undefined4 local_d8;
  undefined4 local_d4;
  undefined1 local_d0;
  undefined4 local_cc;
  undefined1 local_c8;
  undefined4 local_c4;
  undefined1 *local_c0;
  undefined **local_bc;
  undefined4 local_b8;
  undefined4 local_b4;
  undefined4 local_b0;
  undefined1 local_ac;
  undefined4 local_a8;
  undefined1 local_a4;
  _OSVERSIONINFOA local_a0;
  void *local_c;
  undefined1 *puStack_8;
  int local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cddf0;
  local_c = ExceptionList;
  local_c4 = 0;
  local_e0 = &PTR_AthenaCString_DeletingDtor_004d290c;
  local_dc = 0;
  local_d4 = 0;
  local_c8 = 1;
  local_d8 = 0;
  local_cc = 0;
  local_d0 = 0;
  ExceptionList = &local_c;
  AthenaString_AssignCStr(&local_e0,(char *)0x0);
  local_4 = 1;
  local_a0.dwOSVersionInfoSize = 0x94;
  GetVersionExA(&local_a0);
  AthenaString_Assign(&local_e0,"Windows ");
  if (local_a0.dwPlatformId == 2) {
    pcVar1 = "NT ";
  }
  else {
    pcVar1 = "9x ";
  }
  AthenaString_Assign(&local_e0,pcVar1);
  local_bc = &PTR_AthenaCString_DeletingDtor_004d290c;
  local_b8 = 0;
  local_b0 = 0;
  local_a4 = 1;
  local_b4 = 0;
  local_a8 = 0;
  local_ac = 0;
  AthenaString_AssignCStr(&local_bc,(char *)0x0);
  local_4._0_1_ = 2;
  FontFormatString_Parse(&local_bc,0x4da75c);
  local_c0 = &stack0xfffffefc;
  AthenaString_CopyCtor(&stack0xfffffefc,(int)&local_bc);
  AthenaString_AssignFormatted(&local_e0,in_stack_fffffefc,in_stack_ffffff00);
  AthenaString_Assign(&local_e0," ");
  AthenaString_Assign(&local_e0,local_a0.szCSDVersion);
  AthenaString_Assign(&local_e0," - Build ");
  FontFormatString_Parse(&local_bc,0x4d03f8);
  local_c0 = &stack0xfffffefc;
  AthenaString_CopyCtor(&stack0xfffffefc,(int)&local_bc);
  AthenaString_AssignFormatted(&local_e0,in_stack_fffffefc,in_stack_ffffff00);
  AthenaString_CopyCtor(param_1,(int)&local_e0);
  local_c4 = 1;
  local_4._0_1_ = 1;
  AthenaString_dtor(&local_bc);
  local_4 = (uint)local_4._1_3_ << 8;
  AthenaString_dtor(&local_e0);
  ExceptionList = local_c;
  return param_1;
}

