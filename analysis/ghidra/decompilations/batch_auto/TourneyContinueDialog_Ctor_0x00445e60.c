
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void * __thiscall TourneyContinueDialog_Ctor(void *this,int param_1,undefined4 param_2)

{
  float fVar1;
  float fVar2;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cc0b8;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  QuitDialog_ctor(this,param_1);
  *(undefined4 *)((int)this + 0x8b8) = param_2;
  *(undefined ***)this = &PTR_TourneyContinueDialog_scalar_dtor_004d65f0;
  *(undefined4 *)((int)this + 4) = 0x42e20000;
  *(undefined4 *)((int)this + 8) = 0x43160000;
  *(undefined4 *)((int)this + 0xc) = 0x44160000;
  *(undefined4 *)((int)this + 0x10) = 0x43af0000;
  local_4 = 0;
  *(undefined4 *)((int)this + 0x880) = 0x43100000;
  *(undefined4 *)((int)this + 0x884) = 0x436a0000;
  *(undefined4 *)((int)this + 0x888) = 0x42c00000;
  *(undefined4 *)((int)this + 0x88c) = 0x425c0000;
  AthenaList_Append((void *)((int)this + 0x44c),(int)this + 0x87c);
  fVar1 = _DAT_004d62ac - *(float *)((int)this + 4);
  fVar2 = _DAT_004d6360 - *(float *)((int)this + 8);
  *(undefined4 *)((int)this + 0x8a4) = 0x42c00000;
  fVar2 = fVar2 - _DAT_004d5dc4;
  *(undefined4 *)((int)this + 0x8a8) = 0x425c0000;
  *(float *)((int)this + 0x89c) = fVar1;
  *(float *)((int)this + 0x8a0) = fVar2;
  AthenaList_Append((void *)((int)this + 0x44c),(int)this + 0x898);
  ExceptionList = local_c;
  return this;
}

