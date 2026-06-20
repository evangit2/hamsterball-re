
/* WARNING: Function: __SEH_prolog replaced with injection: SEH_prolog */
/* WARNING: Function: __SEH_epilog replaced with injection: EH_epilog3 */
/* Library Function - Single Match
    __write
   
   Library: Visual Studio 2003 Release */

int __cdecl __write(int _FileHandle,void *_Buf,uint _MaxCharCount)

{
  int *piVar1;
  ulong *puVar2;
  int iVar3;
  int local_20;
  
  if ((uint)_FileHandle < DAT_005369c8) {
    iVar3 = (_FileHandle & 0x1fU) * 0x24;
    if ((*(byte *)((&DAT_005369e0)[_FileHandle >> 5] + 4 + iVar3) & 1) != 0) {
      CRT_EnterCriticalSection(_FileHandle);
      if ((*(byte *)((&DAT_005369e0)[_FileHandle >> 5] + 4 + iVar3) & 1) == 0) {
        piVar1 = __errno();
        *piVar1 = 9;
        puVar2 = __doserrno();
        *puVar2 = 0;
        local_20 = -1;
      }
      else {
        local_20 = CRT_WriteFileTranslated(_FileHandle,_Buf,_MaxCharCount);
      }
      CRT_UnlockFileHandle();
      return local_20;
    }
  }
  piVar1 = __errno();
  *piVar1 = 9;
  puVar2 = __doserrno();
  *puVar2 = 0;
  return -1;
}

