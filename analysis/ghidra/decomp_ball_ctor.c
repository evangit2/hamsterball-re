
undefined4 * __thiscall Ball_ctor(void *this,int param_1)

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
  GameObject_ctor(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_FUN_004cf3a0;
  FUN_004531e0((undefined4 *)((int)this + 0xc84));
  local_4 = CONCAT31(local_4._1_3_,1);
  *(undefined1 *)((int)this + 0xc80) = 0;
  iVar1 = FUN_00453150(local_20,0x3f800000,0x3f800000,0,0x3f800000);
  *(undefined4 *)((int)this + 0xc88) = *(undefined4 *)(iVar1 + 4);
  *(undefined4 *)((int)this + 0xc8c) = *(undefined4 *)(iVar1 + 8);
  *(undefined4 *)((int)this + 0xc90) = *(undefined4 *)(iVar1 + 0xc);
  *(undefined4 *)((int)this + 0xc94) = *(undefined4 *)(iVar1 + 0x10);
  FUN_00453200(local_20);
  ExceptionList = local_c;
  return this;
}

