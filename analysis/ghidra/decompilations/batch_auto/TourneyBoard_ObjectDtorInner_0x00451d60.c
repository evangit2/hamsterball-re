
void __fastcall TourneyBoard_ObjectDtorInner(undefined4 *param_1)

{
  void *pvStack_c;
  undefined1 *puStack_8;
  uint local_4;
  
  puStack_8 = &LAB_004cca76;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_TourneyBoard_ObjectScalarDtor_004d8628;
  local_4 = 1;
  if ((undefined4 *)param_1[0x32b] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x32b])(1);
  }
  local_4 = local_4 & 0xffffff00;
  SceneObject_BaseDtor(param_1 + 0x222);
  local_4 = 0xffffffff;
  SceneObject_dtor(param_1);
  ExceptionList = pvStack_c;
  return;
}

