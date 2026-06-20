
/* Library Function - Single Match
    ___initmbctable
   
   Library: Visual Studio 2003 Release */

undefined4 ___initmbctable(void)

{
  if (DAT_00536aec == 0) {
    __setmbcp(-3);
    DAT_00536aec = 1;
  }
  return 0;
}

