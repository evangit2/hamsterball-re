
/* WARNING: Function: __chkstk replaced with injection: alloca_probe */

undefined4 __thiscall D3DXMesh_OptimizeInPlace(void *this,LPCWSTR param_1,int param_2)

{
  HANDLE pvVar1;
  LPVOID pvVar2;
  DWORD DVar3;
  _OSVERSIONINFOA local_9c;
  int local_8;
  
  if (param_1 == (LPCWSTR)0x0) {
    return 0x8876086c;
  }
  if (param_2 != 0) {
    local_9c.dwOSVersionInfoSize = 0x94;
    GetVersionExA(&local_9c);
    if (local_9c.dwPlatformId != 2) {
      local_8 = WideCharToMultiByte(0,0,param_1,-1,(LPSTR)0x0,0,(LPCSTR)0x0,(LPBOOL)0x0);
      WideCharToMultiByte(0,0,param_1,-1,&stack0xffffff58,local_8,(LPCSTR)0x0,(LPBOOL)0x0);
      param_2 = 0;
      param_1 = (LPCWSTR)&stack0xffffff58;
    }
    if (param_2 != 0) {
      pvVar1 = CreateFileW(param_1,0x80000000,1,(LPSECURITY_ATTRIBUTES)0x0,3,0x10000000,(HANDLE)0x0)
      ;
      goto LAB_00487f24;
    }
  }
  pvVar1 = CreateFileA((LPCSTR)param_1,0x80000000,1,(LPSECURITY_ATTRIBUTES)0x0,3,0x10000000,
                       (HANDLE)0x0);
LAB_00487f24:
  *(HANDLE *)this = pvVar1;
  if (pvVar1 != (HANDLE)0xffffffff) {
    pvVar1 = CreateFileMappingA(pvVar1,(LPSECURITY_ATTRIBUTES)0x0,2,0,0,(LPCSTR)0x0);
    *(HANDLE *)((int)this + 4) = pvVar1;
    if (pvVar1 != (HANDLE)0xffffffff) {
      pvVar2 = MapViewOfFile(pvVar1,4,0,0,0);
      *(LPVOID *)((int)this + 8) = pvVar2;
      if (pvVar2 != (LPVOID)0x0) {
        DVar3 = GetFileSize(*(HANDLE *)this,(LPDWORD)0x0);
        *(DWORD *)((int)this + 0xc) = DVar3;
        if (DVar3 != 0xffffffff) {
          return 0;
        }
      }
    }
  }
  GetLastError();
  return 0x88760b59;
}

