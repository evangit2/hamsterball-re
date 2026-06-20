
void * __thiscall MeshWorld_ctor_v2(void *this,undefined4 param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd43b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *(undefined ***)this = &PTR_MeshWorld_DeletingDtor2_004d91c4;
  AthenaList_Init((void *)((int)this + 4),0);
  local_4 = 0;
  SceneList_Ctor((undefined4 *)((int)this + 0x428));
  *(undefined4 *)((int)this + 0x41c) = 0;
  *(undefined4 *)((int)this + 0x420) = 0;
  *(undefined4 *)((int)this + 0x424) = 0;
  *(undefined4 *)((int)this + 0x844) = param_1;
  ExceptionList = local_c;
  return this;
}

