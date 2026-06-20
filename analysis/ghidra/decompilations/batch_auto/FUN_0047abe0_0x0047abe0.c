
undefined4 FUN_0047abe0(int *param_1)

{
  undefined1 *in_stack_fffff7ac;
  LPCSTR in_stack_fffff7b0;
  char *pcVar1;
  undefined1 *local_830;
  int local_82c;
  undefined **local_828;
  undefined4 local_824;
  undefined4 local_820;
  undefined4 local_81c;
  undefined1 local_818;
  undefined4 local_814;
  undefined1 local_810;
  CHAR local_80c [2044];
  void *pvStack_10;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cdefb;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  AthenaString_AssignCStr(&DAT_00534620,"(null)");
  if (DAT_005341e0 != (int *)0x0) {
    AthenaString_AssignCStr(&DAT_00534620,"");
    ShowWindow((HWND)DAT_005341e0[2],0);
    local_828 = &PTR_AthenaCString_DeletingDtor_004d290c;
    local_824 = 0;
    local_81c = 0;
    local_810 = 1;
    local_820 = 0;
    local_814 = 0;
    local_818 = 0;
    AthenaString_AssignCStr(&local_828,(char *)0x0);
    local_4 = 0;
    pcVar1 = (char *)0x47ac91;
    PE_FindSectionInfo(*(LPCVOID *)(*param_1 + 0xc),local_80c,0x800,(int *)&local_830,&local_82c);
    AthenaString_Assign(&DAT_00534620,"<MODULE>");
    AthenaString_Assign(&DAT_00534620,local_80c);
    AthenaString_Assign(&DAT_00534620,"</MODULE>");
    FontFormatString_Parse(&local_828,0x4dad00);
    local_830 = &stack0xfffff7b0;
    AthenaString_CopyCtor(&stack0xfffff7b0,(int)&local_828);
    AthenaString_AssignFormatted(&DAT_00534620,in_stack_fffff7b0,pcVar1);
    local_830 = &stack0xfffff7b0;
    in_stack_fffff7ac = &stack0xfffff7b0;
    App_BuildDiagnosticReport(DAT_005341e0);
    AthenaString_AssignFormatted(&DAT_00534620,in_stack_fffff7ac,in_stack_fffff7b0);
    puStack_8 = (undefined1 *)0xffffffff;
    AthenaString_dtor(&local_82c);
  }
  MWParser_DumpTags(&DAT_00534620,(undefined4 *)&stack0xfffff7ac);
  BugTracker_ShowDialog(in_stack_fffff7ac,in_stack_fffff7b0);
  SetErrorMode(2);
  ExceptionList = pvStack_10;
  return 0;
}

