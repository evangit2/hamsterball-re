
undefined4 * __thiscall MusicChannel_LoadAndAppend(void *this,char *param_1)

{
  void *this_00;
  undefined4 *this_01;
  undefined4 uVar1;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cdb3b;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  this_00 = operator_new(0x534);
  local_4 = 0;
  if (this_00 == (void *)0x0) {
    this_01 = (undefined4 *)0x0;
  }
  else {
    this_01 = MusicChannel_Ctor(this_00,this);
  }
  local_4 = 0xffffffff;
  uVar1 = LoadMusicFile(this_01,param_1);
  if ((char)uVar1 == '\0') {
    if (this_01 != (undefined4 *)0x0) {
      (**(code **)*this_01)(1);
    }
    this_01 = (undefined4 *)0x0;
  }
  AthenaList_Append((void *)((int)this + 0xc),(int)this_01);
  ExceptionList = pvStack_c;
  return this_01;
}

