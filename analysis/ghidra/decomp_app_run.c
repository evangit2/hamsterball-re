
void __fastcall App_Run(int *param_1)

{
  char cVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  int local_78;
  int local_74;
  int local_70;
  undefined1 auStack_6c [28];
  undefined4 local_50 [17];
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd5f8;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  Timer_Init(local_50);
  local_70 = (int)(1000 / (longlong)param_1[0x5c]);
  local_4 = 0;
  local_74 = 0;
  local_78 = 0;
  iVar2 = GetTickCount();
  param_1[0x59] = iVar2;
  cVar1 = *(char *)((int)param_1 + 0x159);
  while (cVar1 == '\0') {
    iVar4 = 0;
    Sleep(0);
    param_1[0x84] = (int)"Background";
    param_1[0x5a] = (int)(1000 / (longlong)param_1[0x5b]);
    iVar2 = GetTickCount();
    if (DAT_005341e4 < iVar2) {
      if ((char)param_1[0x6b] == '\x01') {
        FUN_004bae43((char *)(param_1 + 0x66),&DAT_004d03f8);
      }
      param_1[0x65] = 0;
      iVar2 = GetTickCount();
      DAT_005341e4 = iVar2 + 1000;
    }
    iVar2 = PeekMessageA(&local_70,0,0,0,1);
    while (iVar2 != 0) {
      if (*(char *)((int)param_1 + 0x159) != '\0') goto LAB_0046bfc3;
      TranslateMessage(auStack_6c);
      DispatchMessageA(&local_70);
      iVar2 = PeekMessageA(&local_74,0,0,0,1);
    }
    if (*(char *)((int)param_1 + 0x159) != '\0') break;
    do {
      uVar3 = GetTickCount();
      if (((int)(uVar3 - param_1[0x59]) < param_1[0x5a] + -5) ||
         (local_78 = local_78 + 1, 9 < local_78)) {
        param_1[0x84] = (int)&DAT_004d9584;
        local_78 = 0;
        if ((uint)(local_70 + -5 + local_74) < uVar3) {
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
      iVar2 = param_1[0x59];
      param_1[0x59] = iVar2 + param_1[0x5a];
      if (1000 < (int)(uVar3 - (iVar2 + param_1[0x5a]))) {
        param_1[0x59] = uVar3 - 1000;
      }
      iVar4 = iVar4 + 1;
    } while (iVar4 < 1);
    cVar1 = *(char *)((int)param_1 + 0x159);
  }
LAB_0046bfc3:
  local_4 = 0xffffffff;
  Timer_Cleanup(local_50);
  ExceptionList = pvStack_c;
  return;
}

