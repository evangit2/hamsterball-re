
void __fastcall StreamReader_dtor(undefined4 *param_1)

{
  *param_1 = &PTR_StreamReader_DeletingDtor_004d91b8;
  _free((void *)param_1[1]);
  param_1[1] = 0;
  _free((void *)param_1[2]);
  param_1[2] = 0;
  return;
}

