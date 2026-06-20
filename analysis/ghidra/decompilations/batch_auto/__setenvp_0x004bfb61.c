
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* Library Function - Single Match
    __setenvp
   
   Library: Visual Studio 2003 Release */

int __cdecl __setenvp(void)

{
  undefined4 *puVar1;
  size_t sVar2;
  char *_Dest;
  char *pcVar3;
  int iVar4;
  
  if (DAT_00536aec == 0) {
    ___initmbctable();
  }
  iVar4 = 0;
  pcVar3 = DAT_005352c0;
  if (DAT_005352c0 != (char *)0x0) {
    for (; *pcVar3 != '\0'; pcVar3 = pcVar3 + sVar2 + 1) {
      if (*pcVar3 != '=') {
        iVar4 = iVar4 + 1;
      }
      sVar2 = _strlen(pcVar3);
    }
    puVar1 = _malloc(iVar4 * 4 + 4);
    pcVar3 = DAT_005352c0;
    DAT_005352f0 = puVar1;
    if (puVar1 != (undefined4 *)0x0) {
      do {
        if (*pcVar3 == '\0') {
          _free(DAT_005352c0);
          DAT_005352c0 = (char *)0x0;
          *puVar1 = 0;
          _DAT_00536ae0 = 1;
          return 0;
        }
        sVar2 = _strlen(pcVar3);
        if (*pcVar3 != '=') {
          _Dest = _malloc(sVar2 + 1);
          *puVar1 = _Dest;
          if (_Dest == (char *)0x0) {
            _free(DAT_005352f0);
            DAT_005352f0 = (undefined4 *)0x0;
            return -1;
          }
          strcpy(_Dest,pcVar3);
          puVar1 = puVar1 + 1;
        }
        pcVar3 = pcVar3 + sVar2 + 1;
      } while( true );
    }
  }
  return -1;
}

