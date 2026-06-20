
void __cdecl D3D8_DebugSetMute(undefined4 param_1)

{
  HMODULE pHVar1;
  
  if (DAT_00535248 == (FARPROC)0x0) {
    if (DAT_00535244 != (FARPROC)0x0) goto LAB_0048ce10;
    pHVar1 = GetModuleHandleA("d3d8.dll");
    if (pHVar1 != (HMODULE)0x0) {
      pHVar1 = LoadLibraryA("d3d8.dll");
      if (pHVar1 != (HMODULE)0x0) {
        DAT_00535248 = GetProcAddress(pHVar1,"DebugSetMute");
      }
    }
    pHVar1 = GetModuleHandleA("d3d8d.dll");
    if (pHVar1 != (HMODULE)0x0) {
      pHVar1 = LoadLibraryA("d3d8d.dll");
      if (pHVar1 != (HMODULE)0x0) {
        DAT_00535244 = GetProcAddress(pHVar1,"DebugSetMute");
      }
    }
    if (DAT_00535248 != (FARPROC)0x0) goto LAB_0048cdfc;
  }
  else {
LAB_0048cdfc:
    (*DAT_00535248)(param_1);
  }
  if (DAT_00535244 == (FARPROC)0x0) {
    return;
  }
LAB_0048ce10:
                    /* WARNING: Could not recover jumptable at 0x0048ce10. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*DAT_00535244)();
  return;
}

