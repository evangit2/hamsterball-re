
void * __thiscall
Gear_ctor(void *this,undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
         undefined4 param_5,undefined4 param_6,undefined4 param_7,void *param_8)

{
  void *pvVar1;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cb8a1;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Stands_ctor(this,(int)param_8);
  local_4 = 0;
  *(undefined ***)this = &PTR_Gear_Vec3List_DeletingDtor_004d5ad0;
  AthenaList_Init((void *)((int)this + 0x10f8),0);
  local_4._0_1_ = 1;
  *(undefined4 *)((int)this + 0x10d0) = param_1;
  if ((undefined4 *)((int)this + 0x10d8) != &param_2) {
    *(undefined4 *)((int)this + 0x10d8) = param_2;
    *(undefined4 *)((int)this + 0x10dc) = param_3;
    *(undefined4 *)((int)this + 0x10e0) = param_4;
  }
  if ((undefined4 *)((int)this + 0x10e4) != &param_5) {
    *(undefined4 *)((int)this + 0x10e4) = param_5;
    *(undefined4 *)((int)this + 0x10e8) = param_6;
    *(undefined4 *)((int)this + 0x10ec) = param_7;
  }
  param_8 = operator_new(0x10d0);
  local_4 = CONCAT31(local_4._1_3_,2);
  if (param_8 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = CollisionLevel_ctorWithLevel(param_8,(int)this);
  }
  *(void **)((int)this + 0x10d4) = pvVar1;
  *(undefined4 *)((int)pvVar1 + 0x434) = *(undefined4 *)((int)this + 0x434);
  *(undefined1 *)(*(int *)((int)this + 0x10d4) + 0x431) = 0;
  *(undefined4 *)((int)this + 0x10f0) = 0;
  *(undefined1 *)((int)this + 0x1510) = 0;
  *(undefined4 *)((int)this + 0x10f4) = 0x3f800000;
  ExceptionList = local_c;
  return this;
}

