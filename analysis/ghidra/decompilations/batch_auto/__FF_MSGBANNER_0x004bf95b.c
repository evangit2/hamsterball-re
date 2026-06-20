
/* Library Function - Single Match
    __FF_MSGBANNER
   
   Library: Visual Studio 2003 Release */

void __cdecl __FF_MSGBANNER(void)

{
  if ((DAT_005352c8 == 1) || ((DAT_005352c8 == 0 && (DAT_004fc488 == 1)))) {
    CRT_RuntimeError(0xfc);
    if (DAT_0053532c != (code *)0x0) {
      (*DAT_0053532c)();
    }
    CRT_RuntimeError(0xff);
  }
  return;
}

