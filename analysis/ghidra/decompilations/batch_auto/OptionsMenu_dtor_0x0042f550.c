
void __fastcall OptionsMenu_dtor(undefined4 *param_1)

{
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cb398;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_OptionsMenu_DeletingDtor3_004d4560;
  local_4 = 0;
  if ((undefined4 *)param_1[0x337] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x337])(1);
  }
  param_1[0x337] = 0;
  if ((undefined4 *)param_1[0x338] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x338])(1);
  }
  param_1[0x338] = 0;
  if ((undefined4 *)param_1[0x339] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x339])(1);
  }
  param_1[0x339] = 0;
  if ((undefined4 *)param_1[0x33a] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x33a])(1);
  }
  param_1[0x33a] = 0;
  if ((undefined4 *)param_1[0x33b] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x33b])(1);
  }
  param_1[0x33b] = 0;
  if ((undefined4 *)param_1[0x33c] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x33c])(1);
  }
  param_1[0x33c] = 0;
  if ((undefined4 *)param_1[0x33d] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x33d])(1);
  }
  param_1[0x33d] = 0;
  if ((undefined4 *)param_1[0x33e] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x33e])(1);
  }
  param_1[0x33e] = 0;
  if ((undefined4 *)param_1[0x33f] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x33f])(1);
  }
  param_1[0x33f] = 0;
  if ((undefined4 *)param_1[0x340] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x340])(1);
  }
  param_1[0x340] = 0;
  if ((undefined4 *)param_1[0x341] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x341])(1);
  }
  param_1[0x341] = 0;
  if ((undefined4 *)param_1[0x342] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x342])(1);
  }
  param_1[0x342] = 0;
  local_4 = 0xffffffff;
  UIList_Cleanup(param_1);
  ExceptionList = pvStack_c;
  return;
}

