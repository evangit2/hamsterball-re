
undefined4 * __fastcall CreateMeshBuffer(undefined4 *param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004ccd8b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *param_1 = &PTR_Scene_ScalarDtor_004d8e70;
  AthenaList_Init(param_1 + 3,0);
  local_4 = 0;
  AthenaList_Init(param_1 + 0x109,0);
  param_1[0x21a] = 0;
  param_1[0x21b] = 0;
  param_1[0x21c] = 0;
  param_1[1] = 0;
  *(undefined1 *)(param_1 + 0x217) = 0;
  *(undefined1 *)((int)param_1 + 0x85d) = 0;
  *(undefined1 *)((int)param_1 + 0x85e) = 0;
  param_1[0x219] = 0;
  *(undefined1 *)((int)param_1 + 0x85f) = 0;
  *(undefined1 *)((int)param_1 + 0x861) = 0;
  *(undefined1 *)(param_1 + 0x218) = 0;
  *(undefined1 *)((int)param_1 + 0x862) = 0;
  *(undefined1 *)((int)param_1 + 0x863) = 0;
  param_1[0x20f] = 0;
  param_1[2] = 0;
  param_1[0x213] = 0;
  param_1[0x214] = 0;
  param_1[0x215] = 0;
  ExceptionList = local_c;
  return param_1;
}

