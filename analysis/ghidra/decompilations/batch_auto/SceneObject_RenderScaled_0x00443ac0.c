
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall SceneObject_RenderScaled(int *param_1)

{
  undefined4 local_50 [16];
  void *pvStack_10;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cbfd8;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  Timer_Init(local_50);
  local_4 = 0;
  Gfx_ScaleX(_DAT_004cf44c - (float)param_1[0x439]);
  Gfx_SetPosition(param_1[0x435],(float)param_1[0x436] - (float)param_1[0x434],param_1[0x437]);
  (**(code **)(*param_1 + 0x58))();
  (**(code **)(*param_1 + 0x54))(local_50);
  puStack_8 = (undefined1 *)0xffffffff;
  Timer_Cleanup((undefined4 *)&stack0xffffffac);
  ExceptionList = pvStack_10;
  return;
}

