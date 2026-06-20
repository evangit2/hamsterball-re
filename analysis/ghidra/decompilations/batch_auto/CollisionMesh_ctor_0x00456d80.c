
void * __thiscall CollisionMesh_ctor(void *this,undefined4 param_1)

{
  undefined4 local_18 [3];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cccc9;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *(undefined ***)this = &PTR_Mesh_DeletingDtor_004d8e10;
  *(undefined4 *)((int)this + 0x10) = param_1;
  AthenaList_Init((void *)((int)this + 0x18),0);
  local_4 = 0;
  AthenaList_Init((void *)((int)this + 0x430),0);
  local_4 = CONCAT31(local_4._1_3_,1);
  AthenaList_Init((void *)((int)this + 0x848),0);
  *(undefined4 *)((int)this + 0xc74) = 0;
  if ((undefined4 *)((int)this + 0xca4) != local_18) {
    *(undefined4 *)((int)this + 0xca4) = 0;
    *(undefined4 *)((int)this + 0xca8) = 0;
    *(undefined4 *)((int)this + 0xcac) = 0;
  }
  Ball_InitBattleMode(this);
  ExceptionList = local_c;
  return this;
}

