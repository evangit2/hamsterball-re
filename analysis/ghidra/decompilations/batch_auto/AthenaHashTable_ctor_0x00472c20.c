
void __fastcall AthenaHashTable_ctor(undefined4 *param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cd9e8;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *param_1 = &PTR_AthenaHashTable_DeletingDtor_004d9ef4;
  local_4 = 0;
  D3DTexture_NullDtor(param_1 + 0xc);
  *param_1 = &PTR_LAB_004cf584;
  ExceptionList = local_c;
  return;
}

