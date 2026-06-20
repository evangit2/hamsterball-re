
void __fastcall LoadingScreen_ShowRegisterDialog(int param_1)

{
  void *this;
  int *piVar1;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cb17b;
  local_c = ExceptionList;
  if (*(int *)(*(int *)(param_1 + 0x2d68) + 800) != 0) {
    ExceptionList = &local_c;
    this = operator_new(0x115c);
    local_4 = 0;
    if (this == (void *)0x0) {
      piVar1 = (int *)0x0;
    }
    else {
      piVar1 = RegisterDialog_ctor(this,*(int *)(param_1 + 0x2d68),
                                   (byte *)
                                   "You can buy Hamsterball by clicking on the link below!  Clicking the link will open our web store.  After you purchase the game, you will be given a name and serial number.  Enter those in the space below to unlock the game!"
                                  );
    }
    local_4 = 0xffffffff;
    Scene_AddObject(*(void **)(param_1 + 0x30),piVar1);
  }
  ExceptionList = local_c;
  return;
}

