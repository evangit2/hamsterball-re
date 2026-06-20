
undefined4 * __fastcall AthenaListObj_ctor(undefined4 *param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd53b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *param_1 = &PTR_MeshBuffer_DeletingDtor_004d9348;
  AthenaList_Init(param_1 + 1,0);
  local_4 = 0;
  AthenaList_Free((int)(param_1 + 1));
  *(undefined1 *)(param_1 + 0x108) = 0;
  ExceptionList = local_c;
  return param_1;
}

