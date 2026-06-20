
void __fastcall SceneObject_FreeStrings(undefined4 *param_1)

{
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  puStack_8 = &LAB_004cbde8;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_SceneObject_DeletingDtor_004d5de0;
  uStack_4 = 0;
  _free((void *)param_1[0x220]);
  param_1[0x220] = 0;
  _free((void *)param_1[0x221]);
  param_1[0x221] = 0;
  BaseObject_Init(param_1 + 0x222);
  uStack_4 = 0xffffffff;
  SceneObject_dtor(param_1);
  ExceptionList = pvStack_c;
  return;
}

