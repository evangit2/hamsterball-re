
void * __thiscall TipperVisual_Level_Ctor(void *this,int param_1)

{
  void *pvVar1;
  void *local_c;
  undefined1 *puStack_8;
  int local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cb6c3;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Stands_ctor(this,*(int *)(*(int *)(param_1 + 0x878) + 0x570));
  local_4 = 0;
  *(undefined ***)this = &PTR_Level_Cleanup_DeletingDtor_004d4e78;
  pvVar1 = operator_new(0x10d0);
  local_4._0_1_ = 1;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = TipperVisual_ctor(pvVar1,*(int *)(*(int *)(param_1 + 0x878) + 0x574));
  }
  local_4 = (uint)local_4._1_3_ << 8;
  *(void **)((int)this + 0x10d4) = pvVar1;
  TipperVisual_Attach(pvVar1,(int)this);
  *(int *)((int)this + 0x10d0) = param_1;
  *(undefined1 *)((int)this + 0x10d8) = 0;
  *(undefined4 *)((int)this + 0x10e8) = 0;
  *(undefined4 *)((int)this + 0x10f0) = 0;
  *(undefined4 *)((int)this + 0x10f4) = 0;
  ExceptionList = local_c;
  return this;
}

