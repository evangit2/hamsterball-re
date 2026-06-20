
void __fastcall App_ShowConfirmMenu(int *param_1)

{
  void *this;
  int *piVar1;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_004cb01b;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  (**(code **)(*param_1 + 0x40))();
  this = operator_new(0xcfc);
  uStack_4 = 0;
  if (this == (void *)0x0) {
    piVar1 = (int *)0x0;
  }
  else {
    piVar1 = ConfirmMenu_ctor(this,param_1[0x21e],'\x01');
  }
  uStack_4 = 0xffffffff;
  Scene_AddObject(*(void **)(param_1[0x21e] + 0x184),piVar1);
  ExceptionList = pvStack_c;
  return;
}

