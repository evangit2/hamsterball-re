
void * __thiscall
ArenaStands_ctor
          (void *this,undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
          void *param_5)

{
  undefined4 *puVar1;
  void *pvVar2;
  int iVar3;
  int local_50 [14];
  void *pvStack_18;
  undefined1 uStack_10;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cbc29;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  Stands_ctor(this,(int)param_5);
  local_4 = 0;
  *(undefined ***)this = &PTR_ArenaCollisionLevel_DeletingDtor_004d5a70;
  ToggleTimer_Init((undefined4 *)((int)this + 0x10ec));
  puVar1 = (undefined4 *)((int)this + 0x10d0);
  local_4._0_1_ = 1;
  *(undefined4 *)((int)this + 0x10e4) = param_1;
  if (puVar1 != &param_2) {
    *puVar1 = param_2;
    *(undefined4 *)((int)this + 0x10d4) = param_3;
    *(undefined4 *)((int)this + 0x10d8) = param_4;
  }
  param_5 = operator_new(0x10d0);
  local_4._0_1_ = 2;
  if (param_5 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = CollisionLevel_ctorWithLevel(param_5,(int)this);
  }
  *(void **)((int)this + 0x10e8) = pvVar2;
  *(undefined4 *)((int)pvVar2 + 0x434) = *(undefined4 *)((int)this + 0x434);
  local_4._0_1_ = 1;
  *(undefined1 *)(*(int *)((int)this + 0x10e8) + 0x431) = 0;
  Timer_Init(local_50);
  local_4 = CONCAT31(local_4._1_3_,3);
  if (&param_2 != puVar1) {
    param_2 = *puVar1;
    param_3 = *(undefined4 *)((int)this + 0x10d4);
    param_4 = *(undefined4 *)((int)this + 0x10d8);
  }
  (**(code **)(local_50[0] + 8))(param_2,param_3,param_4);
  SceneObject_CallUpdate((int)this);
  SceneObject_CallRender((int)this);
  *(undefined4 *)((int)this + 0x10dc) = 0;
  iVar3 = RNG_Rand(&PTR_OBJ_VTABLE,100,'\0');
  *(int *)((int)this + 0x10e0) = iVar3;
  *(undefined1 *)((int)this + 0x10f0) = 0;
  *(undefined4 *)((int)this + 0x10f8) = 0;
  *(undefined4 *)((int)this + 0x10f4) = 5;
  *(undefined1 *)((int)this + 0x1100) = 0;
  uStack_10 = 1;
  Timer_Cleanup((undefined4 *)&stack0xffffffa4);
  ExceptionList = pvStack_18;
  return this;
}

