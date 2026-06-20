
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall RegisterDialog_ScalarDtor(int param_1)

{
  undefined4 uStack_5c;
  undefined4 uStack_58;
  int local_50 [12];
  void *pvStack_20;
  undefined4 uStack_18;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cc8b8;
  pvStack_c = ExceptionList;
  uStack_58 = 0x44f934;
  ExceptionList = &pvStack_c;
  Timer_Init(local_50);
  uStack_58 = 0x3eb33333;
  uStack_5c = 0x3eb33333;
  local_4 = 0;
  (**(code **)(local_50[0] + 0x18))();
  Gfx_SetPosition(*(undefined4 *)(param_1 + 8),
                  (*(float *)(param_1 + 0xc) - *(float *)(param_1 + 0x14)) - _DAT_004d039c,
                  *(undefined4 *)(param_1 + 0x10));
  (**(code **)(**(int **)(*(int *)(param_1 + 4) + 0x5a4) + 0x1c))(&uStack_5c);
  uStack_18 = 0xffffffff;
  Timer_Cleanup((undefined4 *)&stack0xffffff9c);
  ExceptionList = pvStack_20;
  return;
}

