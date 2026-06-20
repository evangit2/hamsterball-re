
void * App_GetProductVersion(void *param_1,undefined4 param_2,LPCSTR param_3)

{
  HMODULE hModule;
  HMODULE hModule_00;
  void *_Memory;
  int iVar1;
  uint uStack_134;
  char *pcStack_130;
  undefined4 local_12c;
  undefined **local_128;
  undefined4 local_124;
  undefined4 local_120;
  undefined4 local_11c;
  undefined1 local_118;
  undefined4 local_114;
  undefined1 local_110;
  CHAR local_10c [256];
  void *pvStack_c;
  undefined1 *puStack_8;
  int local_4;
  
  puStack_8 = &LAB_004cde2d;
  pvStack_c = ExceptionList;
  local_12c = 0;
  local_4 = 1;
  local_128 = &PTR_AthenaCString_DeletingDtor_004d290c;
  local_124 = 0;
  local_11c = 0;
  local_110 = 1;
  local_120 = 0;
  local_114 = 0;
  local_118 = 0;
  ExceptionList = &pvStack_c;
  AthenaString_AssignCStr(&local_128,(char *)0x0);
  local_4 = CONCAT31(local_4._1_3_,2);
  hModule = LoadLibraryA(param_3);
  hModule_00 = LoadLibraryA("version.dll");
  if ((hModule == (HMODULE)0x0) || (hModule_00 == (HMODULE)0x0)) {
    FontFormatString_Parse(&local_128,0x4da778);
    goto LAB_00479d54;
  }
  GetModuleFileNameA(hModule,local_10c,0x100);
  if (DAT_005345f8 == (FARPROC)0x0) {
    DAT_005345f8 = GetProcAddress(hModule_00,"GetFileVersionInfoSizeA");
    DAT_005345f4 = GetProcAddress(hModule_00,"GetFileVersionInfoA");
    DAT_005345f0 = GetProcAddress(hModule_00,"VerQueryValueA");
  }
  uStack_134 = (*DAT_005345f8)(local_10c,0);
  if (uStack_134 != 0) {
    _Memory = operator_new(uStack_134);
    (*DAT_005345f4)(local_10c,0,uStack_134,_Memory);
    iVar1 = (*DAT_005345f0)(_Memory,"\\StringFileInfo\\040904B0\\ProductVersion",&stack0xfffffec0,
                            &stack0xfffffebc);
    if (iVar1 == 0) {
      iVar1 = (*DAT_005345f0)(_Memory,"\\StringFileInfo\\040904E4\\ProductVersion",&pcStack_130,
                              &uStack_134);
      if (iVar1 != 0) goto LAB_00479d19;
    }
    else {
LAB_00479d19:
      AthenaString_AssignCStr(&local_128,pcStack_130);
    }
    _free(_Memory);
  }
  FreeLibrary(hModule);
LAB_00479d54:
  if (hModule_00 != (HMODULE)0x0) {
    FreeLibrary(hModule_00);
  }
  AthenaString_CopyCtor(param_1,(int)&local_128);
  local_12c = 1;
  local_4._0_1_ = 1;
  AthenaString_dtor(&local_128);
  local_4 = (uint)local_4._1_3_ << 8;
  AthenaString_dtor(&param_2);
  ExceptionList = pvStack_c;
  return param_1;
}

