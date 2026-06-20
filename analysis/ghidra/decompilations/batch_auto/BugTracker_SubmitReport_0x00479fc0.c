
void BugTracker_SubmitReport(void)

{
  undefined4 *puVar1;
  undefined4 uStack_58;
  undefined **in_stack_ffffffb0;
  char *in_stack_ffffffb4;
  undefined **local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined1 local_18;
  undefined4 local_14;
  undefined1 local_10;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cde93;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  if (DAT_00534618 == (undefined4 *)0x0) {
    ExceptionList = &local_c;
    puVar1 = operator_new(0x614);
    local_4 = 0;
    if (puVar1 == (undefined4 *)0x0) {
      DAT_00534618 = (undefined4 *)0x0;
    }
    else {
      DAT_00534618 = WebClient_Ctor(puVar1);
    }
    local_4 = 0xffffffff;
    in_stack_ffffffb0 = &PTR_AthenaCString_DeletingDtor_004d290c;
    in_stack_ffffffb4 = (char *)0x0;
    uStack_58 = 0x47a043;
    AthenaString_AssignCStr(&stack0xffffffb0,PTR_s_RaptisoftBugTracker_004f77c8);
    BugTracker_SetUserAgent((int)DAT_00534618);
  }
  EnableWindow(DAT_00534640,0);
  local_28 = &PTR_AthenaCString_DeletingDtor_004d290c;
  local_24 = 0;
  local_1c = 0;
  local_10 = 1;
  local_20 = 0;
  local_14 = 0;
  local_18 = 0;
  AthenaString_AssignCStr(&local_28,(char *)0x0);
  local_4 = 1;
  AthenaString_AssignCStr(&local_28,PTR_s_http___bugs_raptisoft_com_cgi_bi_004f77c4);
  AthenaString_Assign(&local_28,"?");
  AthenaString_Assign(&local_28,"\"");
  AthenaString_Assign(&local_28,*(char **)(DAT_005341e0 + 0x20));
  AthenaString_Assign(&local_28,"%20");
  AthenaString_Assign(&local_28,"<ERRORREPORT>");
  uStack_58 = 0x47a0fc;
  AthenaString_CopyCtor(&stack0xffffffb0,0x534620);
  AthenaString_AssignFormatted(&local_28,in_stack_ffffffb0,in_stack_ffffffb4);
  AthenaString_Assign(&local_28,"</ERRORREPORT>");
  AthenaString_Assign(&local_28,"\"");
  AthenaString_CopyCtor(&uStack_58,(int)&local_28);
  WebClient_FetchURL(DAT_00534618);
  DAT_0053464c = 1;
  DAT_0053464d = 0;
  ShowWindow(DAT_00534648,1);
  ShowWindow(DAT_00534644,0);
  SetWindowTextA(DAT_00534648,"Contacting Raptisoft ...");
  InvalidateRect(DAT_0053463c,(RECT *)0x0,0);
  InvalidateRect(DAT_00534648,(RECT *)0x0,0);
  local_4 = 0xffffffff;
  AthenaString_dtor(&local_28);
  ExceptionList = local_c;
  return;
}

