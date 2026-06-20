
/* Library Function - Single Match
    __callnewh
   
   Library: Visual Studio 2003 Release */

int __cdecl __callnewh(size_t _Size)

{
  int iVar1;
  
  if (DAT_00535324 != (code *)0x0) {
    iVar1 = (*DAT_00535324)(_Size);
    if (iVar1 != 0) {
      return 1;
    }
  }
  return 0;
}

