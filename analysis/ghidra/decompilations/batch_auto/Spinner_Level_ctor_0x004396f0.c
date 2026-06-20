
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void * __thiscall
Spinner_Level_ctor(void *this,int param_1,void *param_2,undefined4 param_3,undefined4 param_4,
                  float param_5)

{
  void *pvVar1;
  int local_50 [14];
  void *pvStack_18;
  undefined1 uStack_10;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cba0b;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  Stands_ctor(this,*(int *)(param_1 + 0x4378));
  local_4 = 0;
  *(undefined ***)this = &PTR_Intermediate_Level_scalar_dtor_004d51e0;
  *(int *)((int)this + 0x10d0) = param_1;
  if ((void **)((int)this + 0x10d4) != &param_2) {
    *(void **)((int)this + 0x10d4) = param_2;
    *(undefined4 *)((int)this + 0x10d8) = param_3;
    *(undefined4 *)((int)this + 0x10dc) = param_4;
  }
  *(float *)((int)this + 0x10e0) = param_5;
  *(undefined4 *)((int)this + 0x10e4) = 0;
  Timer_Init(local_50);
  local_4 = CONCAT31(local_4._1_3_,1);
  Gfx_ScaleX(_DAT_004cf44c - param_5);
  Gfx_ScaleZ(0.0);
  (**(code **)(local_50[0] + 8))(param_2,param_3,param_4);
  SceneObject_CallUpdate((int)this);
  SceneObject_CallRender((int)this);
  param_2 = operator_new(0x10d0);
  uStack_10 = 2;
  if (param_2 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = CollisionLevel_ctorWithLevel(param_2,(int)this);
  }
  *(void **)((int)this + 0x10f4) = pvVar1;
  *(undefined4 *)((int)pvVar1 + 0x434) = *(undefined4 *)((int)this + 0x434);
  *(undefined1 *)(*(int *)((int)this + 0x10f4) + 0x431) = 0;
  *(undefined4 *)((int)this + 0x10e8) = 0;
  *(undefined4 *)((int)this + 0x10f0) = 100;
  *(undefined4 *)((int)this + 0x10f8) = 0x3f800000;
  uStack_10 = 0;
  Timer_Cleanup((undefined4 *)&stack0xffffffa4);
  ExceptionList = pvStack_18;
  return this;
}

