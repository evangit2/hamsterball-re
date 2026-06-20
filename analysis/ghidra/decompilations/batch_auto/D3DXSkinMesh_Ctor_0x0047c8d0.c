
void * __thiscall D3DXSkinMesh_Ctor(void *this,int param_1)

{
  void *pvVar1;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cdf29;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *(undefined ***)this = &PTR_D3DXSkinMesh_DeletingDtor_004db26c;
  AthenaList_Init((void *)((int)this + 0x34),0);
  local_4 = 0;
  AthenaList_Init((void *)((int)this + 0x450),0);
  *(int *)((int)this + 0x14) = param_1;
  local_4 = CONCAT31(local_4._1_3_,1);
  *(undefined4 *)((int)this + 4) = 0;
  *(undefined4 *)((int)this + 8) = 0;
  *(undefined4 *)((int)this + 0xc) = 0;
  *(undefined4 *)((int)this + 0x10) = 0;
  *(int *)((int)this + 0x18) = param_1 + 2;
  pvVar1 = operator_new(param_1 * 0xc6c);
  *(void **)((int)this + 0x20) = pvVar1;
  pvVar1 = operator_new(*(int *)((int)this + 0x14) * 0x24);
  *(void **)((int)this + 0x28) = pvVar1;
  pvVar1 = operator_new(*(int *)((int)this + 0x14) * 0x18);
  *(void **)((int)this + 0x30) = pvVar1;
  *(undefined4 *)((int)this + 0x1c) = 0;
  *(undefined4 *)((int)this + 0x24) = 0;
  *(undefined4 *)((int)this + 0x2c) = 0;
  AthenaList_Free((int)this + 0x34);
  ExceptionList = local_c;
  return this;
}

