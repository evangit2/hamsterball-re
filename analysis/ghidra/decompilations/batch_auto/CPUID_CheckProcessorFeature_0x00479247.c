
uint __cdecl CPUID_CheckProcessorFeature(DWORD param_1)

{
  BOOL BVar1;
  uint uVar2;
  int iVar3;
  undefined4 extraout_ECX;
  undefined4 extraout_EDX;
  _OSVERSIONINFOA *p_Var4;
  undefined8 uVar5;
  _OSVERSIONINFOA local_9c;
  char local_5;
  
  p_Var4 = &local_9c;
  for (iVar3 = 0x25; iVar3 != 0; iVar3 = iVar3 + -1) {
    p_Var4->dwOSVersionInfoSize = 0;
    p_Var4 = (_OSVERSIONINFOA *)&p_Var4->dwMajorVersion;
  }
  local_5 = '\0';
  local_9c.dwOSVersionInfoSize = 0x94;
  BVar1 = GetVersionExA(&local_9c);
  if (BVar1 == 0) {
    local_5 = '\x01';
  }
  if (local_9c.dwPlatformId == 1) {
    if ((local_9c.dwMajorVersion < 5) &&
       (((local_9c.dwMajorVersion != 4 || (local_9c.dwMinorVersion < 10)) ||
        ((ushort)local_9c.dwBuildNumber < 0x55d)))) {
      local_5 = '\x01';
    }
  }
  else {
    if (local_9c.dwPlatformId != 2) {
      return 0;
    }
    if (BVar1 != 0) {
      if (param_1 != 10) {
        uVar2 = IsProcessorFeaturePresent(param_1);
        return uVar2;
      }
      goto LAB_004792ec;
    }
  }
  if (param_1 == 6) {
    if (local_5 == '\0') {
      uVar2 = CPUID_DetectFeatures();
      return uVar2 & 4;
    }
  }
  else {
    if (param_1 == 7) {
      uVar5 = CPUID_Check3DNow(extraout_ECX,extraout_EDX);
      return (uint)uVar5;
    }
    if ((param_1 == 10) && (local_5 == '\0')) {
LAB_004792ec:
      uVar2 = CPUID_DetectFeatures();
      return uVar2 & 8;
    }
  }
  return 0;
}

