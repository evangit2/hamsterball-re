
void * __thiscall D3DXSkinMesh_Init(void *this,uint param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cd3d3;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *(undefined4 *)((int)this + 4) = 0;
  *(undefined4 *)((int)this + 8) = 0;
  *(undefined4 *)((int)this + 0xc) = 0;
  *(undefined4 *)((int)this + 0x14) = 0;
  *(undefined4 *)((int)this + 0x18) = 0;
  *(undefined4 *)((int)this + 0x1c) = 0;
  *(uint *)((int)this + 0x20) = param_1 + 1;
  local_4 = 1;
  *(undefined4 *)((int)this + 0x24) = 0;
  Vector_Resize(this,(param_1 + 1) * param_1);
  Vector_Resize((void *)((int)this + 0x10),param_1);
  ExceptionList = local_c;
  return this;
}

