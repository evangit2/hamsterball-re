
void __fastcall SceneObject_DeletingDtor(undefined4 *param_1)

{
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cdda8;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_SceneObject_ScalarDtor_004da6c8;
  local_4 = 0;
  if ((undefined4 *)param_1[0x220] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x220])(1);
  }
  if ((undefined4 *)param_1[0x21f] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x21f])(1);
  }
  if ((undefined4 *)param_1[0x228] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x228])(1);
  }
  local_4 = 0xffffffff;
  SceneObject_dtor(param_1);
  ExceptionList = pvStack_c;
  return;
}

