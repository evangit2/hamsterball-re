
void __thiscall TourneyMenu_LoadSaveAndShow(void *this,char *param_1)

{
  int _FileHandle;
  void *this_00;
  int *piVar1;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cac7b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  _FileHandle = FID_conflict___open(param_1,0x8000);
  __read(_FileHandle,(void *)((int)this + 8),4);
  __read(_FileHandle,(void *)((int)this + 0x14),4);
  __read(_FileHandle,(void *)((int)this + 0x18),0x3c);
  __read(_FileHandle,(void *)((int)this + 0x54),0x3c);
  __read(_FileHandle,(void *)((int)this + 0x90),4);
  __read(_FileHandle,(void *)((int)this + 0x94),1);
  __read(_FileHandle,(void *)((int)this + 0x95),1);
  __read(_FileHandle,(void *)(*(int *)((int)this + 4) + 0x236),1);
  __read(_FileHandle,(void *)(*(int *)((int)this + 4) + 0x23c),4);
  __read(_FileHandle,(void *)(*(int *)((int)this + 4) + 0x5e8),4);
  __read(_FileHandle,(void *)(*(int *)((int)this + 4) + 0x5e4),4);
  __read(_FileHandle,(void *)(*(int *)((int)this + 4) + 0x5f4),4);
  __close(_FileHandle);
  this_00 = operator_new(0x111c);
  local_4 = 0;
  if (this_00 == (void *)0x0) {
    piVar1 = (int *)0x0;
  }
  else {
    piVar1 = TourneyMenu_ctor(this_00,*(int *)((int)this + 4),'\x01');
  }
  local_4 = 0xffffffff;
  Scene_AddObject(*(void **)(*(int *)((int)this + 4) + 0x184),piVar1);
  ExceptionList = local_c;
  return;
}

