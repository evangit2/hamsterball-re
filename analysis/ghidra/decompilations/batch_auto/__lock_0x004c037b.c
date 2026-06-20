
/* Library Function - Single Match
    __lock
   
   Libraries: Visual Studio 2003 Release, Visual Studio 2005 Release */

void __cdecl __lock(int _File)

{
  int iVar1;
  
  if ((&DAT_004fc988)[_File * 2] == 0) {
    iVar1 = CRT_InitCriticalSection(_File);
    if (iVar1 == 0) {
      __amsg_exit(0x11);
    }
  }
  EnterCriticalSection((LPCRITICAL_SECTION)(&DAT_004fc988)[_File * 2]);
  return;
}

