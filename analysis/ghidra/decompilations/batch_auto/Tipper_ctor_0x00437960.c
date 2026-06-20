
void * __thiscall Tipper_ctor(void *this,undefined4 param_1,int param_2)

{
  int iVar1;
  undefined4 local_18 [3];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cb8e8;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Stands_ctor(this,param_2);
  *(undefined4 *)((int)this + 0x10d0) = param_1;
  local_4 = 0;
  *(undefined ***)this = &PTR_MeshNode_RenderState_DeleteDtor_004d4ed8;
  if ((undefined4 *)((int)this + 0x10d8) != local_18) {
    *(undefined4 *)((int)this + 0x10d8) = 0;
    *(undefined4 *)((int)this + 0x10dc) = 0;
    *(undefined4 *)((int)this + 0x10e0) = 0;
  }
  if ((undefined4 *)((int)this + 0x10e4) != local_18) {
    *(undefined4 *)((int)this + 0x10e4) = 0;
    *(undefined4 *)((int)this + 0x10e8) = 0;
    *(undefined4 *)((int)this + 0x10ec) = 0;
  }
  *(undefined4 *)((int)this + 0x10f0) = 0;
  *(undefined4 *)((int)this + 0x10f8) = 0;
  *(undefined4 *)((int)this + 0x10d4) = 0;
  *(undefined4 *)((int)this + 0x10fc) = 0;
  *(undefined4 *)((int)this + 0x10f4) = 0xc61c3c00;
  iVar1 = RNG_Rand(&PTR_OBJ_VTABLE,100,'\0');
  *(int *)((int)this + 0x1100) = iVar1 + 0x32;
  ExceptionList = local_c;
  return this;
}

