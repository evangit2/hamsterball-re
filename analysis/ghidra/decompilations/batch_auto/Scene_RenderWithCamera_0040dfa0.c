/*
 * Function: Scene_RenderWithCamera
 * Address: 0x0040dfa0
 * Signature: Scene_RenderWithCamera(...)
 *
 * Patterns: vtable dispatch, SEH frame, rendering, camera, scene, level. Calls: Scene_RenderWithCamera, Level_RenderDynamicObjects, Timer_Init, Gfx_ScaleZ, Gfx_SetPosition, Wave_Sin, Matrix44_Zero, Gfx_ScaleX. Offsets: 9, Lines: 75
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall Scene_RenderWithCamera(void *this,undefined4 param_1)

{
  int *piVar1;
  int iVar2;
  undefined1 auVar3 [10];
  undefined4 uStack_90;
  undefined4 uStack_8c;
  int iVar4;
  int *piVar5;
  undefined4 uVar6;
  int aiStack_70 [3];
  int iStack_5c;
  int local_50;
  void *pvStack_4c;
  undefined4 uStack_44;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c95d8;
  pvStack_c = ExceptionList;
  aiStack_70[2] = param_1;
  aiStack_70[1] = 0x40dfc6;
  ExceptionList = &pvStack_c;
  Level_RenderDynamicObjects((int)this);
  aiStack_70[2] = 0x40dfcf;
  Timer_Init(&local_50);
  aiStack_70[2] = 0x3f933333;
  aiStack_70[1] = 0x3f933333;
  aiStack_70[0] = 0x3f933333;
  local_4 = 0;
  (**(code **)(local_50 + 0x18))();
  Gfx_ScaleZ(-*(float *)((int)this + 0x43a0));
  piVar1 = (int *)((int)this + 0x4394);
  if (aiStack_70 + 2 != piVar1) {
    aiStack_70[2] = *piVar1;
  }
  iVar2 = aiStack_70[2];
  (**(code **)(iStack_5c + 8))();
  uStack_8c = 0x40e056;
  Gfx_SetPosition(0,*(undefined4 *)((int)this + 0x43b0),0);
  uVar6 = 0;
  auVar3 = Wave_Sin(&PTR_PTR_004f7188,*(float *)((int)this + 0x43a4));
  uStack_8c = 0x40e07e;
  Gfx_SetPosition(0,(float)((float10)auVar3 * (float10)_DAT_004cf9f8),uVar6);
  uVar6 = 0;
  piVar5 = aiStack_70 + 2;
  (**(code **)(**(int **)((int)this + 0x4390) + 0x1c))();
  Matrix44_Zero((int)aiStack_70);
  iVar4 = 0x3f933333;
  uStack_8c = 0x3f933333;
  uStack_90 = 0x3f933333;
  (**(code **)(aiStack_70[0] + 0x18))();
  Gfx_ScaleZ(-*(float *)((int)this + 0x43a0));
  Gfx_ScaleX(180.0);
  Gfx_SetPosition(0xc20c0000,0,0);
  if ((int *)&stack0xffffff78 != piVar1) {
    iVar4 = *piVar1;
    piVar5 = *(int **)((int)this + 0x4398);
    uVar6 = *(undefined4 *)((int)this + 0x439c);
  }
  (**(code **)(iVar2 + 8))(iVar4,piVar5,uVar6);
  Gfx_SetPosition(0,*(undefined4 *)((int)this + 0x43b0),0);
  uVar6 = 0;
  auVar3 = Wave_Sin(&PTR_PTR_004f7188,*(float *)((int)this + 0x43a4));
  Gfx_SetPosition(0,(float)((float10)auVar3 * (float10)_DAT_004cf9f8),uVar6);
  (**(code **)(**(int **)((int)this + 0x4390) + 0x1c))(&stack0xffffff78,0);
  uStack_44 = 0xffffffff;
  Timer_Cleanup(&uStack_90);
  ExceptionList = pvStack_4c;
  return;
}
