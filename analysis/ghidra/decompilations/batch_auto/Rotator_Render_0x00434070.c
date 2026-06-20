
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall Rotator_Render(void *this,char param_1,int param_2)

{
  void *pvVar1;
  float fVar2;
  float fVar3;
  undefined4 local_50 [15];
  void *pvStack_14;
  void *pvStack_c;
  undefined1 *puStack_8;
  float local_4;
  
  local_4 = -NAN;
  puStack_8 = &LAB_004cbbe8;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  Scene_RenderAllObjects(this,param_1,param_2);
  Timer_Init(local_50);
  local_4 = 0.0;
  Gfx_ScaleX(*(float *)((int)this + 0x10f4));
  Gfx_SetPosition(*(float *)((int)this + 0x10dc) - _DAT_004cf9f8,*(undefined4 *)((int)this + 0x10e0)
                  ,*(float *)((int)this + 0x10e4) - _DAT_004cf9f8);
  pvVar1 = *(void **)(*(int *)(*(int *)((int)this + 0x10d0) + 0x878) + 0x174);
  fVar2 = *(float *)((int)pvVar1 + 0x790) + (float)_DAT_004cf308;
  *(float *)((int)pvVar1 + 0x790) = fVar2;
  fVar3 = *(float *)((int)pvVar1 + 0x794) + (float)_DAT_004cf308;
  *(float *)((int)pvVar1 + 0x794) = fVar3;
  Graphics_SetProjection(pvVar1,fVar2,fVar3);
  (**(code **)(**(int **)(*(int *)(*(int *)((int)this + 0x10d0) + 0x878) + 0x578) + 0x1c))(local_50)
  ;
  pvVar1 = *(void **)(*(int *)(*(int *)((int)this + 0x10d0) + 0x878) + 0x174);
  local_4 = *(float *)((int)pvVar1 + 0x790) - (float)_DAT_004cf308;
  *(float *)((int)pvVar1 + 0x790) = local_4;
  fVar2 = *(float *)((int)pvVar1 + 0x794) - (float)_DAT_004cf308;
  *(float *)((int)pvVar1 + 0x794) = fVar2;
  Graphics_SetProjection(pvVar1,local_4,fVar2);
  pvStack_c = (void *)0xffffffff;
  Timer_Cleanup((undefined4 *)&stack0xffffffa8);
  ExceptionList = pvStack_14;
  return;
}

