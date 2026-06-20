
void __fastcall App_CreateScoreDisplay(int param_1)

{
  void *this;
  int *piVar1;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd61b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  this = operator_new(0x8a4);
  local_4 = 0;
  if (this == (void *)0x0) {
    piVar1 = (int *)0x0;
  }
  else {
    piVar1 = SplashScreen_Ctor(this,param_1);
  }
  local_4 = 0xffffffff;
  Scene_AddObject(*(void **)(param_1 + 0x184),piVar1);
  ExceptionList = local_c;
  return;
}

