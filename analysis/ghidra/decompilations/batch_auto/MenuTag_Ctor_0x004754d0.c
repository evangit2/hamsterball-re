
void * __thiscall MenuTag_Ctor(void *this,undefined4 param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cdd0b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *(undefined ***)this = &PTR_MenuTag_DeletingDtor_004da170;
  AthenaList_Init((void *)((int)this + 8),0);
  local_4 = 0;
  *(undefined4 *)((int)this + 4) = param_1;
  AthenaList_Free((int)this + 8);
  ExceptionList = local_c;
  return this;
}

