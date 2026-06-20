
undefined4 eSellerate_ExtractDLL(HMODULE param_1)

{
  HANDLE hFindFile;
  undefined4 uVar1;
  HRSRC hResInfo;
  HGLOBAL hResData;
  DWORD DVar2;
  FILE *_File;
  LPVOID _Str;
  size_t sVar3;
  DWORD _Count;
  FILE *_File_00;
  _WIN32_FIND_DATAA local_248;
  uint local_108 [65];
  
  GetWindowsDirectoryA((LPSTR)local_108,0x104);
  StrCat_Fast(local_108,(uint *)s__eSellerateEngine_dll_004f74b0);
  hFindFile = FindFirstFileA((LPCSTR)local_108,&local_248);
  if (hFindFile == (HANDLE)0xffffffff) {
    FindClose((HANDLE)0xffffffff);
    hResInfo = FindResourceA(param_1,(LPCSTR)0x1,&DAT_004f74ac);
    hResData = LoadResource(param_1,hResInfo);
    if (hResData != (HGLOBAL)0x0) {
      DVar2 = SizeofResource(param_1,hResInfo);
      _File = (FILE *)CRT_fsopen((char *)local_108,&DAT_004f74a8);
      if (_File == (FILE *)0x0) {
        FreeResource(hResData);
      }
      else {
        sVar3 = 1;
        _Count = DVar2;
        _File_00 = _File;
        _Str = LockResource(hResData);
        sVar3 = _fwrite(_Str,sVar3,_Count,_File_00);
        FreeResource(hResData);
        _fclose(_File);
        if (DVar2 <= sVar3) goto LAB_004732b1;
        DeleteFileA((LPCSTR)local_108);
      }
    }
    uVar1 = 1;
  }
  else {
    FindClose(hFindFile);
LAB_004732b1:
    uVar1 = 0;
  }
  return uVar1;
}

