
void __fastcall ArenaBoard_Sky_DeletingDtor(undefined4 *param_1)

{
  void *pvStack_c;
  undefined1 *puStack_8;
  uint local_4;
  
  puStack_8 = &LAB_004caa86;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_ArenaBoard9_PopCylinder_ScalarDtor_004d1bd8;
  local_4 = 1;
  if ((undefined4 *)param_1[0x11f8] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x11f8])(1);
  }
  param_1[0x11f8] = 0;
  if ((undefined4 *)param_1[0x11f9] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x11f9])(1);
  }
  param_1[0x11f9] = 0;
  if ((undefined4 *)param_1[0x11fa] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x11fa])(1);
  }
  param_1[0x11fa] = 0;
  local_4 = local_4 & 0xffffff00;
  Vec3List_Free(param_1 + 0x11fb);
  local_4 = 0xffffffff;
  ArenaBoard_dtor(param_1);
  ExceptionList = pvStack_c;
  return;
}

