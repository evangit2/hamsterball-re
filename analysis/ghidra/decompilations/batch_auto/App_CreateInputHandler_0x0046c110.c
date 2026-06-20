
void __fastcall App_CreateInputHandler(int param_1)

{
  void *pvVar1;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd61b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  pvVar1 = operator_new(0x438);
  local_4 = 0;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = InputHandler_Ctor(pvVar1,param_1);
  }
  *(void **)(param_1 + 0x180) = pvVar1;
  ExceptionList = local_c;
  return;
}

