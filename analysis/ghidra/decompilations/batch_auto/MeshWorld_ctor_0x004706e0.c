
void * __thiscall MeshWorld_ctor(void *this,undefined4 param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cd836;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *(undefined ***)this = &MeshWorld_vtable;
  *(undefined4 *)((int)this + 0x20) = 0xf;
  *(undefined4 *)((int)this + 0x1c) = 0;
  *(undefined1 *)((int)this + 0xc) = 0;
  local_4 = 0;
  AthenaList_Init((void *)((int)this + 0x2c),0);
  local_4 = CONCAT31(local_4._1_3_,1);
  InitMaterialArray((undefined4 *)((int)this + 0x45c));
  *(undefined4 *)((int)this + 4) = param_1;
  *(undefined4 *)((int)this + 0x45c) = 0x4b18967f;
  *(undefined4 *)((int)this + 0x460) = 0x4b18967f;
  *(undefined4 *)((int)this + 0x464) = 0x4b18967f;
  *(undefined4 *)((int)this + 0x468) = 0xcb18967f;
  *(undefined4 *)((int)this + 0x46c) = 0xcb18967f;
  *(undefined4 *)((int)this + 0x470) = 0xcb18967f;
  *(undefined4 *)((int)this + 0x28) = 0;
  *(undefined4 *)((int)this + 0x448) = 0;
  *(undefined4 *)((int)this + 0x44c) = 0;
  *(undefined4 *)((int)this + 0x450) = 0;
  *(undefined4 *)((int)this + 0x444) = 0;
  *(undefined1 *)((int)this + 0x459) = 0;
  *(undefined1 *)((int)this + 0x484) = 0;
  *(undefined4 *)((int)this + 0x454) = 0;
  *(undefined1 *)((int)this + 0x458) = 1;
  ExceptionList = local_c;
  return this;
}

