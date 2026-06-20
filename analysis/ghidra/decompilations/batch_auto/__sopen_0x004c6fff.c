
/* WARNING: Function: __SEH_prolog replaced with injection: SEH_prolog */
/* WARNING: Function: __SEH_epilog replaced with injection: EH_epilog3 */
/* Library Function - Single Match
    __sopen
   
   Library: Visual Studio 2003 Release */

int __cdecl __sopen(char *_Filename,int _OpenFlag,int _ShareFlag,...)

{
  uint uVar1;
  byte in_stack_00000010;
  uint local_24 [6];
  undefined4 uStack_c;
  undefined4 local_8;
  
  uStack_c = 0x4c700b;
  local_24[1] = 0;
  local_8 = 0;
  uVar1 = CRT_OpenFileHandle((void *)_ShareFlag,local_24 + 1,local_24,_Filename,_OpenFlag,
                             in_stack_00000010);
  local_8 = 0xffffffff;
  CRT_UnlockFileHandleConditional();
  return uVar1;
}

