
/* Gadget_ctor - constructs SceneObject/Gadget base. Size 0x870. Fields: vtable@0 (0x4D9170),
   field_4-10=0, app_ptr@14, vtable_18@18, field_1C-28=0, byte_2C=0, field_30=0, AthenaList@34 child
   gadgets, AthenaList@44C, field_864=0, name@868="Generic Gadget", field_870=App->0x1DC */

void * __thiscall Gadget_ctor(void *this,int param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined1 local_4;
  undefined3 uStack_3;
  
  puStack_8 = &LAB_004cd41e;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *(undefined4 *)((int)this + 0x10) = 0;
  *(undefined4 *)((int)this + 0xc) = 0;
  *(undefined4 *)((int)this + 8) = 0;
  *(undefined4 *)((int)this + 4) = 0;
  *(undefined ***)this = &PTR_SceneObject_ScalarDtor_004d9170;
  *(undefined ***)((int)this + 0x18) = &PTR_LAB_004cf584;
  *(undefined4 *)((int)this + 0x28) = 0;
  *(undefined4 *)((int)this + 0x24) = 0;
  *(undefined4 *)((int)this + 0x20) = 0;
  *(undefined4 *)((int)this + 0x1c) = 0;
  local_4 = 1;
  uStack_3 = 0;
  AthenaList_Init((void *)((int)this + 0x34),0);
  _local_4 = CONCAT31(uStack_3,2);
  AthenaList_Init((void *)((int)this + 0x44c),0);
  *(int *)((int)this + 0x14) = param_1;
  *(undefined1 *)((int)this + 0x2c) = 0;
  *(undefined4 *)((int)this + 0x30) = 0;
  *(undefined1 *)((int)this + 0x86c) = 0;
  *(undefined1 *)((int)this + 0x86d) = 0;
  *(undefined1 *)((int)this + 0x86e) = 0;
  *(undefined1 *)((int)this + 0x86f) = 0;
  *(undefined1 *)((int)this + 0x874) = 0;
  *(undefined4 *)((int)this + 0x864) = 0;
  *(char **)((int)this + 0x868) = "Generic Gadget";
  *(undefined4 *)((int)this + 0x870) = *(undefined4 *)(param_1 + 0x1dc);
  ExceptionList = local_c;
  return this;
}

