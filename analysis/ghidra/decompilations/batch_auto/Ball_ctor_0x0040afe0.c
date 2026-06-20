
void * __thiscall Ball_ctor(void *this,int param_1)

{
  int iVar1;
  undefined4 local_20 [5];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c95a6;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Ball_ctor2(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_Ball_dtor_004cf3a0;
  Vec3_Init((undefined4 *)((int)this + 0xc84));
  local_4 = CONCAT31(local_4._1_3_,1);
  *(undefined1 *)((int)this + 0xc80) = 0;
  iVar1 = Matrix_Scale4x4(local_20,0x3f800000,0x3f800000,0,0x3f800000);
  *(undefined4 *)((int)this + 0xc88) = *(undefined4 *)(iVar1 + 4);
  *(undefined4 *)((int)this + 0xc8c) = *(undefined4 *)(iVar1 + 8);
  *(undefined4 *)((int)this + 0xc90) = *(undefined4 *)(iVar1 + 0xc);
  *(undefined4 *)((int)this + 0xc94) = *(undefined4 *)(iVar1 + 0x10);
  Matrix_Identity(local_20);
  ExceptionList = local_c;
  return this;
}

