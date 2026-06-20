
/* WARNING: Function: __SEH_prolog replaced with injection: SEH_prolog */
/* WARNING: Function: __SEH_epilog replaced with injection: EH_epilog3 */
/* Library Function - Single Match
    ___crtInitCritSecAndSpinCount
   
   Library: Visual Studio 2003 Release */

void __cdecl ___crtInitCritSecAndSpinCount(undefined4 param_1,undefined4 param_2)

{
  HMODULE hModule;
  
  if (DAT_0053563c == (code *)0x0) {
    if (DAT_005352d0 != 1) {
      hModule = GetModuleHandleA("kernel32.dll");
      if (hModule != (HMODULE)0x0) {
        DAT_0053563c = GetProcAddress(hModule,"InitializeCriticalSectionAndSpinCount");
        if (DAT_0053563c != (FARPROC)0x0) goto LAB_004c680f;
      }
    }
    DAT_0053563c = ___crtInitCritSecNoSpinCount_8;
  }
LAB_004c680f:
  (*DAT_0053563c)(param_1,param_2);
  return;
}

