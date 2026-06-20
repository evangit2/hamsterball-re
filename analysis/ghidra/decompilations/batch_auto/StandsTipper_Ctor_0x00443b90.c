
void * __thiscall
StandsTipper_Ctor(void *this,undefined4 param_1,int param_2,int param_3,undefined4 param_4,
                 undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,
                 undefined4 param_9)

{
  void *pvVar1;
  void *local_c;
  undefined1 *puStack_8;
  int local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cc003;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Stands_ctor(this,param_2);
  local_4 = 0;
  *(undefined ***)this = &PTR_StandsTipper_DeletingDtor_004d6240;
  pvVar1 = operator_new(0x10d0);
  local_4._0_1_ = 1;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = TipperVisual_ctor(pvVar1,param_3);
  }
  local_4 = (uint)local_4._1_3_ << 8;
  *(void **)((int)this + 0x10ec) = pvVar1;
  TipperVisual_Attach(pvVar1,(int)this);
  if ((undefined4 *)((int)this + 0x10d4) != &param_4) {
    *(undefined4 *)((int)this + 0x10d4) = param_4;
    *(undefined4 *)((int)this + 0x10d8) = param_5;
    *(undefined4 *)((int)this + 0x10dc) = param_6;
  }
  if ((undefined4 *)((int)this + 0x10e0) != &param_7) {
    *(undefined4 *)((int)this + 0x10e0) = param_7;
    *(undefined4 *)((int)this + 0x10e4) = param_8;
    *(undefined4 *)((int)this + 0x10e8) = param_9;
  }
  *(undefined4 *)((int)this + 0x10d0) = 0x43960000;
  *(undefined1 *)((int)this + 0x10f0) = 0;
  *(undefined4 *)((int)this + 0x10f4) = param_1;
  *(undefined4 *)((int)this + 0x10f8) = 0;
  SceneObject_RenderScaled(this);
  ExceptionList = local_c;
  return this;
}

