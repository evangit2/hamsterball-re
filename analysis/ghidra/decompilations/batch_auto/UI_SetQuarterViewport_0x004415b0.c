
void __fastcall UI_SetQuarterViewport(int *param_1)

{
  (**(code **)(*param_1 + 0x40))();
  *(undefined4 *)(param_1[0x21e] + 0x560) = 0x32;
  *(undefined4 *)(param_1[0x21e] + 0x564) = 0x32;
  *(undefined4 *)(param_1[0x21e] + 0x568) = 0x32;
  *(undefined4 *)(param_1[0x21e] + 0x56c) = 0x32;
  return;
}

