
void __thiscall App_ShowResults(void *this,char param_1)

{
  void *this_00;
  int *piVar1;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cae8b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Scene_UpdateChildren(*(int *)((int)this + 0x178));
  this_00 = operator_new(0x87c);
  local_4 = 0;
  if (this_00 == (void *)0x0) {
    piVar1 = (int *)0x0;
  }
  else {
    piVar1 = MusicPlayer_ctor(this_00,(int)this,param_1);
  }
  local_4 = 0xffffffff;
  *(int **)((int)this + 0x228) = piVar1;
  Scene_AddObject(*(void **)((int)this + 0x184),piVar1);
  ExceptionList = local_c;
  return;
}

