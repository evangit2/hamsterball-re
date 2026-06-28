
void __fastcall ScoreDisplay_dtor(undefined4 *param_1)

{
  void *pvStack_c;
  undefined1 *puStack_8;
  int iStack_4;
  
  puStack_8 = &LAB_004cc26e;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_ScoreDisplay_scalar_dtor_004d67e8;
  iStack_4 = 5;
  if (0xf < (uint)param_1[0x455]) {
    _free((void *)param_1[0x450]);
  }
  param_1[0x455] = 0xf;
  param_1[0x454] = 0;
  *(undefined1 *)(param_1 + 0x450) = 0;
  if (0xf < (uint)param_1[0x44e]) {
    _free((void *)param_1[0x449]);
  }
  param_1[0x44e] = 0xf;
  param_1[0x44d] = 0;
  *(undefined1 *)(param_1 + 0x449) = 0;
  ToggleTimer_Cleanup(param_1 + 0x443);
  iStack_4._0_1_ = 4;
  BaseObject_Init(param_1 + 0x43b);
  iStack_4._0_1_ = 3;
  BaseObject_Init(param_1 + 0x434);
  iStack_4._0_1_ = 2;
  BaseObject_Init(param_1 + 0x42d);
  iStack_4._0_1_ = 1;
  BaseObject_Init(param_1 + 0x426);
  iStack_4 = (uint)iStack_4._1_3_ << 8;
  BaseObject_Init(param_1 + 0x41f);
  iStack_4 = 0xffffffff;
  SceneObject_dtor(param_1);
  ExceptionList = pvStack_c;
  return;
}

