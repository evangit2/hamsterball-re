
void __fastcall App_Shutdown(undefined4 *param_1)

{
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cd70b;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_App_ScalarDtor_004d9750;
  local_4 = 0;
  if ((undefined4 *)param_1[0x61] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x61])(1);
    param_1[0x61] = 0;
  }
  if ((undefined4 *)param_1[0x5d] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x5d])(1);
    param_1[0x5d] = 0;
  }
  if ((undefined4 *)param_1[0x5e] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x5e])(1);
    param_1[0x5e] = 0;
  }
  if ((undefined4 *)param_1[0x5f] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x5f])(1);
    param_1[0x5f] = 0;
  }
  if ((undefined4 *)param_1[0x60] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x60])(1);
    param_1[0x60] = 0;
  }
  if ((HWND)param_1[2] != (HWND)0x0) {
    DestroyWindow((HWND)param_1[2]);
    param_1[2] = 0;
  }
  if ((undefined4 *)param_1[0x15] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x15])(1);
    param_1[0x15] = 0;
  }
  CoUninitialize();
  if (0xf < (uint)param_1[0xf]) {
    _free((void *)param_1[10]);
  }
  param_1[0xe] = 0;
  param_1[0xf] = 0xf;
  *(undefined1 *)(param_1 + 10) = 0;
  ExceptionList = pvStack_c;
  return;
}

