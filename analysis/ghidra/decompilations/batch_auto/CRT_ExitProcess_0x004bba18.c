
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl CRT_ExitProcess(UINT param_1,int param_2,int param_3)

{
  HANDLE hProcess;
  undefined4 *puVar1;
  bool bVar2;
  UINT uExitCode;
  
  __lock(8);
  if (DAT_00535310 == 1) {
    uExitCode = param_1;
    hProcess = GetCurrentProcess();
    TerminateProcess(hProcess,uExitCode);
  }
  _DAT_0053530c = 1;
  DAT_00535308 = (undefined1)param_3;
  if (param_2 == 0) {
    if (DAT_00536ae8 != (undefined4 *)0x0) {
      DAT_00536ae4 = DAT_00536ae4 + -1;
      bVar2 = DAT_00536ae4 < DAT_00536ae8;
      while (!bVar2) {
        if ((code *)*DAT_00536ae4 != (code *)0x0) {
          (*(code *)*DAT_00536ae4)();
        }
        DAT_00536ae4 = DAT_00536ae4 + -1;
        bVar2 = DAT_00536ae4 < DAT_00536ae8;
      }
    }
    puVar1 = &DAT_004f7048;
    do {
      if ((code *)*puVar1 != (code *)0x0) {
        (*(code *)*puVar1)();
      }
      puVar1 = puVar1 + 1;
    } while (puVar1 < &DAT_004f7050);
  }
  puVar1 = &DAT_004f7054;
  do {
    if ((code *)*puVar1 != (code *)0x0) {
      (*(code *)*puVar1)();
    }
    puVar1 = puVar1 + 1;
  } while (puVar1 < &DAT_004f705c);
  if (param_3 == 0) {
    DAT_00535310 = 1;
                    /* WARNING: Subroutine does not return */
    ___crtExitProcess(param_1);
  }
  LeaveCriticalSection_indexed(8);
  return;
}

