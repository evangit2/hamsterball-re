
void * __thiscall CollisionNode_ctor(void *this,undefined4 param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd318;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  CollisionNode_BaseInit(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_Transform_ScalarDtor_004d9128;
  NoOp();
  *(undefined4 *)((int)this + 0xc) = 0x3dcccccd;
  *(undefined4 *)((int)this + 0x10) = 0x3dcccccd;
  ExceptionList = local_c;
  return this;
}

