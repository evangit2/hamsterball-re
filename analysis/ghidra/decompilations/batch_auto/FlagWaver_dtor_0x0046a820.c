
void __fastcall FlagWaver_dtor(undefined4 *param_1)

{
  int *piVar1;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cd4db;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_FlagWaver_DeletingDtor_004d9344;
  local_4 = 0;
  _free((void *)param_1[3]);
  piVar1 = (int *)param_1[0x21];
  param_1[3] = 0;
  if (piVar1 != (int *)0x0) {
    (**(code **)(*piVar1 + 8))(piVar1);
  }
  param_1[0x21] = 0;
  local_4 = 0xffffffff;
  Matrix4_Identity(param_1 + 0xc);
  ExceptionList = pvStack_c;
  return;
}

