
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void * __thiscall Gluebie_ctor(void *this,undefined4 param_1,int param_2)

{
  float fVar1;
  int iVar2;
  undefined4 local_18 [3];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cb8e8;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Stands_ctor(this,param_2);
  local_4 = 0;
  *(undefined ***)this = &PTR_MeshNode_Simple_DeleteDtor_004d4f38;
  *(undefined4 *)((int)this + 0x10d0) = param_1;
  if ((undefined4 *)((int)this + 0x10d4) != local_18) {
    *(undefined4 *)((int)this + 0x10d4) = 0;
    *(undefined4 *)((int)this + 0x10d8) = 0;
    *(undefined4 *)((int)this + 0x10dc) = 0;
  }
  *(undefined1 *)((int)this + 0x1104) = 0;
  *(undefined4 *)((int)this + 0x1108) = 0x3f800000;
  *(undefined4 *)((int)this + 0x10ec) = 0;
  iVar2 = RNG_Rand(&PTR_OBJ_VTABLE,0x168,'\0');
  *(float *)((int)this + 0x10f0) = (float)iVar2;
  iVar2 = RNG_Rand(&PTR_OBJ_VTABLE,0x32,'\0');
  fVar1 = (float)iVar2 + _DAT_004cf3ec;
  *(undefined4 *)((int)this + 0x10fc) = 0x3ba3d70a;
  *(undefined4 *)((int)this + 0x10f4) = 0x3f800000;
  *(float *)((int)this + 0x10f8) = fVar1 * _DAT_004cf524;
  iVar2 = RNG_Rand(&PTR_OBJ_VTABLE,2,'\0');
  if (iVar2 == 0) {
    *(undefined4 *)((int)this + 0x10f4) = 0xbf800000;
  }
  iVar2 = RNG_Rand(&PTR_OBJ_VTABLE,0x19,'\0');
  *(float *)((int)this + 0x1100) = ((float)iVar2 + _DAT_004cf480) * _DAT_004cf524;
  ExceptionList = local_c;
  return this;
}

