
void __fastcall App_ShowMainMenu(int param_1)

{
  void *this;
  int *piVar1;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cae8b;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  this = operator_new(0xcdc);
  local_4 = 0;
  if (this == (void *)0x0) {
    piVar1 = (int *)0x0;
  }
  else {
    piVar1 = MainMenu_ctor(this,param_1);
  }
  local_4 = 0xffffffff;
  *(int **)(param_1 + 0x224) = piVar1;
  Scene_AddObject(*(void **)(param_1 + 0x184),piVar1);
  (**(code **)(**(int **)(param_1 + 0x224) + 0x44))(1);
  ExceptionList = this;
  return;
}

