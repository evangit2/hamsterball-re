
void * __thiscall Menu_Ctor(void *this,int param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cdc08;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Gadget_ctor(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_Menu_DeletingDtor_004da0a0;
  AthenaList_Init((void *)((int)this + 0x87c),0);
  local_4._0_1_ = 1;
  AthenaList_Init((void *)((int)this + 0xc94),0);
  local_4._0_1_ = 2;
  AthenaList_Init((void *)((int)this + 0x10ac),0);
  local_4._0_1_ = 3;
  AthenaList_Init((void *)((int)this + 0x14c4),0);
  local_4._0_1_ = 4;
  AthenaList_Init((void *)((int)this + 0x18dc),0);
  local_4._0_1_ = 5;
  AthenaList_Init((void *)((int)this + 0x1cf4),0);
  local_4._0_1_ = 6;
  AthenaList_Init((void *)((int)this + 0x210c),0);
  local_4._0_1_ = 7;
  AthenaList_Init((void *)((int)this + 0x2524),0);
  local_4 = CONCAT31(local_4._1_3_,8);
  AthenaList_Init((void *)((int)this + 0x293c),0);
  *(undefined4 *)((int)this + 0x878) = 0;
  *(undefined4 *)((int)this + 0x2d5c) = 0;
  *(undefined1 *)((int)this + 0x2d64) = 0;
  *(undefined1 *)((int)this + 0x2d58) = 1;
  ExceptionList = local_c;
  return this;
}

