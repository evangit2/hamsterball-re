
void __fastcall StdString3_Clear(undefined4 *param_1)

{
  *param_1 = &PTR_StdString3_DeletingDtor_004d623c;
  if (0xf < (uint)param_1[0x25]) {
    _free((void *)param_1[0x20]);
  }
  param_1[0x25] = 0xf;
  param_1[0x24] = 0;
  *(undefined1 *)(param_1 + 0x20) = 0;
  if (0xf < (uint)param_1[0x1e]) {
    _free((void *)param_1[0x19]);
  }
  param_1[0x1e] = 0xf;
  param_1[0x1d] = 0;
  *(undefined1 *)(param_1 + 0x19) = 0;
  if (0xf < (uint)param_1[0x17]) {
    _free((void *)param_1[0x12]);
  }
  param_1[0x16] = 0;
  param_1[0x17] = 0xf;
  *(undefined1 *)(param_1 + 0x12) = 0;
  return;
}

