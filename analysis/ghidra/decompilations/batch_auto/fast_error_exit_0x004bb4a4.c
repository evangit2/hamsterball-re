
/* Library Function - Single Match
    _fast_error_exit
   
   Library: Visual Studio 2003 Release */

void __cdecl fast_error_exit(DWORD param_1)

{
  if (DAT_005352c8 == 1) {
    __FF_MSGBANNER();
  }
  CRT_RuntimeError(param_1);
                    /* WARNING: Subroutine does not return */
  ___crtExitProcess(0xff);
}

