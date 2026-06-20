
/* WARNING: Function: __SEH_prolog replaced with injection: SEH_prolog */
/* WARNING: Function: __chkstk replaced with injection: alloca_probe */
/* WARNING: Function: __SEH_epilog replaced with injection: EH_epilog3 */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

UINT entry(void)

{
  HMODULE pHVar1;
  int iVar2;
  byte *lpCmdLine;
  uint nShowCmd;
  UINT UVar3;
  int *piVar4;
  int extraout_ECX;
  HINSTANCE hPrevInstance;
  _OSVERSIONINFOA local_114;
  _STARTUPINFOA local_68;
  int local_24;
  uint local_20;
  undefined1 *local_1c;
  undefined4 uStack_c;
  undefined *local_8;
  
  local_8 = &DAT_004e9a50;
  uStack_c = 0x4bb4d4;
  local_114.szCSDVersion[0x7c] = -0x20;
  local_114.szCSDVersion[0x7d] = -0x4c;
  local_114.szCSDVersion[0x7e] = 'K';
  local_114.szCSDVersion[0x7f] = '\0';
  local_1c = (undefined1 *)&local_114;
  local_114.dwOSVersionInfoSize = 0x94;
  GetVersionExA(&local_114);
  DAT_005352d0 = local_114.dwPlatformId;
  DAT_005352dc = local_114.dwMajorVersion;
  _DAT_005352e0 = local_114.dwMinorVersion;
  _DAT_005352d4 = local_114.dwBuildNumber & 0x7fff;
  if (local_114.dwPlatformId != 2) {
    _DAT_005352d4 = _DAT_005352d4 | 0x8000;
  }
  _DAT_005352d8 = local_114.dwMajorVersion * 0x100 + local_114.dwMinorVersion;
  pHVar1 = GetModuleHandleA((LPCSTR)0x0);
  if (((short)pHVar1->unused == 0x5a4d) &&
     (piVar4 = (int *)((int)&pHVar1->unused + pHVar1[0xf].unused), *piVar4 == 0x4550)) {
    if ((short)piVar4[6] == 0x10b) {
      if (0xe < (uint)piVar4[0x1d]) {
        iVar2 = piVar4[0x3a];
        goto LAB_004bb589;
      }
    }
    else if (((short)piVar4[6] == 0x20b) && (0xe < (uint)piVar4[0x21])) {
      iVar2 = piVar4[0x3e];
LAB_004bb589:
      local_20 = (uint)(iVar2 != 0);
      goto LAB_004bb58f;
    }
  }
  local_20 = 0;
LAB_004bb58f:
  iVar2 = __heap_init();
  if (iVar2 == 0) {
    fast_error_exit(0x1c);
  }
  iVar2 = CRT_MTInitTLS();
  if (iVar2 == 0) {
    fast_error_exit(0x10);
  }
  CRT_CallAtExitHandlers();
  local_8 = (undefined *)0x0;
  iVar2 = CRT_IOInit();
  if (iVar2 < 0) {
    __amsg_exit(0x1b);
  }
  DAT_00536af0 = GetCommandLineA();
  DAT_005352c0 = ___crtGetEnvironmentStringsA();
  iVar2 = CRT_GetMainArgs(extraout_ECX);
  if (iVar2 < 0) {
    __amsg_exit(8);
  }
  iVar2 = __setenvp();
  if (iVar2 < 0) {
    __amsg_exit(9);
  }
  local_24 = CRT_Initterm();
  if (local_24 != 0) {
    __amsg_exit(local_24);
  }
  local_68.dwFlags = 0;
  GetStartupInfoA(&local_68);
  lpCmdLine = CRT_ScanMBCSWhitespace();
  if ((local_68.dwFlags & 1) == 0) {
    nShowCmd = 10;
  }
  else {
    nShowCmd = (uint)local_68.wShowWindow;
  }
  hPrevInstance = (HINSTANCE)0x0;
  pHVar1 = GetModuleHandleA((LPCSTR)0x0);
  UVar3 = WinMain(pHVar1,hPrevInstance,(LPSTR)lpCmdLine,nShowCmd);
  if (local_20 == 0) {
    CRT_FlsAlloc(UVar3);
  }
  CRT_ExitProcessNoCleanup();
  return UVar3;
}

