
void * __thiscall CollisionLevel_ctorWithLevel(void *this,int param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd168;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Level_ctor(this,*(undefined4 *)(param_1 + 4));
  local_4 = 0;
  *(undefined ***)this = &PTR_Level_DeletingDtor2_004d9068;
  Level_LoadMeshes(this,param_1);
  ExceptionList = local_c;
  return this;
}

