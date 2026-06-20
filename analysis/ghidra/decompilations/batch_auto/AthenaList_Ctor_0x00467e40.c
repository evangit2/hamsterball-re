
void * __thiscall AthenaList_Ctor(void *this,void *param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd358;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  StdException_Ctor(this);
  *(undefined ***)this = &PTR_Exception_DeletingDtor_004e998c;
  *(undefined4 *)((int)this + 0x20) = 0;
  *(undefined4 *)((int)this + 0x24) = 0xf;
  local_4 = 0;
  *(undefined1 *)((int)this + 0x10) = 0;
  StdString_Substr((void *)((int)this + 0xc),param_1,0,0xffffffff);
  ExceptionList = local_c;
  return this;
}

