
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void * __thiscall
Pendulum_ctor(void *this,undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4
             ,void *param_5)

{
  void *pvVar1;
  int iVar2;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cb8d1;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Stands_ctor(this,(int)param_5);
  local_4 = 0;
  *(undefined ***)this = &PTR_Catapult_Vec3List_DeletingDtor_004d5b30;
  AthenaList_Init((void *)((int)this + 0x10e4),0);
  local_4._0_1_ = 1;
  *(undefined4 *)((int)this + 0x10d0) = param_1;
  if ((undefined4 *)((int)this + 0x10d8) != &param_2) {
    *(undefined4 *)((int)this + 0x10d8) = param_2;
    *(undefined4 *)((int)this + 0x10dc) = param_3;
    *(undefined4 *)((int)this + 0x10e0) = param_4;
  }
  param_5 = operator_new(0x10d0);
  local_4._0_1_ = 2;
  if (param_5 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = CollisionLevel_ctorWithLevel(param_5,(int)this);
  }
  *(void **)((int)this + 0x10d4) = pvVar1;
  *(undefined4 *)((int)pvVar1 + 0x434) = *(undefined4 *)((int)this + 0x434);
  local_4 = CONCAT31(local_4._1_3_,1);
  *(undefined1 *)(*(int *)((int)this + 0x10d4) + 0x431) = 0;
  param_5 = (void *)RNG_Rand(&PTR_OBJ_VTABLE,0x168,'\0');
  *(float *)((int)this + 0x14fc) = (float)(int)param_5;
  iVar2 = RNG_Rand(&PTR_OBJ_VTABLE,10,'\0');
  *(float *)((int)this + 0x1500) = (float)iVar2 * _DAT_004cf3d0 + _DAT_004cf3f0;
  ExceptionList = local_c;
  return this;
}

