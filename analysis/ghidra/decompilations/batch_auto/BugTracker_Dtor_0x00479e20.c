
void __fastcall BugTracker_Dtor(undefined4 *param_1)

{
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cde6b;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_BugTracker_DeletingDtor_004da864;
  local_4 = 0;
  if (DAT_00534600 != (LPTOP_LEVEL_EXCEPTION_FILTER)0x0) {
    SetUnhandledExceptionFilter(DAT_00534600);
  }
  DeleteObject((HGDIOBJ)param_1[2]);
  DeleteObject((HGDIOBJ)param_1[3]);
  if ((undefined4 *)param_1[4] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[4])(1);
  }
  local_4 = 0xffffffff;
  AthenaString_dtor(param_1 + 6);
  ExceptionList = pvStack_c;
  return;
}

