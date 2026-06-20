
void * __thiscall
Stands_CtorRotator(void *this,undefined4 param_1,undefined4 param_2,undefined4 param_3,
                  undefined4 param_4,void *param_5)

{
  undefined4 *puVar1;
  void *pvVar2;
  int local_50 [14];
  void *pvStack_18;
  undefined1 uStack_10;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cbb0b;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  Stands_ctor(this,(int)param_5);
  puVar1 = (undefined4 *)((int)this + 0x10d8);
  local_4 = 0;
  *(undefined ***)this = &PTR_Rotator_DeletingDtor7_004d5a10;
  *(undefined4 *)((int)this + 0x10d0) = param_1;
  if (puVar1 != &param_2) {
    *puVar1 = param_2;
    *(undefined4 *)((int)this + 0x10dc) = param_3;
    *(undefined4 *)((int)this + 0x10e0) = param_4;
  }
  param_5 = operator_new(0x10d0);
  local_4._0_1_ = 1;
  if (param_5 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = CollisionLevel_ctorWithLevel(param_5,(int)this);
  }
  *(void **)((int)this + 0x10d4) = pvVar2;
  *(undefined4 *)((int)pvVar2 + 0x434) = *(undefined4 *)((int)this + 0x434);
  local_4._0_1_ = 0;
  *(undefined1 *)(*(int *)((int)this + 0x10d4) + 0x431) = 0;
  Timer_Init(local_50);
  local_4 = CONCAT31(local_4._1_3_,2);
  if (&param_2 != puVar1) {
    param_2 = *puVar1;
    param_3 = *(undefined4 *)((int)this + 0x10dc);
    param_4 = *(undefined4 *)((int)this + 0x10e0);
  }
  (**(code **)(local_50[0] + 8))(param_2,param_3,param_4);
  SceneObject_CallUpdate((int)this);
  SceneObject_CallRender((int)this);
  *(undefined1 *)((int)this + 0x10e4) = 0;
  uStack_10 = 0;
  Timer_Cleanup((undefined4 *)&stack0xffffffa4);
  ExceptionList = pvStack_18;
  return this;
}

