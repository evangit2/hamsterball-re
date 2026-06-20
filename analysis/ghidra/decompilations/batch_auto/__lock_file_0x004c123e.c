
/* Library Function - Single Match
    __lock_file
   
   Library: Visual Studio 2003 Release */

void __cdecl __lock_file(FILE *_File)

{
  if (((FILE *)0x4fcaa7 < _File) && (_File < (FILE *)0x4fcd09)) {
    __lock(((int)&_File[-0x27e56]._bufsiz >> 5) + 0x10);
    return;
  }
  EnterCriticalSection((LPCRITICAL_SECTION)(_File + 1));
  return;
}

