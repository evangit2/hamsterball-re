
void * __thiscall
Rotator_ctor(void *this,undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            void *param_5)

{
  void *pvVar1;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cb7e1;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Stands_ctor(this,(int)param_5);
  local_4 = 0;
  *(undefined ***)this = &PTR_Lifter_DeletingDtor_004d5518;
  AthenaList_Init((void *)((int)this + 0x10f0),0);
  local_4._0_1_ = 1;
  *(undefined4 *)((int)this + 0x10d0) = param_1;
  if ((undefined4 *)((int)this + 0x10d8) != &param_2) {
    *(undefined4 *)((int)this + 0x10d8) = param_2;
    *(undefined4 *)((int)this + 0x10dc) = param_3;
    *(undefined4 *)((int)this + 0x10e0) = param_4;
  }
  param_5 = (void *)RNG_Rand(&PTR_OBJ_VTABLE,0xfa,'\0');
  *(float *)((int)this + 0x10e4) = (float)(int)param_5;
  param_5 = operator_new(0x10d0);
  local_4 = CONCAT31(local_4._1_3_,2);
  if (param_5 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = CollisionLevel_ctorWithLevel(param_5,(int)this);
  }
  *(void **)((int)this + 0x10d4) = pvVar1;
  *(undefined4 *)((int)pvVar1 + 0x434) = *(undefined4 *)((int)this + 0x434);
  *(undefined1 *)(*(int *)((int)this + 0x10d4) + 0x431) = 0;
  *(undefined4 *)((int)this + 0x10e8) = 0;
  *(undefined4 *)((int)this + 0x10ec) = 0x3f800000;
  if (*(char *)(*(int *)(*(int *)((int)this + 0x10d0) + 0x878) + 0x237) == '\0') {
    *(undefined4 *)((int)this + 0x10e8) = 0xbe4ccccd;
    *(undefined4 *)((int)this + 0x10ec) = 0;
  }
  ExceptionList = local_c;
  return this;
}

