
void __fastcall SceneObject_FreeStrings(undefined4 *param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cbde8;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *param_1 = &PTR_SceneObject_DeletingDtor_004d5de0;
  local_4 = 0;
  _free((void *)param_1[0x220]);
  param_1[0x220] = 0;
  _free((void *)param_1[0x221]);
  param_1[0x221] = 0;
  BaseObject_Init(param_1 + 0x222);
  local_4 = 0xffffffff;
  SceneObject_dtor(param_1);
  ExceptionList = local_c;
  return;
}

