
void __fastcall D3DTexture_Ctor(undefined4 *param_1)

{
  param_1[4] = 0;
  param_1[3] = 0;
  param_1[2] = 0;
  param_1[1] = 0;
  *(undefined1 *)(param_1 + 5) = 1;
  *(undefined1 *)((int)param_1 + 0x15) = 0;
  *(undefined1 *)((int)param_1 + 0x16) = 1;
  param_1[6] = 0;
  *param_1 = &PTR_BaseObject_DeletingDtor_004d9ecc;
  param_1[7] = 0;
  param_1[8] = 0;
  param_1[9] = 0;
  return;
}

