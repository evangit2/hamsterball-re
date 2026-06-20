
void __fastcall App_CreateInputDevice(int param_1)

{
  void *pvVar1;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd61b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  pvVar1 = operator_new(0x91c);
  local_4 = 0;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = SoundDevice_ctor(pvVar1,param_1);
  }
  *(void **)(param_1 + 0x178) = pvVar1;
  ExceptionList = local_c;
  return;
}

