
int CPUID_IsMMXAvailable(void)

{
  LSTATUS LVar1;
  undefined4 extraout_ECX;
  undefined4 extraout_ECX_00;
  undefined4 uVar2;
  undefined4 extraout_EDX;
  undefined4 extraout_EDX_00;
  undefined4 uVar3;
  undefined8 uVar4;
  int local_14;
  DWORD local_10 [2];
  HKEY local_8;
  
  LVar1 = RegOpenKeyA((HKEY)&DAT_80000002,"Software\\Microsoft\\Direct3D",&local_8);
  uVar2 = extraout_ECX;
  uVar3 = extraout_EDX;
  if (LVar1 == 0) {
    local_10[1] = 4;
    LVar1 = RegQueryValueExA(local_8,"DisableMMX",(LPDWORD)0x0,local_10,(LPBYTE)&local_14,
                             local_10 + 1);
    if (((LVar1 == 0) && (local_10[0] == 4)) && (local_14 != 0)) {
      RegCloseKey(local_8);
      DAT_004f77b8 = 0;
      return 0;
    }
    RegCloseKey(local_8);
    uVar2 = extraout_ECX_00;
    uVar3 = extraout_EDX_00;
  }
  if (DAT_004f77b8 < 0) {
    DAT_004f77b8 = 0;
    uVar4 = CPUID_CheckMMX(uVar2,uVar3);
    if ((int)uVar4 != 0) {
      DAT_004f77b8 = 1;
    }
  }
  return DAT_004f77b8;
}

