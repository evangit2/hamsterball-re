
undefined4 * __fastcall Transform_ctor(undefined4 *param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cd9e8;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  param_1[4] = 0;
  param_1[3] = 0;
  param_1[2] = 0;
  param_1[1] = 0;
  *(undefined1 *)(param_1 + 5) = 1;
  *(undefined1 *)((int)param_1 + 0x15) = 0;
  *(undefined1 *)((int)param_1 + 0x16) = 1;
  param_1[6] = 0;
  local_4 = 0;
  *param_1 = &PTR_AthenaHashTable_DeletingDtor_004d9ef4;
  CRT_InitLocaleVtable(param_1 + 0xc);
  param_1[0xc] = &PTR_D3DTexture_DeletingDtor_004d9ee0;
  param_1[8] = 0;
  *(undefined1 *)(param_1 + 0xb) = 0;
  param_1[0xd] = param_1;
  ExceptionList = local_c;
  return param_1;
}

