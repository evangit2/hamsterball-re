
/* Library Function - Single Match
    __tell
   
   Library: Visual Studio 2003 Release */

long __cdecl __tell(int _FileHandle)

{
  long lVar1;
  
  lVar1 = __lseek(_FileHandle,0,1);
  return lVar1;
}

