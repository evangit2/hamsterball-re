
BOOL SetFileSecurityW(LPCWSTR lpFileName,SECURITY_INFORMATION SecurityInformation,
                     PSECURITY_DESCRIPTOR pSecurityDescriptor)

{
  int iVar1;
  int local_14 [4];
  
  CRT_InitCriticalSection(local_14);
  iVar1 = D3DXMesh_OptimizeInPlace(local_14,(LPCWSTR)SecurityInformation,0);
  if (-1 < iVar1) {
    iVar1 = D3DX_CreateTextureFromFile_Thunk();
  }
  CRT_SharedMemCloseIfValid(local_14);
  return iVar1;
}

