
uint eSellerate_ReadAffiliateKey(undefined4 param_1,undefined4 param_2,char *param_3,ushort param_4)

{
  HMODULE hModule;
  FARPROC pFVar1;
  FARPROC pFVar2;
  BOOL BVar3;
  char local_418 [1024];
  undefined1 local_18 [4];
  FARPROC local_14;
  undefined4 local_10;
  FARPROC local_c;
  int local_8;
  
  local_8 = 0x400;
  pFVar2 = (FARPROC)param_3;
  if ((param_3 != (char *)0x0) && (param_4 != 0)) {
    *param_3 = '\0';
    hModule = LoadLibraryA(s_advapi32_004f7524);
    local_14 = GetProcAddress(hModule,s_RegOpenKeyExA_004f7514);
    local_c = GetProcAddress(hModule,s_RegQueryValueExA_004f7500);
    pFVar1 = GetProcAddress(hModule,s_RegCloseKey_004f74f4);
    pFVar2 = pFVar1;
    if ((hModule != (HMODULE)0x0) &&
       (((local_14 != (FARPROC)0x0 && (local_c != (FARPROC)0x0)) && (pFVar1 != (FARPROC)0x0)))) {
      AthenaString_SprintfToBuffer(local_418,(byte *)s_Software_eSellerate_Affiliates___004f74cc);
      pFVar2 = (FARPROC)(*local_14)(0x80000001,local_418,0,1,&local_10);
      if (pFVar2 == (FARPROC)0x0) {
        (*local_c)(local_10,&DAT_004f74c8,0,local_18,local_418,&local_8);
        if (local_8 <= (int)(uint)param_4) {
          strcpy(param_3,local_418);
        }
        (*pFVar1)(local_10);
        BVar3 = FreeLibrary(hModule);
        return CONCAT22((short)((uint)BVar3 >> 0x10),(undefined2)local_8);
      }
    }
  }
  return (uint)pFVar2 & 0xffff0000;
}

