
/* Library Function - Single Match
    _atof
   
   Library: Visual Studio 2003 Release */

double __cdecl _atof(char *_String)

{
  int iVar1;
  uint local_1c [6];
  
  while( true ) {
    iVar1 = _isspace((uint)(byte)*_String);
    if (iVar1 == 0) break;
    _String = _String + 1;
  }
  _strlen(_String);
  iVar1 = CRT_StrtodScan(local_1c,(byte *)_String);
  return *(double *)(iVar1 + 0x10);
}

