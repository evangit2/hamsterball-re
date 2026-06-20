
void __cdecl BugTracker_ShowDialog(undefined4 param_1,LPCSTR param_2)

{
  bool bVar1;
  bool bVar2;
  LPCSTR lpWindowName;
  HMODULE pHVar3;
  HWND hWnd;
  int iVar4;
  undefined4 *puVar5;
  HWND__ HVar6;
  char *pcVar7;
  LPVOID pvVar8;
  undefined **local_ec;
  LPCSTR local_e8;
  undefined4 local_e4;
  undefined4 local_e0;
  undefined1 local_dc;
  undefined4 local_d8;
  undefined1 local_d4;
  HWND local_d0;
  undefined **local_cc;
  char *local_c8;
  undefined4 local_c4;
  undefined4 local_c0;
  undefined1 local_bc;
  undefined4 local_b8;
  undefined1 local_b4;
  undefined **local_b0;
  LPCSTR local_ac;
  undefined4 local_a8;
  undefined4 local_a4;
  undefined1 local_a0;
  undefined4 local_9c;
  undefined1 local_98;
  undefined **local_94;
  LPCSTR local_90;
  undefined4 local_8c;
  undefined4 local_88;
  undefined1 local_84;
  undefined4 local_80;
  undefined1 local_7c;
  tagRECT local_78;
  WNDCLASSA local_68;
  undefined4 local_40 [6];
  tagMSG local_28;
  void *local_c;
  undefined1 *puStack_8;
  int local_4;
  
  puStack_8 = &LAB_004cdedc;
  local_c = ExceptionList;
  local_4 = 0;
  local_68.style = 8;
  local_68.cbClsExtra = 0;
  local_68.cbWndExtra = 0;
  ExceptionList = &local_c;
  local_68.hCursor = LoadCursorA((HINSTANCE)0x0,&DAT_00007f00);
  pcVar7 = "MAINICON";
  pHVar3 = GetModuleHandleA((LPCSTR)0x0);
  local_68.hIcon = LoadIconA(pHVar3,pcVar7);
  local_68.hInstance = GetModuleHandleA((LPCSTR)0x0);
  local_68.lpfnWndProc = (WNDPROC)&LAB_0047a1c0;
  local_68.lpszClassName = "RaptisoftCrashWindow";
  local_68.lpszMenuName = (LPCSTR)0x0;
  local_68.hbrBackground = GetSysColorBrush(0xf);
  RegisterClassA(&local_68);
  local_94 = &PTR_AthenaCString_DeletingDtor_004d290c;
  local_90 = (LPCSTR)0x0;
  local_88 = 0;
  local_7c = 1;
  local_8c = 0;
  local_80 = 0;
  local_84 = 0;
  AthenaString_AssignCStr(&local_94,(char *)0x0);
  local_4._0_1_ = 1;
  FontFormatString_Parse(&local_94,0x4dacd0);
  lpWindowName = local_90;
  pvVar8 = (LPVOID)0x0;
  pHVar3 = GetModuleHandleA((LPCSTR)0x0);
  DAT_0053463c = CreateWindowExA(0,"RaptisoftCrashWindow",lpWindowName,0x92ca0000,100,100,500,0x1cc,
                                 (HWND)0x0,(HMENU)0x0,pHVar3,pvVar8);
  GetClientRect(DAT_0053463c,&local_78);
  pvVar8 = (LPVOID)0x0;
  pHVar3 = GetModuleHandleA((LPCSTR)0x0);
  DAT_00534644 = CreateWindowExA(0x200,"EDIT",(LPCSTR)0x0,0x50201804,0xf,0x6e,local_78.right + -0x1e
                                 ,0x104,DAT_0053463c,(HMENU)&DAT_0000012c,pHVar3,pvVar8);
  SetWindowTextA(DAT_00534644,param_2);
  pvVar8 = (LPVOID)0x0;
  pHVar3 = GetModuleHandleA((LPCSTR)0x0);
  DAT_00534648 = CreateWindowExA(0x200,"EDIT",(LPCSTR)0x0,0x40201804,0xf,0x6e,local_78.right + -0x1e
                                 ,0x104,DAT_0053463c,(HMENU)&DAT_00000190,pHVar3,pvVar8);
  pvVar8 = (LPVOID)0x0;
  pHVar3 = GetModuleHandleA((LPCSTR)0x0);
  local_d0 = CreateWindowExA(0,"STATIC",(LPCSTR)0x0,0x50000000,10,10,local_78.right + -0x14,0x50,
                             DAT_0053463c,(HMENU)0x0,pHVar3,pvVar8);
  local_b0 = &PTR_AthenaCString_DeletingDtor_004d290c;
  local_ac = (LPCSTR)0x0;
  local_a4 = 0;
  local_98 = 1;
  local_a8 = 0;
  local_9c = 0;
  local_a0 = 0;
  AthenaString_AssignCStr(&local_b0,(char *)0x0);
  local_4._0_1_ = 2;
  FontFormatString_Parse(&local_b0,0x4dabe0);
  SetWindowTextA(local_d0,local_ac);
  pvVar8 = (LPVOID)0x0;
  pHVar3 = GetModuleHandleA((LPCSTR)0x0);
  DAT_00534640 = CreateWindowExA(0,"BUTTON",(LPCSTR)0x0,0x50004000,10,0x181,
                                 local_78.right / 2 + -0xf,0x1e,DAT_0053463c,(HMENU)&DAT_00000064,
                                 pHVar3,pvVar8);
  SetWindowTextA(DAT_00534640,"Send Report");
  EnableWindow(DAT_00534640,1);
  pvVar8 = (LPVOID)0x0;
  pHVar3 = GetModuleHandleA((LPCSTR)0x0);
  HVar6.unused = 0x181;
  hWnd = CreateWindowExA(0,"BUTTON",(LPCSTR)0x0,0x50004000,local_78.right / 2 + 5,0x181,
                         local_78.right / 2 + -0xf,0x1e,DAT_0053463c,(HMENU)&DAT_000000c8,pHVar3,
                         pvVar8);
  local_cc = &PTR_AthenaCString_DeletingDtor_004d290c;
  local_c8 = (char *)0x0;
  local_c0 = 0;
  local_b4 = 1;
  local_c4 = 0;
  local_b8 = 0;
  local_bc = 0;
  AthenaString_AssignCStr(&local_cc,(char *)0x0);
  local_4 = CONCAT31(local_4._1_3_,3);
  AthenaString_SprintfToBuffer(local_c8,(byte *)"Close %s");
  SetWindowTextA(hWnd,local_c8);
  ShowWindow(DAT_0053463c,5);
  do {
    do {
      if (DAT_0053460c != '\0') {
        local_4._1_3_ = (uint3)((uint)local_4 >> 8);
        local_4._0_1_ = 2;
        AthenaString_dtor(&local_cc);
        local_4._0_1_ = 1;
        AthenaString_dtor(&local_b0);
        local_4 = (uint)local_4._1_3_ << 8;
        AthenaString_dtor(&local_94);
        local_4 = 0xffffffff;
        AthenaString_dtor(&param_1);
        ExceptionList = local_c;
        return;
      }
      pcVar7 = (char *)0x47a7fb;
      iVar4 = PeekMessageA(&local_28,(HWND)0x0,0,0,1);
      while ((iVar4 != 0 && (DAT_0053460c == '\0'))) {
        TranslateMessage(&local_28);
        DispatchMessageA(&local_28);
        pcVar7 = (char *)0x47a833;
        iVar4 = PeekMessageA(&local_28,(HWND)0x0,0,0,1);
      }
    } while ((((DAT_00534618 == 0) || (DAT_0053464c == '\0')) || (DAT_0053464d != '\0')) ||
            (bVar2 = false, *(int *)(DAT_00534618 + 4) == 2));
    local_ec = &PTR_AthenaCString_DeletingDtor_004d290c;
    local_e8 = (LPCSTR)0x0;
    local_e0 = 0;
    local_d4 = 1;
    local_e4 = 0;
    local_d8 = 0;
    local_dc = 0;
    AthenaString_AssignCStr(&local_ec,"Contacting Raptisoft ...");
    local_4._0_1_ = 4;
    AthenaString_AssignCRLF(&local_ec);
    AthenaString_AssignCRLF(&local_ec);
    if (*(int *)(DAT_00534618 + 4) == 1) {
      Sprite_DrawColoredRect(local_40);
      local_4 = CONCAT31(local_4._1_3_,5);
      AthenaString_Set(local_40,*(char **)(DAT_00534618 + 0xc));
      bVar1 = true;
      puVar5 = MWParser_ReadTag((int)local_40);
      if (puVar5 == (undefined4 *)0x0) {
LAB_0047aa92:
        AthenaString_Assign(&local_ec,"Error: Could not contact Raptisoft reporting server!");
        AthenaString_AssignCRLF(&local_ec);
        AthenaString_AssignCRLF(&local_ec);
        AthenaString_Assign(&local_ec,"We\'re sorry, we could not process your error report.  This")
        ;
        AthenaString_AssignCRLF(&local_ec);
        AthenaString_Assign(&local_ec,"may be due to a temporary server outage, or could be because"
                           );
        AthenaString_AssignCRLF(&local_ec);
        AthenaString_Assign(&local_ec,"you are not connected to the internet.  You may press");
        AthenaString_AssignCRLF(&local_ec);
        AthenaString_Assign(&local_ec,"Send Report if you wish to try to report this error again,");
        AthenaString_AssignCRLF(&local_ec);
        AthenaString_Assign(&local_ec,"or click the close button to close the program.");
        EnableWindow(DAT_00534640,1);
      }
      else {
        do {
          iVar4 = __stricmp((char *)puVar5[1],"RESULT");
          if (iVar4 == 0) {
            iVar4 = __stricmp((char *)puVar5[2],"OK");
            if (iVar4 == 0) {
              bVar1 = false;
              AthenaString_Assign(&local_ec,"Error report sent successfully!");
              AthenaString_AssignCRLF(&local_ec);
              AthenaString_Assign(&local_ec,"Thank you for your feedback!");
              AthenaString_AssignCRLF(&local_ec);
              AthenaString_AssignCRLF(&local_ec);
              AthenaString_Assign(&local_ec,
                                  "You can click the close button to safely shut down this");
              AthenaString_AssignCRLF(&local_ec);
              AthenaString_Assign(&local_ec,"program now.");
            }
            iVar4 = __stricmp((char *)puVar5[2],"retry");
            if (iVar4 == 0) {
              bVar1 = false;
              bVar2 = true;
              AthenaString_Assign(&local_ec,"Retrying...");
              BugTracker_SubmitReport();
            }
          }
          puVar5 = MWParser_ReadTag((int)local_40);
        } while (puVar5 != (undefined4 *)0x0);
        if (bVar1) goto LAB_0047aa92;
      }
      local_4._0_1_ = 4;
      StreamReader_dtor(local_40);
    }
    else {
      AthenaString_Assign(&local_ec,"Could not send error report:");
      AthenaString_AssignCRLF(&local_ec);
      local_d0 = (HWND)&stack0xfffffee4;
      AthenaString_CopyCtor(&stack0xfffffee4,DAT_00534618 + 8);
      AthenaString_AssignFormatted(&local_ec,HVar6.unused,pcVar7);
      AthenaString_AssignCRLF(&local_ec);
      AthenaString_AssignCRLF(&local_ec);
      AthenaString_Assign(&local_ec,"We\'re sorry we couldn\'t connect to deliver this error");
      AthenaString_AssignCRLF(&local_ec);
      AthenaString_Assign(&local_ec,"report, but we appreciate the effort!  You can click");
      AthenaString_AssignCRLF(&local_ec);
      AthenaString_Assign(&local_ec,"Send Report to try again, or simply click the close");
      AthenaString_AssignCRLF(&local_ec);
      AthenaString_Assign(&local_ec,"button to shut down the program.");
      EnableWindow(DAT_00534640,1);
    }
    SetWindowTextA(DAT_00534648,local_e8);
    if (!bVar2) {
      DAT_0053464d = '\x01';
    }
    local_4 = CONCAT31(local_4._1_3_,3);
    AthenaString_dtor(&local_ec);
  } while( true );
}

