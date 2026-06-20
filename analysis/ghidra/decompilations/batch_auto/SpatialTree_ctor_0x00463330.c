
void * __thiscall SpatialTree_ctor(void *this,undefined4 param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd148;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  CollisionNode_BaseInit(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_SpatialTree_DeletingDtor_004d9038;
  NoOp();
  *(undefined1 *)((int)this + 0x1a) = 1;
  *(undefined1 *)((int)this + 0x1b) = 1;
  *(undefined1 *)((int)this + 0x1c) = 1;
  *(undefined1 *)((int)this + 0x1d) = 1;
  *(undefined1 *)((int)this + 0x1e) = 1;
  *(undefined4 *)((int)this + 0xc) = 0x3dcccccd;
  *(undefined4 *)((int)this + 0x10) = 6;
  *(undefined4 *)((int)this + 0x14) = 0x3f666666;
  *(undefined1 *)((int)this + 0x18) = 0;
  *(undefined1 *)((int)this + 0x19) = 0;
  ExceptionList = local_c;
  return this;
}

