
void __fastcall App_Run(int *param_1)

{
  char cVar1;
  int iVar2;
  DWORD DVar3;
  int iVar4;
  int iVar5;
  int local_78;
  DWORD local_74;
  tagMSG local_6c;
  undefined4 local_50 [17];
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd5f8;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  Timer_Init(local_50);
  iVar2 = param_1[0x5c];
  local_4 = 0;
  local_74 = 0;
  local_78 = 0;
  DVar3 = GetTickCount();
  param_1[0x59] = DVar3;
  cVar1 = *(char *)((int)param_1 + 0x159);
  while (cVar1 == '\0') {
    iVar5 = 0;
    Sleep(0);
    param_1[0x84] = (int)"Background";
    param_1[0x5a] = (int)(1000 / (longlong)param_1[0x5b]);
    DVar3 = GetTickCount();
    if (DAT_005341e4 < (int)DVar3) {
      if ((char)param_1[0x6b] == '\x01') {
        AthenaString_SprintfToBuffer((char *)(param_1 + 0x66),&DAT_004d03f8);
      }
      param_1[0x65] = 0;
      DVar3 = GetTickCount();
      DAT_005341e4 = DVar3 + 1000;
    }
    iVar4 = PeekMessageA(&local_6c,(HWND)0x0,0,0,1);
    while (iVar4 != 0) {
      if (*(char *)((int)param_1 + 0x159) != '\0') goto LAB_0046bfc3;
      TranslateMessage(&local_6c);
      DispatchMessageA(&local_6c);
      iVar4 = PeekMessageA(&local_6c,(HWND)0x0,0,0,1);
    }
    if (*(char *)((int)param_1 + 0x159) != '\0') break;
    do {
      DVar3 = GetTickCount();
      if (((int)(DVar3 - param_1[0x59]) < param_1[0x5a] + -5) ||
         (local_78 = local_78 + 1, 9 < local_78)) {
        param_1[0x84] = (int)&DAT_004d9584;
        local_78 = 0;
        if ((int)(1000 / (longlong)iVar2) + -5 + local_74 < DVar3) {
          if (((void *)param_1[0x5d] != (void *)0x0) &&
             ((*(char *)((int)param_1 + 0x15a) != '\0' || ((char)param_1[0x56] == '\0')))) {
            param_1[0x65] = param_1[0x65] + 1;
            Graphics_BeginFrame((void *)param_1[0x5d],(int)local_50);
            (**(code **)(*param_1 + 0x24))();
            (**(code **)(*param_1 + 0x28))();
            (**(code **)(*param_1 + 0x2c))();
            Graphics_PresentOrEnd((void *)param_1[0x5d],'\x01');
          }
          local_74 = GetTickCount();
        }
        break;
      }
      param_1[99] = param_1[99] + 1;
      param_1[0x84] = (int)"Update";
      Graphics_BeginFrame((void *)param_1[0x5d],(int)local_50);
      (**(code **)(*param_1 + 0x20))();
      iVar4 = param_1[0x59];
      param_1[0x59] = iVar4 + param_1[0x5a];
      if (1000 < (int)(DVar3 - (iVar4 + param_1[0x5a]))) {
        param_1[0x59] = DVar3 - 1000;
      }
      iVar5 = iVar5 + 1;
    } while (iVar5 < 1);
    cVar1 = *(char *)((int)param_1 + 0x159);
  }
LAB_0046bfc3:
  local_4 = 0xffffffff;
  Timer_Cleanup(local_50);
  ExceptionList = pvStack_c;
  return;
}

