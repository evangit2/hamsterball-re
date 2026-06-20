
/* Library Function - Single Match
    __amsg_exit
   
   Library: Visual Studio 2003 Release */

void __cdecl __amsg_exit(int param_1)

{
  if (DAT_005352c8 == 1) {
    __FF_MSGBANNER();
  }
  CRT_RuntimeError(param_1);
  (*(code *)PTR___exit_004fc484)(0xff);
  return;
}

