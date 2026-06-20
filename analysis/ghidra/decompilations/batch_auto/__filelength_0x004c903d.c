
/* WARNING: Function: __SEH_prolog replaced with injection: SEH_prolog */
/* WARNING: Function: __SEH_epilog replaced with injection: EH_epilog3 */
/* Library Function - Single Match
    __filelength
   
   Library: Visual Studio 2003 Release */

long __cdecl __filelength(int _FileHandle)

{
  DWORD DVar1;
  int *piVar2;
  ulong *puVar3;
  int iVar4;
  DWORD local_24;
  
  if (DAT_005369c8 <= (uint)_FileHandle) {
LAB_004c90f5:
    piVar2 = __errno();
    *piVar2 = 9;
    puVar3 = __doserrno();
    *puVar3 = 0;
    return -1;
  }
  iVar4 = (_FileHandle & 0x1fU) * 0x24;
  if ((*(byte *)((&DAT_005369e0)[_FileHandle >> 5] + 4 + iVar4) & 1) == 0) goto LAB_004c90f5;
  CRT_EnterCriticalSection(_FileHandle);
  if ((*(byte *)((&DAT_005369e0)[_FileHandle >> 5] + 4 + iVar4) & 1) == 0) {
    piVar2 = __errno();
    *piVar2 = 9;
    puVar3 = __doserrno();
    *puVar3 = 0;
  }
  else {
    DVar1 = __lseek_lk(_FileHandle,0,1);
    if (DVar1 != 0xffffffff) {
      local_24 = __lseek_lk(_FileHandle,0,2);
      if (DVar1 != local_24) {
        __lseek_lk(_FileHandle,DVar1,0);
      }
      goto LAB_004c90dc;
    }
  }
  local_24 = 0xffffffff;
LAB_004c90dc:
  CRT_UnlockFileHandleThunk();
  return local_24;
}

