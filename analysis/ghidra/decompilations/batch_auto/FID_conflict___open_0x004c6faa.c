
/* WARNING: Function: __SEH_prolog replaced with injection: SEH_prolog */
/* WARNING: Function: __SEH_epilog replaced with injection: EH_epilog3 */
/* Library Function - Multiple Matches With Different Base Names
    __open
    __wopen
   
   Library: Visual Studio 2003 Release */

int __cdecl FID_conflict___open(char *_Filename,int _OpenFlag,...)

{
  uint uVar1;
  byte in_stack_0000000c;
  uint local_24 [6];
  undefined4 uStack_c;
  undefined4 local_8;
  
  uStack_c = 0x4c6fb6;
  local_24[1] = 0;
  local_8 = 0;
  uVar1 = CRT_OpenFileHandle((void *)0x40,local_24 + 1,local_24,_Filename,_OpenFlag,
                             in_stack_0000000c);
  local_8 = 0xffffffff;
  CRT_CloseFileHandleThunk();
  return uVar1;
}

