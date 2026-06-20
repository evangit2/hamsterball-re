
/* Library Function - Single Match
    ___endstdio
   
   Library: Visual Studio 2003 Release */

void ___endstdio(void)

{
  __flushall();
  if (DAT_00535308 != '\0') {
    __fcloseall();
    return;
  }
  return;
}

