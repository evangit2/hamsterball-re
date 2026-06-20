
void * __fastcall Sound_LoadAndAppend(int param_1)

{
  void *this;
  void *pvVar1;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd29b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  this = operator_new(0x420);
  pvVar1 = (void *)0x0;
  local_4 = 0;
  if (this != (void *)0x0) {
    pvVar1 = SoundList_Ctor(this,param_1);
  }
  local_4 = 0xffffffff;
  AthenaList_Append((void *)(param_1 + 4),(int)pvVar1);
  Sound_LoadOggOrWav(pvVar1);
  ExceptionList = local_c;
  return pvVar1;
}

