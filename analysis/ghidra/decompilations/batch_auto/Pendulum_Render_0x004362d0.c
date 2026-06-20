
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall Pendulum_Render(int param_1)

{
  undefined1 auVar1 [10];
  undefined4 uVar2;
  undefined4 local_50 [15];
  void *pvStack_14;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cbbe8;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  Timer_Init(local_50);
  local_4 = 0;
  Gfx_ScaleY(8.0);
  Gfx_ScaleZ(12.0);
  Gfx_ScaleX(*(float *)(param_1 + 0x24));
  uVar2 = *(undefined4 *)(param_1 + 0x10);
  auVar1 = Wave_Sin(&PTR_PTR_004f7188,*(float *)(param_1 + 0x20));
  Gfx_SetPosition(*(undefined4 *)(param_1 + 8),
                  (float)((float10)auVar1 * (float10)_DAT_004d039c +
                         (float10)*(float *)(param_1 + 0xc)),uVar2);
  (**(code **)(**(int **)(*(int *)(param_1 + 4) + 0x4370) + 0x1c))(local_50);
  pvStack_c = (void *)0xffffffff;
  Timer_Cleanup((undefined4 *)&stack0xffffffa8);
  ExceptionList = pvStack_14;
  return;
}

