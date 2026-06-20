
/* Library Function - Single Match
    ___crtMessageBoxA
   
   Library: Visual Studio 2003 Release */

int __cdecl ___crtMessageBoxA(LPCSTR _LpText,LPCSTR _LpCaption,UINT _UType)

{
  HMODULE hModule;
  int iVar1;
  int iVar2;
  undefined1 local_14 [8];
  byte local_c;
  undefined1 local_8 [4];
  
  iVar2 = 0;
  if (DAT_00535628 == (FARPROC)0x0) {
    hModule = LoadLibraryA("user32.dll");
    if ((hModule == (HMODULE)0x0) ||
       (DAT_00535628 = GetProcAddress(hModule,"MessageBoxA"), DAT_00535628 == (FARPROC)0x0)) {
      return 0;
    }
    DAT_0053562c = GetProcAddress(hModule,"GetActiveWindow");
    DAT_00535630 = GetProcAddress(hModule,"GetLastActivePopup");
    if ((DAT_005352d0 == 2) &&
       (DAT_00535638 = GetProcAddress(hModule,"GetUserObjectInformationA"),
       DAT_00535638 != (FARPROC)0x0)) {
      DAT_00535634 = GetProcAddress(hModule,"GetProcessWindowStation");
    }
  }
  if ((DAT_00535634 == (FARPROC)0x0) ||
     (((iVar1 = (*DAT_00535634)(), iVar1 != 0 &&
       (iVar1 = (*DAT_00535638)(iVar1,1,local_14,0xc,local_8), iVar1 != 0)) && ((local_c & 1) != 0))
     )) {
    if (((DAT_0053562c != (FARPROC)0x0) && (iVar2 = (*DAT_0053562c)(), iVar2 != 0)) &&
       (DAT_00535630 != (FARPROC)0x0)) {
      iVar2 = (*DAT_00535630)(iVar2);
    }
  }
  else if (DAT_005352dc < 4) {
    _UType = _UType | 0x40000;
  }
  else {
    _UType = _UType | 0x200000;
  }
  iVar2 = (*DAT_00535628)(iVar2,_LpText,_LpCaption,_UType);
  return iVar2;
}

