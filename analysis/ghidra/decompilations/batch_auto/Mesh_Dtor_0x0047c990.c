
void __fastcall Mesh_Dtor(undefined4 *param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  uint local_4;
  
  puStack_8 = &LAB_004cdf29;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *param_1 = &PTR_D3DXSkinMesh_DeletingDtor_004db26c;
  local_4 = 1;
  if ((void *)param_1[8] != (void *)0x0) {
    _free((void *)param_1[8]);
    param_1[8] = 0;
  }
  if ((void *)param_1[10] != (void *)0x0) {
    _free((void *)param_1[10]);
    param_1[10] = 0;
  }
  if ((void *)param_1[0xc] != (void *)0x0) {
    _free((void *)param_1[0xc]);
    param_1[0xc] = 0;
  }
  AthenaList_Free((int)(param_1 + 0xd));
  local_4 = local_4 & 0xffffff00;
  Vec3List_Free(param_1 + 0x114);
  local_4 = 0xffffffff;
  Vec3List_Free(param_1 + 0xd);
  ExceptionList = local_c;
  return;
}

